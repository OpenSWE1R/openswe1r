#include <stdint.h>
#include <stdbool.h>

//FIXME: Packed!
typedef union {
  struct {   
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t type:4;
    uint8_t system:1;      /* S flag */
    uint8_t dpl:2;
    uint8_t present:1;     /* P flag */
    uint8_t limit1:4;
    uint8_t avail:1;
    uint8_t is_64_code:1;  /* L flag */
    uint8_t db:1;          /* DB flag */
    uint8_t granularity:1; /* G flag */
    uint8_t base2;
  };
  uint64_t desc;
} SegmentDescriptor;

static uint32_t SegmentBase(SegmentDescriptor descriptor) {
  return (uint32_t)(((descriptor.desc >> 16) & 0xFFFFFF) | ((descriptor.desc >> 32) & 0xFF000000));
}

static uint32_t SegmentLimit(SegmentDescriptor descriptor) {
  return (descriptor.limit0 | (((uint32_t)descriptor.limit1) << 16));
}

static SegmentDescriptor CreateDescriptor(uint32_t base, uint32_t limit, bool code) {
  SegmentDescriptor descriptor = {0};

  descriptor.base0 = base & 0xFFFF;
  descriptor.base1 = (base >> 16) & 0xFF;
  descriptor.base2 = base >> 24;

  if (limit > 0xFFFFF) {
    limit >>= 12;
    descriptor.granularity = 1;
  }
  descriptor.limit0 = limit & 0xFFFF;
  descriptor.limit1 = limit >> 16;

  descriptor.dpl = 3;
  descriptor.present = 1;
  descriptor.db = 1;   //32 bit
  descriptor.type = code ? 0xB : 3;
  descriptor.system = 1;  //code or data

  return descriptor;
}
