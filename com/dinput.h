#ifndef __OPENSWE1R_COM_DINPUT_H__
#define __OPENSWE1R_COM_DINPUT_H__

#define API__DIRECTINPUT_VERSION 0x0600

#include "../windows.h"

#include "../export.h"
#include "../emulation.h"

typedef struct {
  const API(GUID) *pguid;
  API(DWORD)   dwOfs;
  API(DWORD)   dwType;
  API(DWORD)   dwFlags;
} API(DIOBJECTDATAFORMAT);
typedef Address API(LPDIOBJECTDATAFORMAT);

typedef struct {
  API(DWORD)   dwSize;
  API(DWORD)   dwObjSize;
  API(DWORD)   dwFlags;
  API(DWORD)   dwDataSize;
  API(DWORD)   dwNumObjs;
  API(LPDIOBJECTDATAFORMAT) rgodf;
} API(DIDATAFORMAT);

typedef struct {
    API(DWORD)   dwSize;
    API(GUID)    guidInstance;
    API(GUID)    guidProduct;
    API(DWORD)   dwDevType;
    char    tszInstanceName[API(MAX_PATH)];
    char    tszProductName[API(MAX_PATH)];
#if (API__DIRECTINPUT_VERSION >= 0x0500)
    API(GUID)    guidFFDriver;
    uint16_t    wUsagePage;
    uint16_t    wUsage;
#endif
} API(DIDEVICEINSTANCEA);

typedef struct {
    API(DWORD)       dwOfs;
    API(DWORD)       dwData;
    API(DWORD)       dwTimeStamp;
    API(DWORD)       dwSequence;
#if (API__DIRECTINPUT_VERSION >= 0x0800)
    API(UINT_PTR)    uAppData;
#endif
} API(DIDEVICEOBJECTDATA);


// From Microsoft DX6 SDK headers

enum {
  API(DIK_ESCAPE) =          0x01,
  API(DIK_1) =               0x02,
  API(DIK_2) =               0x03,
  API(DIK_3) =               0x04,
  API(DIK_4) =               0x05,
  API(DIK_5) =               0x06,
  API(DIK_6) =               0x07,
  API(DIK_7) =               0x08,
  API(DIK_8) =               0x09,
  API(DIK_9) =               0x0A,
  API(DIK_0) =               0x0B,
  API(DIK_MINUS) =           0x0C,    /* - on main keyboard */
  API(DIK_EQUALS) =          0x0D,
  API(DIK_BACK) =            0x0E,    /* backspace */
  API(DIK_TAB) =             0x0F,
  API(DIK_Q) =               0x10,
  API(DIK_W) =               0x11,
  API(DIK_E) =               0x12,
  API(DIK_R) =               0x13,
  API(DIK_T) =               0x14,
  API(DIK_Y) =               0x15,
  API(DIK_U) =               0x16,
  API(DIK_I) =               0x17,
  API(DIK_O) =               0x18,
  API(DIK_P) =               0x19,
  API(DIK_LBRACKET) =        0x1A,
  API(DIK_RBRACKET) =        0x1B,
  API(DIK_RETURN) =          0x1C,    /* Enter on main keyboard */
  API(DIK_LCONTROL) =        0x1D,
  API(DIK_A) =               0x1E,
  API(DIK_S) =               0x1F,
  API(DIK_D) =               0x20,
  API(DIK_F) =               0x21,
  API(DIK_G) =               0x22,
  API(DIK_H) =               0x23,
  API(DIK_J) =               0x24,
  API(DIK_K) =               0x25,
  API(DIK_L) =               0x26,
  API(DIK_SEMICOLON) =       0x27,
  API(DIK_APOSTROPHE) =      0x28,
  API(DIK_GRAVE) =           0x29,    /* accent grave */
  API(DIK_LSHIFT) =          0x2A,
  API(DIK_BACKSLASH) =       0x2B,
  API(DIK_Z) =               0x2C,
  API(DIK_X) =               0x2D,
  API(DIK_C) =               0x2E,
  API(DIK_V) =               0x2F,
  API(DIK_B) =               0x30,
  API(DIK_N) =               0x31,
  API(DIK_M) =               0x32,
  API(DIK_COMMA) =           0x33,
  API(DIK_PERIOD) =          0x34,    /* . on main keyboard */
  API(DIK_SLASH) =           0x35,    /* / on main keyboard */
  API(DIK_RSHIFT) =          0x36,
  API(DIK_MULTIPLY) =        0x37,    /* * on numeric keypad */
  API(DIK_LMENU) =           0x38,    /* left Alt */
  API(DIK_SPACE) =           0x39,
  API(DIK_CAPITAL) =         0x3A,
  API(DIK_F1) =              0x3B,
  API(DIK_F2) =              0x3C,
  API(DIK_F3) =              0x3D,
  API(DIK_F4) =              0x3E,
  API(DIK_F5) =              0x3F,
  API(DIK_F6) =              0x40,
  API(DIK_F7) =              0x41,
  API(DIK_F8) =              0x42,
  API(DIK_F9) =              0x43,
  API(DIK_F10) =             0x44,
  API(DIK_NUMLOCK) =         0x45,
  API(DIK_SCROLL) =          0x46,    /* Scroll Lock */
  API(DIK_NUMPAD7) =         0x47,
  API(DIK_NUMPAD8) =         0x48,
  API(DIK_NUMPAD9) =         0x49,
  API(DIK_SUBTRACT) =        0x4A,    /* - on numeric keypad */
  API(DIK_NUMPAD4) =         0x4B,
  API(DIK_NUMPAD5) =         0x4C,
  API(DIK_NUMPAD6) =         0x4D,
  API(DIK_ADD) =             0x4E,    /* + on numeric keypad */
  API(DIK_NUMPAD1) =         0x4F,
  API(DIK_NUMPAD2) =         0x50,
  API(DIK_NUMPAD3) =         0x51,
  API(DIK_NUMPAD0) =         0x52,
  API(DIK_DECIMAL) =         0x53,    /* . on numeric keypad */
  API(DIK_OEM_102) =         0x56,    /* < > | on UK/Germany keyboards */
  API(DIK_F11) =             0x57,
  API(DIK_F12) =             0x58,

  API(DIK_F13) =             0x64,    /*                     (NEC PC98) */
  API(DIK_F14) =             0x65,    /*                     (NEC PC98) */
  API(DIK_F15) =             0x66,    /*                     (NEC PC98) */

  API(DIK_KANA) =            0x70,    /* (Japanese keyboard)            */
  API(DIK_ABNT_C1) =         0x73,    /* / ? on Portugese (Brazilian) keyboards */
  API(DIK_CONVERT) =         0x79,    /* (Japanese keyboard)            */
  API(DIK_NOCONVERT) =       0x7B,    /* (Japanese keyboard)            */
  API(DIK_YEN) =             0x7D,    /* (Japanese keyboard)            */
  API(DIK_ABNT_C2) =         0x7E,    /* Numpad . on Portugese (Brazilian) keyboards */
  API(DIK_NUMPADEQUALS) =    0x8D,    /* = on numeric keypad (NEC PC98) */
  API(DIK_PREVTRACK) =       0x90,    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
  API(DIK_AT) =              0x91,    /*                     (NEC PC98) */
  API(DIK_COLON) =           0x92,    /*                     (NEC PC98) */
  API(DIK_UNDERLINE) =       0x93,    /*                     (NEC PC98) */
  API(DIK_KANJI) =           0x94,    /* (Japanese keyboard)            */
  API(DIK_STOP) =            0x95,    /*                     (NEC PC98) */
  API(DIK_AX) =              0x96,    /*                     (Japan AX) */
  API(DIK_UNLABELED) =       0x97,    /*                        (J3100) */
  API(DIK_NEXTTRACK) =       0x99,    /* Next Track */
  API(DIK_NUMPADENTER) =     0x9C,    /* Enter on numeric keypad */
  API(DIK_RCONTROL) =        0x9D,
  API(DIK_MUTE) =            0xA0,    /* Mute */
  API(DIK_CALCULATOR) =      0xA1,    /* Calculator */
  API(DIK_PLAYPAUSE) =       0xA2,    /* Play / Pause */
  API(DIK_MEDIASTOP) =       0xA4,    /* Media Stop */
  API(DIK_VOLUMEDOWN) =      0xAE,    /* Volume - */
  API(DIK_VOLUMEUP) =        0xB0,    /* Volume + */
  API(DIK_WEBHOME) =         0xB2,    /* Web home */
  API(DIK_NUMPADCOMMA) =     0xB3,    /* , on numeric keypad (NEC PC98) */
  API(DIK_DIVIDE) =          0xB5,    /* / on numeric keypad */
  API(DIK_SYSRQ) =           0xB7,
  API(DIK_RMENU) =           0xB8,    /* right Alt */
  API(DIK_PAUSE) =           0xC5,    /* Pause */
  API(DIK_HOME) =            0xC7,    /* Home on arrow keypad */
  API(DIK_UP) =              0xC8,    /* UpArrow on arrow keypad */
  API(DIK_PRIOR) =           0xC9,    /* PgUp on arrow keypad */
  API(DIK_LEFT) =            0xCB,    /* LeftArrow on arrow keypad */
  API(DIK_RIGHT) =           0xCD,    /* RightArrow on arrow keypad */
  API(DIK_END) =             0xCF,    /* End on arrow keypad */
  API(DIK_DOWN) =            0xD0,    /* DownArrow on arrow keypad */
  API(DIK_NEXT) =            0xD1,    /* PgDn on arrow keypad */
  API(DIK_INSERT) =          0xD2,    /* Insert on arrow keypad */
  API(DIK_DELETE) =          0xD3,    /* Delete on arrow keypad */
  API(DIK_LWIN) =            0xDB,    /* Left Windows key */
  API(DIK_RWIN) =            0xDC,    /* Right Windows key */
  API(DIK_APPS) =            0xDD,    /* AppMenu key */
  API(DIK_POWER) =           0xDE,    /* System Power */
  API(DIK_SLEEP) =           0xDF,    /* System Sleep */
  API(DIK_WAKE) =            0xE3,    /* System Wake */
  API(DIK_WEBSEARCH) =       0xE5,    /* Web Search */
  API(DIK_WEBFAVORITES) =    0xE6,    /* Web Favorites */
  API(DIK_WEBREFRESH) =      0xE7,    /* Web Refresh */
  API(DIK_WEBSTOP) =         0xE8,    /* Web Stop */
  API(DIK_WEBFORWARD) =      0xE9,    /* Web Forward */
  API(DIK_WEBBACK) =         0xEA,    /* Web Back */
  API(DIK_MYCOMPUTER) =      0xEB,    /* My Computer */
  API(DIK_MAIL) =            0xEC,    /* Mail */
  API(DIK_MEDIASELECT) =     0xED,    /* Media Select */

/*
 *  Alternate names for keys, to facilitate transition from DOS.
 */
  API(DIK_BACKSPACE) =       API(DIK_BACK),            /* backspace */
  API(DIK_NUMPADSTAR) =      API(DIK_MULTIPLY),        /* * on numeric keypad */
  API(DIK_LALT) =            API(DIK_LMENU),           /* left Alt */
  API(DIK_CAPSLOCK) =        API(DIK_CAPITAL),         /* CapsLock */
  API(DIK_NUMPADMINUS) =     API(DIK_SUBTRACT),        /* - on numeric keypad */
  API(DIK_NUMPADPLUS) =      API(DIK_ADD),             /* + on numeric keypad */
  API(DIK_NUMPADPERIOD) =    API(DIK_DECIMAL),         /* . on numeric keypad */
  API(DIK_NUMPADSLASH) =     API(DIK_DIVIDE),          /* / on numeric keypad */
  API(DIK_RALT) =            API(DIK_RMENU),           /* right Alt */
  API(DIK_UPARROW) =         API(DIK_UP),              /* UpArrow on arrow keypad */
  API(DIK_PGUP) =            API(DIK_PRIOR),           /* PgUp on arrow keypad */
  API(DIK_LEFTARROW) =       API(DIK_LEFT),            /* LeftArrow on arrow keypad */
  API(DIK_RIGHTARROW) =      API(DIK_RIGHT),           /* RightArrow on arrow keypad */
  API(DIK_DOWNARROW) =       API(DIK_DOWN),            /* DownArrow on arrow keypad */
  API(DIK_PGDN) =            API(DIK_NEXT),            /* PgDn on arrow keypad */

/*
 *  Alternate names for keys originally not used on US keyboards.
 */
  API(DIK_CIRCUMFLEX) =      API(DIK_PREVTRACK)        /* Japanese keyboard */
};

#endif
