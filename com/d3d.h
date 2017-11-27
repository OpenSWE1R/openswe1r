#ifndef __OPENSWE1R_COM_D3D_H__
#define __OPENSWE1R_COM_D3D_H__

#include "../emulation.h"
#include "../windows.h"

typedef struct {
  void* vtable;
  Address surface;
  GLuint handle;
} Direct3DTexture2;

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
} D3DRECT;

#define D3DCLEAR_TARGET  0x00000001
#define D3DCLEAR_ZBUFFER 0x00000002
#define D3DCLEAR_STENCIL 0x00000004

typedef float D3DVALUE; // Assumes 32 bit float

typedef struct {
    uint32_t       dwSize;
    uint32_t       dwX;
    uint32_t       dwY;        /* Viewport Top left */
    uint32_t       dwWidth;
    uint32_t       dwHeight;   /* Viewport Dimensions */
    D3DVALUE    dvClipX;        /* Top left of clip volume */
    D3DVALUE    dvClipY;
    D3DVALUE    dvClipWidth;    /* Clip Volume Dimensions */
    D3DVALUE    dvClipHeight;
    D3DVALUE    dvMinZ;         /* Min/max of clip Volume */
    D3DVALUE    dvMaxZ;
} D3DVIEWPORT2;


// From DX6 SDK

typedef enum _D3DRENDERSTATETYPE {
    D3DRENDERSTATE_ANTIALIAS          = 2,    /* D3DANTIALIASMODE */
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,    /* TRUE for perspective correction */
    D3DRENDERSTATE_ZENABLE            = 7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
    D3DRENDERSTATE_FILLMODE           = 8,    /* D3DFILL_MODE        */
    D3DRENDERSTATE_SHADEMODE          = 9,    /* D3DSHADEMODE */
    D3DRENDERSTATE_LINEPATTERN        = 10,   /* D3DLINEPATTERN */
    D3DRENDERSTATE_ZWRITEENABLE       = 14,   /* TRUE to enable z writes */
    D3DRENDERSTATE_ALPHATESTENABLE    = 15,   /* TRUE to enable alpha tests */
    D3DRENDERSTATE_LASTPIXEL          = 16,   /* TRUE for last-pixel on lines */
    D3DRENDERSTATE_SRCBLEND           = 19,   /* D3DBLEND */
    D3DRENDERSTATE_DESTBLEND          = 20,   /* D3DBLEND */
    D3DRENDERSTATE_CULLMODE           = 22,   /* D3DCULL */
    D3DRENDERSTATE_ZFUNC              = 23,   /* D3DCMPFUNC */
    D3DRENDERSTATE_ALPHAREF           = 24,   /* D3DFIXED */
    D3DRENDERSTATE_ALPHAFUNC          = 25,   /* D3DCMPFUNC */
    D3DRENDERSTATE_DITHERENABLE       = 26,   /* TRUE to enable dithering */
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_ALPHABLENDENABLE   = 27,   /* TRUE to enable alpha blending */
#endif /* DIRECT3D_VERSION >= 0x0500 */
    D3DRENDERSTATE_FOGENABLE          = 28,   /* TRUE to enable fog blending */
    D3DRENDERSTATE_SPECULARENABLE     = 29,   /* TRUE to enable specular */
    D3DRENDERSTATE_ZVISIBLE           = 30,   /* TRUE to enable z checking */
    D3DRENDERSTATE_STIPPLEDALPHA      = 33,   /* TRUE to enable stippled alpha (RGB device only) */
    D3DRENDERSTATE_FOGCOLOR           = 34,   /* D3DCOLOR */
    D3DRENDERSTATE_FOGTABLEMODE       = 35,   /* D3DFOGMODE */
#if(DIRECT3D_VERSION >= 0x0700)
    D3DRENDERSTATE_FOGSTART           = 36,   /* Fog start (for both vertex and pixel fog) */
    D3DRENDERSTATE_FOGEND             = 37,   /* Fog end      */
    D3DRENDERSTATE_FOGDENSITY         = 38,   /* Fog density  */
#endif /* DIRECT3D_VERSION >= 0x0700 */
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_EDGEANTIALIAS      = 40,   /* TRUE to enable edge antialiasing */
    D3DRENDERSTATE_COLORKEYENABLE     = 41,   /* TRUE to enable source colorkeyed textures */
    D3DRENDERSTATE_ZBIAS              = 47,   /* LONG Z bias */
    D3DRENDERSTATE_RANGEFOGENABLE     = 48,   /* Enables range-based fog */
#endif /* DIRECT3D_VERSION >= 0x0500 */

#if(DIRECT3D_VERSION >= 0x0600)
    D3DRENDERSTATE_STENCILENABLE      = 52,   /* BOOL enable/disable stenciling */
    D3DRENDERSTATE_STENCILFAIL        = 53,   /* D3DSTENCILOP to do if stencil test fails */
    D3DRENDERSTATE_STENCILZFAIL       = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
    D3DRENDERSTATE_STENCILPASS        = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
    D3DRENDERSTATE_STENCILFUNC        = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
    D3DRENDERSTATE_STENCILREF         = 57,   /* Reference value used in stencil test */
    D3DRENDERSTATE_STENCILMASK        = 58,   /* Mask value used in stencil test */
    D3DRENDERSTATE_STENCILWRITEMASK   = 59,   /* Write mask applied to values written to stencil buffer */
    D3DRENDERSTATE_TEXTUREFACTOR      = 60,   /* D3DCOLOR used for multi-texture blend */
#endif /* DIRECT3D_VERSION >= 0x0600 */

#if(DIRECT3D_VERSION >= 0x0600)

    /*
     * 128 values [128, 255] are reserved for texture coordinate wrap flags.
     * These are constructed with the D3DWRAP_U and D3DWRAP_V macros. Using
     * a flags uint16_t preserves forward compatibility with texture coordinates
     * that are >2D.
     */
    D3DRENDERSTATE_WRAP0              = 128,  /* wrap for 1st texture coord. set */
    D3DRENDERSTATE_WRAP1              = 129,  /* wrap for 2nd texture coord. set */
    D3DRENDERSTATE_WRAP2              = 130,  /* wrap for 3rd texture coord. set */
    D3DRENDERSTATE_WRAP3              = 131,  /* wrap for 4th texture coord. set */
    D3DRENDERSTATE_WRAP4              = 132,  /* wrap for 5th texture coord. set */
    D3DRENDERSTATE_WRAP5              = 133,  /* wrap for 6th texture coord. set */
    D3DRENDERSTATE_WRAP6              = 134,  /* wrap for 7th texture coord. set */
    D3DRENDERSTATE_WRAP7              = 135,  /* wrap for 8th texture coord. set */
#endif /* DIRECT3D_VERSION >= 0x0600 */
#if(DIRECT3D_VERSION >= 0x0700)
    D3DRENDERSTATE_CLIPPING            = 136,
    D3DRENDERSTATE_LIGHTING            = 137,
    D3DRENDERSTATE_EXTENTS             = 138,
    D3DRENDERSTATE_AMBIENT             = 139,
    D3DRENDERSTATE_FOGVERTEXMODE       = 140,
    D3DRENDERSTATE_COLORVERTEX         = 141,
    D3DRENDERSTATE_LOCALVIEWER         = 142,
    D3DRENDERSTATE_NORMALIZENORMALS    = 143,
    D3DRENDERSTATE_COLORKEYBLENDENABLE = 144,
    D3DRENDERSTATE_DIFFUSEMATERIALSOURCE    = 145,
    D3DRENDERSTATE_SPECULARMATERIALSOURCE   = 146,
    D3DRENDERSTATE_AMBIENTMATERIALSOURCE    = 147,
    D3DRENDERSTATE_EMISSIVEMATERIALSOURCE   = 148,
    D3DRENDERSTATE_VERTEXBLEND              = 151,
    D3DRENDERSTATE_CLIPPLANEENABLE          = 152,

#endif /* DIRECT3D_VERSION >= 0x0700 */

//
// retired renderstates - not supported for DX7 interfaces
//
    D3DRENDERSTATE_TEXTUREHANDLE      = 1,    /* Texture handle for legacy interfaces (Texture,Texture2) */
    D3DRENDERSTATE_TEXTUREADDRESS     = 3,    /* D3DTEXTUREADDRESS  */
    D3DRENDERSTATE_WRAPU              = 5,    /* TRUE for wrapping in u */
    D3DRENDERSTATE_WRAPV              = 6,    /* TRUE for wrapping in v */
    D3DRENDERSTATE_MONOENABLE         = 11,   /* TRUE to enable mono rasterization */
    D3DRENDERSTATE_ROP2               = 12,   /* ROP2 */
    D3DRENDERSTATE_PLANEMASK          = 13,   /* uint32_t physical plane mask */
    D3DRENDERSTATE_TEXTUREMAG         = 17,   /* D3DTEXTUREFILTER */
    D3DRENDERSTATE_TEXTUREMIN         = 18,   /* D3DTEXTUREFILTER */
    D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,   /* D3DTEXTUREBLEND */
    D3DRENDERSTATE_SUBPIXEL           = 31,   /* TRUE to enable subpixel correction */
    D3DRENDERSTATE_SUBPIXELX          = 32,   /* TRUE to enable correction in X only */
    D3DRENDERSTATE_STIPPLEENABLE      = 39,   /* TRUE to enable stippling */
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_BORDERCOLOR        = 43,   /* Border color for texturing w/border */
    D3DRENDERSTATE_TEXTUREADDRESSU    = 44,   /* Texture addressing mode for U coordinate */
    D3DRENDERSTATE_TEXTUREADDRESSV    = 45,   /* Texture addressing mode for V coordinate */
    D3DRENDERSTATE_MIPMAPLODBIAS      = 46,   /* D3DVALUE Mipmap LOD bias */
    D3DRENDERSTATE_ANISOTROPY         = 49,   /* Max. anisotropy. 1 = no anisotropy */
#endif /* DIRECT3D_VERSION >= 0x0500 */
    D3DRENDERSTATE_FLUSHBATCH         = 50,   /* Explicit flush for DP batching (DX5 Only) */
#if(DIRECT3D_VERSION >= 0x0600)
    D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT=51, /* BOOL enable sort-independent transparency */
#endif /* DIRECT3D_VERSION >= 0x0600 */
    D3DRENDERSTATE_STIPPLEPATTERN00   = 64,   /* Stipple pattern 01...  */
    D3DRENDERSTATE_STIPPLEPATTERN01   = 65,
    D3DRENDERSTATE_STIPPLEPATTERN02   = 66,
    D3DRENDERSTATE_STIPPLEPATTERN03   = 67,
    D3DRENDERSTATE_STIPPLEPATTERN04   = 68,
    D3DRENDERSTATE_STIPPLEPATTERN05   = 69,
    D3DRENDERSTATE_STIPPLEPATTERN06   = 70,
    D3DRENDERSTATE_STIPPLEPATTERN07   = 71,
    D3DRENDERSTATE_STIPPLEPATTERN08   = 72,
    D3DRENDERSTATE_STIPPLEPATTERN09   = 73,
    D3DRENDERSTATE_STIPPLEPATTERN10   = 74,
    D3DRENDERSTATE_STIPPLEPATTERN11   = 75,
    D3DRENDERSTATE_STIPPLEPATTERN12   = 76,
    D3DRENDERSTATE_STIPPLEPATTERN13   = 77,
    D3DRENDERSTATE_STIPPLEPATTERN14   = 78,
    D3DRENDERSTATE_STIPPLEPATTERN15   = 79,
    D3DRENDERSTATE_STIPPLEPATTERN16   = 80,
    D3DRENDERSTATE_STIPPLEPATTERN17   = 81,
    D3DRENDERSTATE_STIPPLEPATTERN18   = 82,
    D3DRENDERSTATE_STIPPLEPATTERN19   = 83,
    D3DRENDERSTATE_STIPPLEPATTERN20   = 84,
    D3DRENDERSTATE_STIPPLEPATTERN21   = 85,
    D3DRENDERSTATE_STIPPLEPATTERN22   = 86,
    D3DRENDERSTATE_STIPPLEPATTERN23   = 87,
    D3DRENDERSTATE_STIPPLEPATTERN24   = 88,
    D3DRENDERSTATE_STIPPLEPATTERN25   = 89,
    D3DRENDERSTATE_STIPPLEPATTERN26   = 90,
    D3DRENDERSTATE_STIPPLEPATTERN27   = 91,
    D3DRENDERSTATE_STIPPLEPATTERN28   = 92,
    D3DRENDERSTATE_STIPPLEPATTERN29   = 93,
    D3DRENDERSTATE_STIPPLEPATTERN30   = 94,
    D3DRENDERSTATE_STIPPLEPATTERN31   = 95,

//
// retired renderstate names - the values are still used under new naming conventions
//
    D3DRENDERSTATE_FOGTABLESTART      = 36,   /* Fog table start    */
    D3DRENDERSTATE_FOGTABLEEND        = 37,   /* Fog table end      */
    D3DRENDERSTATE_FOGTABLEDENSITY    = 38,   /* Fog table density  */

#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_FORCE_uint32_t        = 0x7fffffff, /* force 32-bit size enum */
#endif /* DIRECT3D_VERSION >= 0x0500 */
} D3DRENDERSTATETYPE;







typedef uint32_t D3DCOLORMODEL;


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
} D3DPRIMCAPS;

typedef struct {
    uint32_t dwSize;
    uint32_t dwCaps;                   /* Lighting caps */
    uint32_t dwLightingModel;          /* Lighting model - RGB or mono */
    uint32_t dwNumLights;              /* Number of lights that can be handled */
} D3DLIGHTINGCAPS;

typedef struct {
    uint32_t dwSize;
    uint32_t dwCaps;
} D3DTRANSFORMCAPS;

typedef struct {
    uint32_t            dwSize;                 /* Size of D3DDEVICEDESC structure */
    uint32_t            dwFlags;                /* Indicates which fields have valid data */
    D3DCOLORMODEL    dcmColorModel;          /* Color model of device */
    uint32_t            dwDevCaps;              /* Capabilities of device */
    D3DTRANSFORMCAPS dtcTransformCaps;       /* Capabilities of transform */
    uint8_t             bClipping;              /* Device can do 3D clipping */
    D3DLIGHTINGCAPS  dlcLightingCaps;        /* Capabilities of lighting */
    D3DPRIMCAPS      dpcLineCaps;
    D3DPRIMCAPS      dpcTriCaps;
    uint32_t            dwDeviceRenderBitDepth; /* One of DDBB_8, 16, etc.. */
    uint32_t            dwDeviceZBufferBitDepth;/* One of DDBD_16, 32, etc.. */
    uint32_t            dwMaxBufferSize;        /* Maximum execute buffer size */
    uint32_t            dwMaxVertexCount;       /* Maximum vertex count */
#if(DIRECT3D_VERSION >= 0x0500)
    // *** New fields for DX5 *** //

    // Width and height caps are 0 for legacy HALs.
    uint32_t        dwMinTextureWidth, dwMinTextureHeight;
    uint32_t        dwMaxTextureWidth, dwMaxTextureHeight;
    uint32_t        dwMinStippleWidth, dwMaxStippleWidth;
    uint32_t        dwMinStippleHeight, dwMaxStippleHeight;
#endif /* DIRECT3D_VERSION >= 0x0500 */

#if(DIRECT3D_VERSION >= 0x0600)
    // New fields for DX6
    uint32_t       dwMaxTextureRepeat;
    uint32_t       dwMaxTextureAspectRatio;
    uint32_t       dwMaxAnisotropy;

    // Guard band that the rasterizer can accommodate
    // Screen-space vertices inside this space but outside the viewport
    // will get clipped properly.
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;

    D3DVALUE    dvExtentsAdjust;
    uint32_t       dwStencilCaps;

    uint32_t       dwFVFCaps;
    uint32_t       dwTextureOpCaps;
    uint16_t        wMaxTextureBlendStages;
    uint16_t        wMaxSimultaneousTextures;
#endif /* DIRECT3D_VERSION >= 0x0600 */
} D3DDEVICEDESC;





#endif
