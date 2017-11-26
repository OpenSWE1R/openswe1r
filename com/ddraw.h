#ifndef __OPENSWE1R_COM_DDRAW_H__
#define __OPENSWE1R_COM_DDRAW_H__

#include "../emulation.h"

#include "d3d.h"

typedef struct {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  union {
    uint32_t dwRGBBitCount;
    uint32_t dwYUVBitCount;
    uint32_t dwZBufferBitDepth;
    uint32_t dwAlphaBitDepth;
    uint32_t dwLuminanceBitCount;
    uint32_t dwBumpBitCount;
    uint32_t dwPrivateFormatBitCount;
  };
  union {
    uint32_t dwRBitMask;
    uint32_t dwYBitMask;
    uint32_t dwStencilBitDepth;
    uint32_t dwLuminanceBitMask;
    uint32_t dwBumpDuBitMask;
    uint32_t dwOperations;
  };
  union {
    uint32_t  dwGBitMask;
    uint32_t  dwUBitMask;
    uint32_t  dwZBitMask;
    uint32_t  dwBumpDvBitMask;
    struct {
      uint16_t wFlipMSTypes;
      uint16_t wBltMSTypes;
    } MultiSampleCaps;
  };
  union {
    uint32_t dwBBitMask;
    uint32_t dwVBitMask;
    uint32_t dwStencilBitMask;
    uint32_t dwBumpLuminanceBitMask;
  };
  union {
    uint32_t dwRGBAlphaBitMask;
    uint32_t dwYUVAlphaBitMask;
    uint32_t dwLuminanceAlphaBitMask;
    uint32_t dwRGBZBitMask;
    uint32_t dwYUVZBitMask;
  };
} DDPIXELFORMAT;


typedef struct {
  uint32_t dwCaps;
} DDSCAPS;


typedef struct {
  uint32_t dwCaps;
  uint32_t dwCaps2;
  uint32_t dwCaps3;
  union {
    uint32_t dwCaps4;
    uint32_t dwVolumeDepth;
  };
} DDSCAPS2;

#define DD_ROP_SPACE (256/32) // space required to store ROP array

typedef struct {
  uint32_t    dwSize;
  uint32_t    dwCaps;
  uint32_t    dwCaps2;
  uint32_t    dwCKeyCaps;
  uint32_t    dwFXCaps;
  uint32_t    dwFXAlphaCaps;
  uint32_t    dwPalCaps;
  uint32_t    dwSVCaps;
  uint32_t    dwAlphaBltConstBitDepths;
  uint32_t    dwAlphaBltPixelBitDepths;
  uint32_t    dwAlphaBltSurfaceBitDepths;
  uint32_t    dwAlphaOverlayConstBitDepths;
  uint32_t    dwAlphaOverlayPixelBitDepths;
  uint32_t    dwAlphaOverlaySurfaceBitDepths;
  uint32_t    dwZBufferBitDepths;
  uint32_t    dwVidMemTotal;
  uint32_t    dwVidMemFree;
  uint32_t    dwMaxVisibleOverlays;
  uint32_t    dwCurrVisibleOverlays;
  uint32_t    dwNumFourCCCodes;
  uint32_t    dwAlignBoundarySrc;
  uint32_t    dwAlignSizeSrc;
  uint32_t    dwAlignBoundaryDest;
  uint32_t    dwAlignSizeDest;
  uint32_t    dwAlignStrideAlign;
  uint32_t    dwRops[DD_ROP_SPACE];
  DDSCAPS  ddsOldCaps;
  uint32_t    dwMinOverlayStretch;
  uint32_t    dwMaxOverlayStretch;
  uint32_t    dwMinLiveVideoStretch;
  uint32_t    dwMaxLiveVideoStretch;
  uint32_t    dwMinHwCodecStretch;
  uint32_t    dwMaxHwCodecStretch;
  uint32_t    dwReserved1;
  uint32_t    dwReserved2;
  uint32_t    dwReserved3;
  uint32_t    dwSVBCaps;
  uint32_t    dwSVBCKeyCaps;
  uint32_t    dwSVBFXCaps;
  uint32_t    dwSVBRops[DD_ROP_SPACE];
  uint32_t    dwVSBCaps;
  uint32_t    dwVSBCKeyCaps;
  uint32_t    dwVSBFXCaps;
  uint32_t    dwVSBRops[DD_ROP_SPACE];
  uint32_t    dwSSBCaps;
  uint32_t    dwSSBCKeyCaps;
  uint32_t    dwSSBFXCaps;
  uint32_t    dwSSBRops[DD_ROP_SPACE];
  uint32_t    dwMaxVideoPorts;
  uint32_t    dwCurrVideoPorts;
  uint32_t    dwSVBCaps2;
  uint32_t    dwNLVBCaps;
  uint32_t    dwNLVBCaps2;
  uint32_t    dwNLVBCKeyCaps;
  uint32_t    dwNLVBFXCaps;
  uint32_t    dwNLVBRops[DD_ROP_SPACE];
  DDSCAPS2 ddsCaps;
} DDCAPS;

#define DDSCAPS_TEXTURE 0x00001000
#define DDSCAPS_MIPMAP  0x00400000

typedef struct {
  uint32_t dw1;
  uint32_t dw2;
} DDCOLORKEY;

typedef struct {
  uint32_t      dwSize;
  uint32_t      dwFlags;
  uint32_t      dwHeight;
  uint32_t      dwWidth;
  union {
    uint32_t  lPitch;
    uint32_t dwLinearSize;
  };
  union {
    uint32_t dwBackBufferCount;
    uint32_t dwDepth;
  };
  union {
    uint32_t dwMipMapCount;
    uint32_t dwRefreshRate;
    uint32_t dwSrcVBHandle;
  };
  uint32_t      dwAlphaBitDepth;
  uint32_t      dwReserved;
  Address     lpSurface;
  union {
    DDCOLORKEY ddckCKDestOverlay;
    uint32_t      dwEmptyFaceColor;
  };
  DDCOLORKEY ddckCKDestBlt;
  DDCOLORKEY ddckCKSrcOverlay;
  DDCOLORKEY ddckCKSrcBlt;
  union {
    DDPIXELFORMAT ddpfPixelFormat;
    uint32_t         dwFVF;
  };
  DDSCAPS2   ddsCaps;
  uint32_t      dwTextureStage;
} DDSURFACEDESC2;

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_RGB         0x00000040

typedef Address IDirectDrawSurface4[100];

typedef struct {
  IDirectDrawSurface4 vtable;
  Address texture; // Direct3DTexture2*
  DDSURFACEDESC2 desc;
} DirectDrawSurface4;

#endif
