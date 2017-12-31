// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#ifndef __OPENSWE1R_COM_D3D_H__
#define __OPENSWE1R_COM_D3D_H__

#define API__DIRECT3D_VERSION 0x0600

#include "../emulation.h"
#include "../windows.h"

typedef struct {
  void* vtable;
  Address surface;
  GLuint handle;
} API(Direct3DTexture2);

typedef struct {
  union {
    uint32_t x1;
    uint32_t lX1;
  };
  union {
    uint32_t y1;
    uint32_t lY1;
  };
  union {
    uint32_t x2;
    uint32_t lX2;
  };
  union {
    uint32_t y2;
    uint32_t lY2;
  };
} API(D3DRECT);

enum {
  API(D3DCLEAR_TARGET) = 0x00000001,
  API(D3DCLEAR_ZBUFFER) = 0x00000002,
  API(D3DCLEAR_STENCIL) = 0x00000004
};

typedef float API(D3DVALUE); // Assumes 32 bit float

typedef struct {
    uint32_t       dwSize;
    uint32_t       dwX;
    uint32_t       dwY;        /* Viewport Top left */
    uint32_t       dwWidth;
    uint32_t       dwHeight;   /* Viewport Dimensions */
    API(D3DVALUE)    dvClipX;        /* Top left of clip volume */
    API(D3DVALUE)    dvClipY;
    API(D3DVALUE)    dvClipWidth;    /* Clip Volume Dimensions */
    API(D3DVALUE)    dvClipHeight;
    API(D3DVALUE)    dvMinZ;         /* Min/max of clip Volume */
    API(D3DVALUE)    dvMaxZ;
} API(D3DVIEWPORT2);


// From DX6 SDK

typedef enum {
    API(D3DRENDERSTATE_ANTIALIAS)         = 2,    /* D3DANTIALIASMODE */
    API(D3DRENDERSTATE_TEXTUREPERSPECTIVE)= 4,    /* TRUE for perspective correction */
    API(D3DRENDERSTATE_ZENABLE)           = 7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
    API(D3DRENDERSTATE_FILLMODE)          = 8,    /* D3DFILL_MODE        */
    API(D3DRENDERSTATE_SHADEMODE)         = 9,    /* D3DSHADEMODE */
    API(D3DRENDERSTATE_LINEPATTERN)       = 10,   /* D3DLINEPATTERN */
    API(D3DRENDERSTATE_ZWRITEENABLE)      = 14,   /* TRUE to enable z writes */
    API(D3DRENDERSTATE_ALPHATESTENABLE)   = 15,   /* TRUE to enable alpha tests */
    API(D3DRENDERSTATE_LASTPIXEL)         = 16,   /* TRUE for last-pixel on lines */
    API(D3DRENDERSTATE_SRCBLEND)          = 19,   /* D3DBLEND */
    API(D3DRENDERSTATE_DESTBLEND)         = 20,   /* D3DBLEND */
    API(D3DRENDERSTATE_CULLMODE)          = 22,   /* D3DCULL */
    API(D3DRENDERSTATE_ZFUNC)             = 23,   /* D3DCMPFUNC */
    API(D3DRENDERSTATE_ALPHAREF)          = 24,   /* D3DFIXED */
    API(D3DRENDERSTATE_ALPHAFUNC)         = 25,   /* D3DCMPFUNC */
    API(D3DRENDERSTATE_DITHERENABLE)      = 26,   /* TRUE to enable dithering */
#if(API__DIRECT3D_VERSION >= 0x0500)
    API(D3DRENDERSTATE_ALPHABLENDENABLE)  = 27,   /* TRUE to enable alpha blending */
#endif /* API__DIRECT3D_VERSION >= 0x0500 */
    API(D3DRENDERSTATE_FOGENABLE)         = 28,   /* TRUE to enable fog blending */
    API(D3DRENDERSTATE_SPECULARENABLE)    = 29,   /* TRUE to enable specular */
    API(D3DRENDERSTATE_ZVISIBLE)          = 30,   /* TRUE to enable z checking */
    API(D3DRENDERSTATE_STIPPLEDALPHA)     = 33,   /* TRUE to enable stippled alpha (RGB device only) */
    API(D3DRENDERSTATE_FOGCOLOR)          = 34,   /* D3DCOLOR */
    API(D3DRENDERSTATE_FOGTABLEMODE)      = 35,   /* D3DFOGMODE */
#if(API__DIRECT3D_VERSION >= 0x0700)
    API(D3DRENDERSTATE_FOGSTART)          = 36,   /* Fog start (for both vertex and pixel fog) */
    API(D3DRENDERSTATE_FOGEND)            = 37,   /* Fog end      */
    API(D3DRENDERSTATE_FOGDENSITY)        = 38,   /* Fog density  */
#endif /* API__DIRECT3D_VERSION >= 0x0700 */
#if(API__DIRECT3D_VERSION >= 0x0500)
    API(D3DRENDERSTATE_EDGEANTIALIAS)     = 40,   /* TRUE to enable edge antialiasing */
    API(D3DRENDERSTATE_COLORKEYENABLE)    = 41,   /* TRUE to enable source colorkeyed textures */
    API(D3DRENDERSTATE_ZBIAS)             = 47,   /* LONG Z bias */
    API(D3DRENDERSTATE_RANGEFOGENABLE)    = 48,   /* Enables range-based fog */
#endif /* API__DIRECT3D_VERSION >= 0x0500 */

#if(API__DIRECT3D_VERSION >= 0x0600)
    API(D3DRENDERSTATE_STENCILENABLE)     = 52,   /* BOOL enable/disable stenciling */
    API(D3DRENDERSTATE_STENCILFAIL)       = 53,   /* D3DSTENCILOP to do if stencil test fails */
    API(D3DRENDERSTATE_STENCILZFAIL)      = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
    API(D3DRENDERSTATE_STENCILPASS)       = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
    API(D3DRENDERSTATE_STENCILFUNC)       = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
    API(D3DRENDERSTATE_STENCILREF)        = 57,   /* Reference value used in stencil test */
    API(D3DRENDERSTATE_STENCILMASK)       = 58,   /* Mask value used in stencil test */
    API(D3DRENDERSTATE_STENCILWRITEMASK)  = 59,   /* Write mask applied to values written to stencil buffer */
    API(D3DRENDERSTATE_TEXTUREFACTOR)     = 60,   /* D3DCOLOR used for multi-texture blend */
#endif /* API__DIRECT3D_VERSION >= 0x0600 */

#if(API__DIRECT3D_VERSION >= 0x0600)

    /*
     * 128 values [128, 255] are reserved for texture coordinate wrap flags.
     * These are constructed with the D3DWRAP_U and D3DWRAP_V macros. Using
     * a flags uint16_t preserves forward compatibility with texture coordinates
     * that are >2D.
     */
    API(D3DRENDERSTATE_WRAP0)             = 128,  /* wrap for 1st texture coord. set */
    API(D3DRENDERSTATE_WRAP1)             = 129,  /* wrap for 2nd texture coord. set */
    API(D3DRENDERSTATE_WRAP2)             = 130,  /* wrap for 3rd texture coord. set */
    API(D3DRENDERSTATE_WRAP3)             = 131,  /* wrap for 4th texture coord. set */
    API(D3DRENDERSTATE_WRAP4)             = 132,  /* wrap for 5th texture coord. set */
    API(D3DRENDERSTATE_WRAP5)             = 133,  /* wrap for 6th texture coord. set */
    API(D3DRENDERSTATE_WRAP6)             = 134,  /* wrap for 7th texture coord. set */
    API(D3DRENDERSTATE_WRAP7)             = 135,  /* wrap for 8th texture coord. set */
#endif /* API__DIRECT3D_VERSION >= 0x0600 */
#if(API__DIRECT3D_VERSION >= 0x0700)
    API(D3DRENDERSTATE_CLIPPING)           = 136,
    API(D3DRENDERSTATE_LIGHTING)           = 137,
    API(D3DRENDERSTATE_EXTENTS)            = 138,
    API(D3DRENDERSTATE_AMBIENT)            = 139,
    API(D3DRENDERSTATE_FOGVERTEXMODE)      = 140,
    API(D3DRENDERSTATE_COLORVERTEX)        = 141,
    API(D3DRENDERSTATE_LOCALVIEWER)        = 142,
    API(D3DRENDERSTATE_NORMALIZENORMALS)   = 143,
    API(D3DRENDERSTATE_COLORKEYBLENDENABLE)     = 144,
    API(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE)   = 145,
    API(D3DRENDERSTATE_SPECULARMATERIALSOURCE)  = 146,
    API(D3DRENDERSTATE_AMBIENTMATERIALSOURCE)   = 147,
    API(D3DRENDERSTATE_EMISSIVEMATERIALSOURCE)  = 148,
    API(D3DRENDERSTATE_VERTEXBLEND)             = 151,
    API(D3DRENDERSTATE_CLIPPLANEENABLE)         = 152,

#endif /* API__DIRECT3D_VERSION >= 0x0700 */

//
// retired renderstates - not supported for DX7 interfaces
//
    API(D3DRENDERSTATE_TEXTUREHANDLE)     = 1,    /* Texture handle for legacy interfaces (Texture,Texture2) */
    API(D3DRENDERSTATE_TEXTUREADDRESS)    = 3,    /* D3DTEXTUREADDRESS  */
    API(D3DRENDERSTATE_WRAPU)             = 5,    /* TRUE for wrapping in u */
    API(D3DRENDERSTATE_WRAPV)             = 6,    /* TRUE for wrapping in v */
    API(D3DRENDERSTATE_MONOENABLE)        = 11,   /* TRUE to enable mono rasterization */
    API(D3DRENDERSTATE_ROP2)              = 12,   /* ROP2 */
    API(D3DRENDERSTATE_PLANEMASK)         = 13,   /* uint32_t physical plane mask */
    API(D3DRENDERSTATE_TEXTUREMAG)        = 17,   /* D3DTEXTUREFILTER */
    API(D3DRENDERSTATE_TEXTUREMIN)        = 18,   /* D3DTEXTUREFILTER */
    API(D3DRENDERSTATE_TEXTUREMAPBLEND)   = 21,   /* D3DTEXTUREBLEND */
    API(D3DRENDERSTATE_SUBPIXEL)          = 31,   /* TRUE to enable subpixel correction */
    API(D3DRENDERSTATE_SUBPIXELX)         = 32,   /* TRUE to enable correction in X only */
    API(D3DRENDERSTATE_STIPPLEENABLE)     = 39,   /* TRUE to enable stippling */
#if(API__DIRECT3D_VERSION >= 0x0500)
    API(D3DRENDERSTATE_BORDERCOLOR)       = 43,   /* Border color for texturing w/border */
    API(D3DRENDERSTATE_TEXTUREADDRESSU)   = 44,   /* Texture addressing mode for U coordinate */
    API(D3DRENDERSTATE_TEXTUREADDRESSV)   = 45,   /* Texture addressing mode for V coordinate */
    API(D3DRENDERSTATE_MIPMAPLODBIAS)     = 46,   /* D3DVALUE Mipmap LOD bias */
    API(D3DRENDERSTATE_ANISOTROPY)        = 49,   /* Max. anisotropy. 1 = no anisotropy */
#endif /* API__DIRECT3D_VERSION >= 0x0500 */
    API(D3DRENDERSTATE_FLUSHBATCH)        = 50,   /* Explicit flush for DP batching (DX5 Only) */
#if(API__DIRECT3D_VERSION >= 0x0600)
    API(D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT)=51, /* BOOL enable sort-independent transparency */
#endif /* API__DIRECT3D_VERSION >= 0x0600 */
    API(D3DRENDERSTATE_STIPPLEPATTERN00)  = 64,   /* Stipple pattern 01...  */
    API(D3DRENDERSTATE_STIPPLEPATTERN01)  = 65,
    API(D3DRENDERSTATE_STIPPLEPATTERN02)  = 66,
    API(D3DRENDERSTATE_STIPPLEPATTERN03)  = 67,
    API(D3DRENDERSTATE_STIPPLEPATTERN04)  = 68,
    API(D3DRENDERSTATE_STIPPLEPATTERN05)  = 69,
    API(D3DRENDERSTATE_STIPPLEPATTERN06)  = 70,
    API(D3DRENDERSTATE_STIPPLEPATTERN07)  = 71,
    API(D3DRENDERSTATE_STIPPLEPATTERN08)  = 72,
    API(D3DRENDERSTATE_STIPPLEPATTERN09)  = 73,
    API(D3DRENDERSTATE_STIPPLEPATTERN10)  = 74,
    API(D3DRENDERSTATE_STIPPLEPATTERN11)  = 75,
    API(D3DRENDERSTATE_STIPPLEPATTERN12)  = 76,
    API(D3DRENDERSTATE_STIPPLEPATTERN13)  = 77,
    API(D3DRENDERSTATE_STIPPLEPATTERN14)  = 78,
    API(D3DRENDERSTATE_STIPPLEPATTERN15)  = 79,
    API(D3DRENDERSTATE_STIPPLEPATTERN16)  = 80,
    API(D3DRENDERSTATE_STIPPLEPATTERN17)  = 81,
    API(D3DRENDERSTATE_STIPPLEPATTERN18)  = 82,
    API(D3DRENDERSTATE_STIPPLEPATTERN19)  = 83,
    API(D3DRENDERSTATE_STIPPLEPATTERN20)  = 84,
    API(D3DRENDERSTATE_STIPPLEPATTERN21)  = 85,
    API(D3DRENDERSTATE_STIPPLEPATTERN22)  = 86,
    API(D3DRENDERSTATE_STIPPLEPATTERN23)  = 87,
    API(D3DRENDERSTATE_STIPPLEPATTERN24)  = 88,
    API(D3DRENDERSTATE_STIPPLEPATTERN25)  = 89,
    API(D3DRENDERSTATE_STIPPLEPATTERN26)  = 90,
    API(D3DRENDERSTATE_STIPPLEPATTERN27)  = 91,
    API(D3DRENDERSTATE_STIPPLEPATTERN28)  = 92,
    API(D3DRENDERSTATE_STIPPLEPATTERN29)  = 93,
    API(D3DRENDERSTATE_STIPPLEPATTERN30)  = 94,
    API(D3DRENDERSTATE_STIPPLEPATTERN31)  = 95,

//
// retired renderstate names - the values are still used under new naming conventions
//
    API(D3DRENDERSTATE_FOGTABLESTART)     = 36,   /* Fog table start    */
    API(D3DRENDERSTATE_FOGTABLEEND)       = 37,   /* Fog table end      */
    API(D3DRENDERSTATE_FOGTABLEDENSITY)   = 38,   /* Fog table density  */

#if(API__DIRECT3D_VERSION >= 0x0500)
    API(D3DRENDERSTATE_FORCE_DWORD)       = 0x7fffffff, /* force 32-bit size enum */
#endif /* API__DIRECT3D_VERSION >= 0x0500 */
} API(D3DRENDERSTATETYPE);







typedef uint32_t API(D3DCOLORMODEL);


typedef struct {
    uint32_t dwSize;
    uint32_t dwMiscCaps;                 /* Capability flags */
    uint32_t dwRasterCaps;
    uint32_t dwZCmpCaps;
    uint32_t dwSrcBlendCaps;
    uint32_t dwDestBlendCaps;
    uint32_t dwAlphaCmpCaps;
    uint32_t dwShadeCaps;
    uint32_t dwTextureCaps;
    uint32_t dwTextureFilterCaps;
    uint32_t dwTextureBlendCaps;
    uint32_t dwTextureAddressCaps;
    uint32_t dwStippleWidth;             /* maximum width and height of */
    uint32_t dwStippleHeight;            /* of supported stipple (up to 32x32) */
} API(D3DPRIMCAPS);

typedef struct {
    uint32_t dwSize;
    uint32_t dwCaps;                   /* Lighting caps */
    uint32_t dwLightingModel;          /* Lighting model - RGB or mono */
    uint32_t dwNumLights;              /* Number of lights that can be handled */
} API(D3DLIGHTINGCAPS);

typedef struct {
    uint32_t dwSize;
    uint32_t dwCaps;
} API(D3DTRANSFORMCAPS);

typedef struct {
    uint32_t             dwSize;                 /* Size of D3DDEVICEDESC structure */
    uint32_t             dwFlags;                /* Indicates which fields have valid data */
    API(D3DCOLORMODEL)    dcmColorModel;          /* Color model of device */
    uint32_t             dwDevCaps;              /* Capabilities of device */
    API(D3DTRANSFORMCAPS) dtcTransformCaps;       /* Capabilities of transform */
    uint8_t              bClipping;              /* Device can do 3D clipping */
    API(D3DLIGHTINGCAPS)  dlcLightingCaps;        /* Capabilities of lighting */
    API(D3DPRIMCAPS)      dpcLineCaps;
    API(D3DPRIMCAPS)      dpcTriCaps;
    uint32_t             dwDeviceRenderBitDepth; /* One of DDBB_8, 16, etc.. */
    uint32_t             dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */
    uint32_t             dwMaxBufferSize;        /* Maximum execute buffer size */
    uint32_t             dwMaxVertexCount;       /* Maximum vertex count */
#if(API__DIRECT3D_VERSION >= 0x0500)
    // *** New fields for DX5 *** //

    // Width and height caps are 0 for legacy HALs.
    uint32_t        dwMinTextureWidth, dwMinTextureHeight;
    uint32_t        dwMaxTextureWidth, dwMaxTextureHeight;
    uint32_t        dwMinStippleWidth, dwMaxStippleWidth;
    uint32_t        dwMinStippleHeight, dwMaxStippleHeight;
#endif /* API__DIRECT3D_VERSION >= 0x0500 */

#if(API__DIRECT3D_VERSION >= 0x0600)
    // New fields for DX6
    uint32_t        dwMaxTextureRepeat;
    uint32_t        dwMaxTextureAspectRatio;
    uint32_t        dwMaxAnisotropy;

    // Guard band that the rasterizer can accommodate
    // Screen-space vertices inside this space but outside the viewport
    // will get clipped properly.
    API(D3DVALUE)    dvGuardBandLeft;
    API(D3DVALUE)    dvGuardBandTop;
    API(D3DVALUE)    dvGuardBandRight;
    API(D3DVALUE)    dvGuardBandBottom;

    API(D3DVALUE)    dvExtentsAdjust;
    uint32_t        dwStencilCaps;

    uint32_t        dwFVFCaps;
    uint32_t        dwTextureOpCaps;
    uint16_t        wMaxTextureBlendStages;
    uint16_t        wMaxSimultaneousTextures;
#endif /* API__DIRECT3D_VERSION >= 0x0600 */
} API(D3DDEVICEDESC);

typedef enum { 
  API(D3DBLEND_ZERO)             = 1,
  API(D3DBLEND_ONE)              = 2,
  API(D3DBLEND_SRCCOLOR)         = 3,
  API(D3DBLEND_INVSRCCOLOR)      = 4,
  API(D3DBLEND_SRCALPHA)         = 5,
  API(D3DBLEND_INVSRCALPHA)      = 6,
  API(D3DBLEND_DESTALPHA)        = 7,
  API(D3DBLEND_INVDESTALPHA)     = 8,
  API(D3DBLEND_DESTCOLOR)        = 9,
  API(D3DBLEND_INVDESTCOLOR)     = 10,
  API(D3DBLEND_SRCALPHASAT)      = 11,
  API(D3DBLEND_BOTHSRCALPHA)     = 12,
  API(D3DBLEND_BOTHINVSRCALPHA)  = 13,
  API(D3DBLEND_BLENDFACTOR)      = 14,
  API(D3DBLEND_INVBLENDFACTOR)   = 15,
  API(D3DBLEND_SRCCOLOR2)        = 16,
  API(D3DBLEND_INVSRCCOLOR2)     = 17
} API(D3DBLEND);

enum {
  API(DDCAPS_3D) =              0x00000001l,
  API(DDCAPS_BLTDEPTHFILL) =    0x10000000l
};

enum {
  API(DDCAPS2_CANRENDERWINDOWED) = 0x00080000l
};

enum {
  API(D3DPTBLENDCAPS_DECAL) =            0x00000001L,
  API(D3DPTBLENDCAPS_MODULATE) =         0x00000002L,
  API(D3DPTBLENDCAPS_DECALALPHA) =       0x00000004L,
  API(D3DPTBLENDCAPS_MODULATEALPHA) =    0x00000008L,
  API(D3DPTBLENDCAPS_DECALMASK) =        0x00000010L,
  API(D3DPTBLENDCAPS_MODULATEMASK) =     0x00000020L,
  API(D3DPTBLENDCAPS_COPY) =             0x00000040L,
  API(D3DPTBLENDCAPS_ADD) =              0x00000080L
};

// Subset of actual type
typedef enum {
  API(D3DANTIALIAS_NONE)          = 0
} API(D3DANTIALIASMODE);

// Subset of actual type
typedef enum {
  API(D3DFILL_SOLID)          = 3
} API(D3DFILLMODE);

// Subset of actual type
typedef enum {
  API(D3DSHADE_GOURAUD)           = 2
} API(D3DSHADEMODE);

// Subset of actual type
typedef enum {
  API(D3DTBLEND_MODULATE)         = 2,
  API(D3DTBLEND_MODULATEALPHA)    = 4
} API(D3DTEXTUREBLEND);

// Subset of actual type
typedef enum {
  API(D3DFILTER_MIPNEAREST)       = 3
} API(D3DTEXTUREFILTER);

// Subset of actual type
typedef enum {
  API(D3DCULL_NONE)               = 1
} API(D3DCULL);

// Subset of actual type
typedef enum {
  API(D3DCMP_LESSEQUAL)           = 4,
  API(D3DCMP_NOTEQUAL)            = 6
} API(D3DCMPFUNC);

// Subset of actual type
typedef enum {
  API(D3DFOG_NONE)                = 0,
  API(D3DFOG_LINEAR)              = 3
} API(D3DFOGMODE);

typedef enum {
  API(D3DTSS_COLOROP)        =  1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
  API(D3DTSS_COLORARG1)      =  2, /* D3DTA_* (texture arg) */
  API(D3DTSS_COLORARG2)      =  3, /* D3DTA_* (texture arg) */
  API(D3DTSS_ALPHAOP)        =  4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
  API(D3DTSS_ALPHAARG1)      =  5, /* D3DTA_* (texture arg) */
  API(D3DTSS_ALPHAARG2)      =  6, /* D3DTA_* (texture arg) */
  API(D3DTSS_BUMPENVMAT00)   =  7, /* D3DVALUE (bump mapping matrix) */
  API(D3DTSS_BUMPENVMAT01)   =  8, /* D3DVALUE (bump mapping matrix) */
  API(D3DTSS_BUMPENVMAT10)   =  9, /* D3DVALUE (bump mapping matrix) */
  API(D3DTSS_BUMPENVMAT11)   = 10, /* D3DVALUE (bump mapping matrix) */
  API(D3DTSS_TEXCOORDINDEX)  = 11, /* identifies which set of texture coordinates index this texture */
  API(D3DTSS_ADDRESS)        = 12, /* D3DTEXTUREADDRESS for both coordinates */
  API(D3DTSS_ADDRESSU)       = 13, /* D3DTEXTUREADDRESS for U coordinate */
  API(D3DTSS_ADDRESSV)       = 14, /* D3DTEXTUREADDRESS for V coordinate */
  API(D3DTSS_BORDERCOLOR)    = 15, /* D3DCOLOR */
  API(D3DTSS_MAGFILTER)      = 16, /* D3DTEXTUREMAGFILTER filter to use for magnification */
  API(D3DTSS_MINFILTER)      = 17, /* D3DTEXTUREMINFILTER filter to use for minification */
  API(D3DTSS_MIPFILTER)      = 18, /* D3DTEXTUREMIPFILTER filter to use between mipmaps during minification */
  API(D3DTSS_MIPMAPLODBIAS)  = 19, /* D3DVALUE Mipmap LOD bias */
  API(D3DTSS_MAXMIPLEVEL)    = 20, /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
  API(D3DTSS_MAXANISOTROPY)  = 21, /* DWORD maximum anisotropy */
  API(D3DTSS_BUMPENVLSCALE)  = 22, /* D3DVALUE scale for bump map luminance */
  API(D3DTSS_BUMPENVLOFFSET) = 23, /* D3DVALUE offset for bump map luminance */
  API(D3DTSS_FORCE_DWORD)   = 0x7fffffff /* force 32-bit size enum */
} API(D3DTEXTURESTAGESTATETYPE);

// Subset of actual type
typedef enum {
  API(D3DTFG_LINEAR)       = 2     // linear interpolation
} API(D3DTEXTUREMAGFILTER);

// Subset of actual type
typedef enum {
  API(D3DTFN_LINEAR)       = 2     // linear interpolation
} API(D3DTEXTUREMINFILTER);

// Subset of actual type
typedef enum {
  API(D3DTFP_NONE)         = 1     // mipmapping disabled (use MAG filter)
} API(D3DTEXTUREMIPFILTER);

// Subset of actual type
typedef enum {
  API(D3DTADDRESS_WRAP)           = 1,
  API(D3DTADDRESS_CLAMP)          = 3
} API(D3DTEXTUREADDRESS);

#endif
