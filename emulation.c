#include <unicorn/unicorn.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#if __STDC_NO_THREADS__
#include "c11threads.h"
#else
#include <threads.h>
#endif

#include "common.h"
#include "descriptor.h"
#include "emulation.h"
#include "exe.h"

//FIXME: These are hacks (register when mapping instead!)!
extern Exe* exe;
uint8_t* stack = NULL;
uint8_t* heap = NULL;

static uint32_t gdtAddress = 0xA0000000; //FIXME: Search somehow?!
static uint32_t gdtSize = 31 * sizeof(SegmentDescriptor); //FIXME: 31 came from the UC sample, why?!

static uint32_t tlsAddress = 0xB0000000; //FIXME: No idea where to put this yet
static uint32_t tlsSize = 0x1000;

static uint32_t stackAddress = 0xC0000000; // FIXME: Search free region instead..?
static uint32_t stackSize = 16 * 1024 * 1024; // 4 MiB stack should be PLENTY

#define HEAP_ADDRESS 0x0D000000
static uint32_t heapAddress = HEAP_ADDRESS;
static uint32_t heapSize = 2048 * 1024 * 1024; // 2048 MiB

static uc_engine *uc;
static uint32_t ucAlignment = 0x1000;


unsigned int currentThread = 0;
unsigned int threadCount = 0;

typedef struct {
  uint8_t raw[10];
} X87Register;
typedef struct {

  uint64_t sleep;

  // Standard stuff
  uint32_t eip;
  uint32_t esp;
  uint32_t ebp;
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t eflags;

  // x87
  uint16_t fpcw;
  uint16_t fpsw;
  uint16_t fptw;
  X87Register fp[8];

//FIXME: MMX?
} ThreadContext;
ThreadContext* threads = NULL; //FIXME: Store pointers to threads instead? (Probably doesn't matter for re-volt)

static void TransferContext(ThreadContext* ctx, bool write) {
  uc_err(*transfer)(uc_engine*, int, void*) = write ? uc_reg_write : uc_reg_read;
  transfer(uc, UC_X86_REG_EIP, &ctx->eip);
  transfer(uc, UC_X86_REG_ESP, &ctx->esp);
  transfer(uc, UC_X86_REG_EBP, &ctx->ebp);
  transfer(uc, UC_X86_REG_EAX, &ctx->eax);
  transfer(uc, UC_X86_REG_EBX, &ctx->ebx);
  transfer(uc, UC_X86_REG_ECX, &ctx->ecx);
  transfer(uc, UC_X86_REG_EDX, &ctx->edx);
  transfer(uc, UC_X86_REG_ESI, &ctx->esi);
  transfer(uc, UC_X86_REG_EDI, &ctx->edi);
  transfer(uc, UC_X86_REG_EFLAGS, &ctx->eflags);

  transfer(uc, UC_X86_REG_FPSW, &ctx->fpsw);
  transfer(uc, UC_X86_REG_FPCW, &ctx->fpcw);
  transfer(uc, UC_X86_REG_FPTAG, &ctx->fptw);
  //FIXME: Use REG_ST0 etc. here? They seem to contain garbage?!
  transfer(uc, UC_X86_REG_FP0, &ctx->fp[0]);
  transfer(uc, UC_X86_REG_FP1, &ctx->fp[1]);
  transfer(uc, UC_X86_REG_FP2, &ctx->fp[2]);
  transfer(uc, UC_X86_REG_FP3, &ctx->fp[3]);
  transfer(uc, UC_X86_REG_FP4, &ctx->fp[4]);
  transfer(uc, UC_X86_REG_FP5, &ctx->fp[5]);
  transfer(uc, UC_X86_REG_FP6, &ctx->fp[6]);
  transfer(uc, UC_X86_REG_FP7, &ctx->fp[7]);
}


static void PrintContext(ThreadContext* ctx) {
  printf("EIP: 0x%08" PRIX32 "\n", ctx->eip);
  printf("ESP: 0x%08" PRIX32 "\n", ctx->esp);
  printf("EBP: 0x%08" PRIX32 "\n", ctx->ebp);
  printf("EAX: 0x%08" PRIX32 "\n", ctx->eax);
  printf("EBX: 0x%08" PRIX32 "\n", ctx->ebx);
  printf("ECX: 0x%08" PRIX32 "\n", ctx->ecx);
  printf("EDX: 0x%08" PRIX32 "\n", ctx->edx);
  printf("ESI: 0x%08" PRIX32 "\n", ctx->esi);
  printf("EDI: 0x%08" PRIX32 "\n", ctx->edi);
  printf("EFLAGS: 0x%08" PRIX32 "\n", ctx->eflags);

  printf("FPSW: 0x%04" PRIX16 "\n", ctx->fpsw);
  printf("FPCW: 0x%04" PRIX16 "\n", ctx->fpcw);
  printf("FPTAG: 0x%04" PRIX16 "\n", ctx->fptw);

  for(unsigned int i = 0; i < 8; i++) {
    printf("FP%d: 0x", i);
    for(unsigned int j = 0; j < 10; j++) {
      printf("%02" PRIX8, ctx->fp[i].raw[9 - j]);
    }
    printf("\n");
  }
}



// Callback for tracing all kinds of memory errors
static void UcErrorHook(uc_engine* uc, uc_mem_type type, uint64_t address, int size, int64_t value, void* user_data) {
/*
FIXME: type is one of 
    UC_MEM_READ_UNMAPPED = 19,    // Unmapped memory is read from
    UC_MEM_WRITE_UNMAPPED = 20,   // Unmapped memory is written to
    UC_MEM_FETCH_UNMAPPED = 21,   // Unmapped memory is fetched
    UC_MEM_WRITE_PROT = 22,  // Write to write protected, but mapped, memory
    UC_MEM_READ_PROT = 23,   // Read from read protected, but mapped, memory
    UC_MEM_FETCH_PROT = 24,  // Fetch from non-executable, but mapped, memory
*/
  printf("Unicorn-Engine error of type %d at 0x%" PRIx64 ", size = 0x%" PRIX32 "\n", type, address, size);
  uc_emu_stop(uc);

  ThreadContext ctx;
  TransferContext(&ctx, false);
  PrintContext(&ctx);

  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  printf("Emulation returned %X\n", eip);

  int esp;
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  for(int i = 0; i < 100; i++) {
    printf("Stack [%d] = %X\n", i, *(uint32_t*)Memory(esp + i * 4));
  }

  assert(false);
}

// Callback for tracing instructions
static void UcTraceHook(void* uc, uint64_t address, uint32_t size, void* user_data) {
  int eip, esp, eax, esi;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);
  uc_reg_read(uc, UC_X86_REG_ESP, &esp);
  uc_reg_read(uc, UC_X86_REG_EAX, &eax);
  uc_reg_read(uc, UC_X86_REG_ESI, &esi);
  static uint32_t id = 0;
  printf("%7" PRIu32 " TRACE Emulation at 0x%X (ESP: 0x%X); eax = 0x%08" PRIX32 " esi = 0x%08" PRIX32 " (TS: %" PRIu64 ")\n", id++, eip, esp, eax, esi, GetTimerValue());
}

void MapMemory(void* memory, uint32_t address, uint32_t size, bool read, bool write, bool execute) {
  //FIXME: Permissions!
  uc_err err;
  assert(size % ucAlignment == 0);
  err = uc_mem_map_ptr(uc, address, size, UC_PROT_ALL, memory);
  if (err) {
    printf("Failed on uc_mem_map_ptr() with error returned %u: %s\n", err, uc_strerror(err));
  }
  //FIXME: Add to mapped memory list
}

Address Allocate(Size size) {
  static uint32_t address = HEAP_ADDRESS;
  uint32_t ret = address;
  address += size;
#if 1
  // Debug memset to detect memory errors
  memset(Memory(ret), 0xDD, size);
#endif
  //FIXME: Proper allocator

#if 1
//FIXME: This is a hack to fix alignment + to avoid too small allocations
address += 0x1000;
address &= 0xFFFFF000;
#endif

  return ret;
}

void Free(Address address) {
  //FIXME!
}

void* Memory(uint32_t address) {

  if (address >= heapAddress && address < (heapAddress + heapSize)) {
    return &heap[address - heapAddress];
  }

  if (address >= stackAddress && address < (stackAddress + stackSize)) {
    return &stack[address - stackAddress];
  }

  if (address >= exe->peHeader.imageBase) {
    address -= exe->peHeader.imageBase;
    for(unsigned int sectionIndex = 0; sectionIndex < exe->coffHeader.numberOfSections; sectionIndex++) {
      PeSection* section = &exe->sections[sectionIndex];
      if ((address >= section->virtualAddress) && (address < (section->virtualAddress + section->virtualSize))) {
        assert(exe->mappedSections[sectionIndex] != NULL);
        uint32_t offset = address - section->virtualAddress;
        return &exe->mappedSections[sectionIndex][offset];
      }
    }
  }

  return NULL;
}

void CreateBreakpoint(uint32_t address, void* callback, void* user) {
  //FIXME: Loop over imports?!
  assert(false);
#if 0
#ifdef UC_KVM
  assert(false);
#endif
  uc_hook importHook;
  uc_hook_add(uc, &importHook, UC_HOOK_BLOCK /*UC_HOOK_CODE*/, callback, user, address, address);
#endif
}

Address CreateOut() {
  Address code_address = Allocate(2);
  uint8_t* code = Memory(code_address);
  *code++ = 0xEE; // OUT DX, AL
  //FIXME: Are changes to regs even registered here?!
  *code++ = 0xC3; // End block with RET
  return code_address;
}

typedef struct _OutHandler {
  struct _OutHandler* next;
  Address address;
  void(*callback)(void* uc, uint64_t address, uint32_t size, void* user_data);
  void* user_data;
} OutHandler;
OutHandler* outHandlers = NULL;

static void outHookHandler(uc_engine *uc, uint32_t port, int size, uint32_t value, void *user_data) {
  int eip;
  uc_reg_read(uc, UC_X86_REG_EIP, &eip);

  OutHandler* outHandler = outHandlers;
  while(outHandler != NULL) {
    if (outHandler->address == eip) {
      outHandler->callback(uc, outHandler->address, 0, outHandler->user_data);
    }
    outHandler = outHandler->next;
  }
}

void AddOutHandler(Address address, void(*callback)(void* uc, uint64_t address, uint32_t size, void* user_data), void* user_data) {
  OutHandler* outHandler = malloc(sizeof(OutHandler));
  outHandler->address = address;
  outHandler->callback = callback;
  outHandler->user_data = user_data;
  outHandler->next = outHandlers;
  outHandlers = outHandler;

  uc_hook outHook;
  uc_hook_add(uc, &outHook, UC_HOOK_INSN, outHookHandler, user_data, address, address, UC_X86_INS_OUT);

#if 0
#ifndef UC_KVM
  CreateBreakpoint(address, callback, user_data);
#endif
#endif
}

#if 0

//FIXME: Bad to use allocate in this file..?
Address CreateCallback(void* callback, void* user) {
  //FIXME: This might be faster, but needs some more work..
  Address address = Allocate(1 + 1 + strlen(user) + 1);
  static bool hooked = false;
  if (hooked == false) {
    uc_hook interruptHook;
    uc_hook_add(uc, &interruptHook, UC_HOOK_INTR, callback, NULL, 1, 0);
    hooked = true;
  }
  uint8_t* code = Memory(address);
  code[0] = 0xCC;
  code[1] = 0xC3;
  strcpy(&code[2], user);
  
  return address;
}

#endif

void InitializeEmulation() {

  uc_err err;

  err = uc_open(UC_ARCH_X86, UC_MODE_32, &uc);
  if (err) {
    printf("Failed on uc_open() with error returned %u: %s\n", err, uc_strerror(err));
  }

#ifndef UC_KVM
  // Add hooks to catch errors
  uc_hook errorHooks[6];
  {
    // Hook for memory read on unmapped memory
    uc_hook_add(uc, &errorHooks[0], UC_HOOK_MEM_READ_UNMAPPED, UcErrorHook, NULL, 1, 0);

    // Hook for invalid memory write events
    uc_hook_add(uc, &errorHooks[1], UC_HOOK_MEM_WRITE_UNMAPPED, UcErrorHook, NULL, 1, 0);

    // Hook for invalid memory fetch for execution events
    uc_hook_add(uc, &errorHooks[2], UC_HOOK_MEM_FETCH_UNMAPPED, UcErrorHook, NULL, 1, 0);

    // Hook for memory read on read-protected memory
    uc_hook_add(uc, &errorHooks[3], UC_HOOK_MEM_READ_PROT, UcErrorHook, NULL, 1, 0);

    // Hook for memory write on write-protected memory
    uc_hook_add(uc, &errorHooks[4], UC_HOOK_MEM_WRITE_PROT, UcErrorHook, NULL, 1, 0);

    // Hook for memory fetch on non-executable memory
    uc_hook_add(uc, &errorHooks[5], UC_HOOK_MEM_FETCH_PROT, UcErrorHook, NULL, 1, 0);
  }
#endif

#ifndef UC_KVM
  // Setup segments
  SegmentDescriptor* gdtEntries;
  assert(posix_memalign((void**)&gdtEntries, ucAlignment, AlignUp(gdtSize, ucAlignment)) == 0);
  memset(gdtEntries, 0x00, gdtSize);

  gdtEntries[14] = CreateDescriptor(0x00000000, 0xFFFFF000, true);  // CS
  gdtEntries[15] = CreateDescriptor(0x00000000, 0xFFFFF000, false);  // DS
  gdtEntries[16] = CreateDescriptor(tlsAddress, tlsSize - 1, false); // FS

  //FIXME: Remove? We never switch to ring 0 anyway (Came from UC sample code)
  gdtEntries[17] = CreateDescriptor(0x00000000, 0xFFFFF000, false);  // Ring 0
  gdtEntries[17].dpl = 0;  //set descriptor privilege level

  err = uc_mem_map_ptr(uc, gdtAddress, AlignUp(gdtSize, ucAlignment), UC_PROT_WRITE | UC_PROT_READ, gdtEntries);

  uc_x86_mmr gdtr;
  gdtr.base = gdtAddress;  
  gdtr.limit = gdtSize - 1;
  err = uc_reg_write(uc, UC_X86_REG_GDTR, &gdtr);

  int cs = 0x73;
  err = uc_reg_write(uc, UC_X86_REG_CS, &cs);

  int ds = 0x7B;
  err = uc_reg_write(uc, UC_X86_REG_DS, &ds);

  int es = 0x7B;
  err = uc_reg_write(uc, UC_X86_REG_ES, &es);

  int fs = 0x83;
  err = uc_reg_write(uc, UC_X86_REG_FS, &fs);

  //FIXME: Do we require GS?!

//  int ss = 0x7B;
  int ss = 0x88; // Ring 0 - Why?!
  err = uc_reg_write(uc, UC_X86_REG_SS, &ss);
#endif

#if 0
  //FIXME: Steal actual register values, consult Windows ABI
  int eax;
  int ebx;
  int ecx;
  int edx;
  uc_reg_write(uc, UC_X86_REG_EAX, &eax);
  uc_reg_write(uc, UC_X86_REG_EBX, &ebx);
  uc_reg_write(uc, UC_X86_REG_ECX, &ecx);
  uc_reg_write(uc, UC_X86_REG_EDX, &edx);
#endif

  // Map and set TLS (not exposed via flat memory)
  uint8_t* tls;
  assert(posix_memalign((void**)&tls, ucAlignment, tlsSize) == 0);
  memset(tls, 0xBB, tlsSize);
  err = uc_mem_map_ptr(uc, tlsAddress, tlsSize, UC_PROT_WRITE | UC_PROT_READ, tls);

  // Allocate a heap
  assert(posix_memalign((void **)&heap, ucAlignment, heapSize) == 0);
  memset(heap, 0xAA, heapSize);
  MapMemory(heap, heapAddress, heapSize, true, true, true);
}

void SetTracing(bool enabled) {
  // Add a trace hook so we get proper EIP after running
  static uc_hook traceHook = -1;
  if (enabled) {
    if (traceHook == -1) {
      uc_hook_add(uc, &traceHook, UC_HOOK_CODE, UcTraceHook, NULL, 1, 0);
    }
  } else {
    if (traceHook != -1) {
      uc_hook_del(uc, traceHook);
      traceHook = -1;
    }
  }
}


unsigned int CreateEmulatedThread(uint32_t eip) {

  //FIXME: Dirty hack!
  // Map and set stack
  //FIXME: Use requested size
  if (stack == NULL) {
    assert(posix_memalign((void **)&stack, ucAlignment, stackSize) == 0);
    MapMemory(stack, stackAddress, stackSize, true, true, false);
  }
  static int threadId = 0;
  uint32_t esp = stackAddress + stackSize / 2 + 256 * 1024 * threadId++; // 256 kiB per late thread
  assert(threadId < 4);

  threads = realloc(threads, ++threadCount * sizeof(ThreadContext));
  ThreadContext* ctx = &threads[threadCount - 1];
  TransferContext(ctx, false); //FIXME: Find safe defaults instead?!
  ctx->eip = eip;
  ctx->esp = esp;
  ctx->ebp = 0;
  ctx->sleep = 0;
  PrintContext(ctx);
}

void SleepThread(uint64_t duration) {
  threads[currentThread].sleep = duration;
  uc_emu_stop(uc);
}

void DeleteEmulatedThread() {
  //FIXME: How to deal with deletion of the running thread?
}

static unsigned int GetThreadCount() {
  //FIXME: Protect with mutex?!
  return threadCount;
}

static int SliceThread(void* userData) {
  while(GetThreadCount() > 0) {
    // Run in 500ms timeslices, we don't use the shitty UC feature because it can't handle "while(1);"
    //FIXME: Disabled because it caused crashes
    //uc_emu_stop(uc);
    //thrd_sleep(&(struct timespec){.tv_nsec=500*1000*1000}, NULL);
    thrd_sleep(&(struct timespec){.tv_sec=5}, NULL);
  }
}

void RunEmulation() {
  uc_err err;

  thrd_t sliceThread;
  int tret = thrd_create(&sliceThread, SliceThread, NULL);

  //FIXME: plenty of options to optimize in single threaded mode.. (register readback not necessary etc.)
  while(GetThreadCount() > 0) {

    // Very simple round robin schedule.. Re-Volt only uses threads during load screens anyway..
    currentThread++;
    currentThread %= threadCount;

    // Get current thread
    ThreadContext* ctx = &threads[currentThread];

    //FIXME: Decrement time by time slice instead..
    if (ctx->sleep > 0) {
      printf("\n\n\n\n\nNot waking thread %d from sleep yet\n\n\n\n\n\n", currentThread);
      ctx->sleep -= 1;
      continue;
    }

    TransferContext(ctx, true);
    err = uc_emu_start(uc, ctx->eip, 0, 0, 0);

    // threads array might be relocated if a thread was modified in a callback; update ctx pointer
    ctx = &threads[currentThread];

    if (err) {
      printf("Failed on uc_emu_start() with error returned %u: %s\n", err, uc_strerror(err));

      ThreadContext ctx;
      TransferContext(&ctx, false);
      PrintContext(&ctx);

      assert(false);
    }

    TransferContext(ctx, false);

    printf("\n\n\n\n\nEmulation slice completed for thread %d (Count: %d) with %d at 0x%X\n", currentThread, threadCount, err, ctx->eip);

    PrintContext(ctx);
    printf("\n\n\n\n\n");
  }
  
  tret = thrd_join(sliceThread, NULL);

}

void CleanupEmulation(void) {
  uc_close(uc);
}
