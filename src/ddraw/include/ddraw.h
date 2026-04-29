#pragma once

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdint.h>

#ifndef WINAPI
#define WINAPI
#endif

#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

typedef uint32_t DWORD;
typedef uint32_t ULONG;
typedef int32_t LONG;
typedef int32_t HRESULT;
typedef void *LPVOID;

typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef void *HWND;
typedef void *HDC;

typedef int BOOL;

typedef struct _RGNDATAHEADER
{
    DWORD dwSize;
    DWORD iType;
    DWORD nCount;
    DWORD nRgnSize;
    RECT rcBound;
} RGNDATAHEADER;

typedef struct _RGNDATA
{
    RGNDATAHEADER rdh;
    char Buffer[1];
} RGNDATA;

typedef RGNDATA *LPRGNDATA;

#define RDH_RECTANGLES 1

typedef struct _GUID
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(DDRAW_STATIC)
#define DDRAW_API
#elif defined(_WIN32)
#if defined(DDRAW_EXPORTS)
#define DDRAW_API __declspec(dllexport)
#else
#define DDRAW_API __declspec(dllimport)
#endif
#else
#define DDRAW_API __attribute__((visibility("default")))
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0)
#endif

#ifndef E_NOINTERFACE
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#endif

#ifndef E_INVALIDARG
#define E_INVALIDARG ((HRESULT)0x80070057L)
#endif

#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT)0x80004001L)
#endif

#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#endif

#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif

#define DD_OK S_OK

    typedef struct IUnknown IUnknown;

    typedef struct IDirectDraw IDirectDraw;
    typedef struct IDirectDrawVtbl IDirectDrawVtbl;
    typedef IDirectDraw *LPDIRECTDRAW;

    typedef struct IDirectDrawSurface IDirectDrawSurface;
    typedef struct IDirectDrawSurfaceVtbl IDirectDrawSurfaceVtbl;
    typedef IDirectDrawSurface *LPDIRECTDRAWSURFACE;

    typedef struct IDirectDrawClipper IDirectDrawClipper;
    typedef struct IDirectDrawClipperVtbl IDirectDrawClipperVtbl;
    typedef IDirectDrawClipper *LPDIRECTDRAWCLIPPER;

    typedef struct _DDSCAPS
    {
        DWORD dwCaps;
    } DDSCAPS;

    typedef struct _DDSURFACEDESC
    {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwWidth;
        DWORD dwHeight;
        LONG lPitch;
        LPVOID lpSurface;
        DDSCAPS ddsCaps;
    } DDSURFACEDESC;

    typedef DDSURFACEDESC *LPDDSURFACEDESC;

    typedef struct _DDPIXELFORMAT
    {
        DWORD dwSize;
        DWORD dwFlags;
        DWORD dwFourCC;
        DWORD dwRGBBitCount;
        DWORD dwRBitMask;
        DWORD dwGBitMask;
        DWORD dwBBitMask;
        DWORD dwRGBAlphaBitMask;
    } DDPIXELFORMAT;

    typedef DDPIXELFORMAT *LPDDPIXELFORMAT;

#define DDPF_ALPHAPIXELS ((DWORD)0x00000001L)
#define DDPF_ALPHA ((DWORD)0x00000002L)
#define DDPF_FOURCC ((DWORD)0x00000004L)
#define DDPF_RGB ((DWORD)0x00000040L)

#define DDSD_CAPS ((DWORD)0x00000001L)
#define DDSD_HEIGHT ((DWORD)0x00000002L)
#define DDSD_WIDTH ((DWORD)0x00000004L)

#define DDSCAPS_PRIMARYSURFACE ((DWORD)0x00000001L)
#define DDSCAPS_OFFSCREENPLAIN ((DWORD)0x00000040L)

#define DDLOCK_WAIT ((DWORD)0x00000001L)

    struct IDirectDraw
    {
        IDirectDrawVtbl *lpVtbl;
    };

    struct IDirectDrawSurface
    {
        IDirectDrawSurfaceVtbl *lpVtbl;
    };

    struct IDirectDrawVtbl
    {
        HRESULT(STDMETHODCALLTYPE *QueryInterface)(LPDIRECTDRAW self, const GUID *riid, void **out_obj);
        ULONG(STDMETHODCALLTYPE *AddRef)(LPDIRECTDRAW self);
        ULONG(STDMETHODCALLTYPE *Release)(LPDIRECTDRAW self);
        HRESULT(STDMETHODCALLTYPE *SetCooperativeLevel)(LPDIRECTDRAW self, HWND hwnd, DWORD flags);
        HRESULT(STDMETHODCALLTYPE *SetDisplayMode)(LPDIRECTDRAW self, DWORD width, DWORD height, DWORD bpp);
        HRESULT(STDMETHODCALLTYPE *CreateSurface)(LPDIRECTDRAW self, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE *out_surface, IUnknown *outer_unknown);
        HRESULT(STDMETHODCALLTYPE *CreateClipper)(LPDIRECTDRAW self, DWORD flags, LPDIRECTDRAWCLIPPER *out_clipper, IUnknown *outer_unknown);
    };

    struct IDirectDrawSurfaceVtbl
    {
        HRESULT(STDMETHODCALLTYPE *QueryInterface)(LPDIRECTDRAWSURFACE self, const GUID *riid, void **out_obj);
        ULONG(STDMETHODCALLTYPE *AddRef)(LPDIRECTDRAWSURFACE self);
        ULONG(STDMETHODCALLTYPE *Release)(LPDIRECTDRAWSURFACE self);
        HRESULT(STDMETHODCALLTYPE *Lock)(LPDIRECTDRAWSURFACE self, RECT *rect, LPDDSURFACEDESC desc, DWORD flags, LPVOID event_handle);
        HRESULT(STDMETHODCALLTYPE *Unlock)(LPDIRECTDRAWSURFACE self, LPVOID rect_or_surface);
        HRESULT(STDMETHODCALLTYPE *SetClipper)(LPDIRECTDRAWSURFACE self, LPDIRECTDRAWCLIPPER clipper);
        HRESULT(STDMETHODCALLTYPE *GetClipper)(LPDIRECTDRAWSURFACE self, LPDIRECTDRAWCLIPPER *out_clipper);
    };

    struct IDirectDrawClipperVtbl
    {
        HRESULT(STDMETHODCALLTYPE *QueryInterface)(LPDIRECTDRAWCLIPPER self, const GUID *riid, void **out_obj);
        ULONG(STDMETHODCALLTYPE *AddRef)(LPDIRECTDRAWCLIPPER self);
        ULONG(STDMETHODCALLTYPE *Release)(LPDIRECTDRAWCLIPPER self);
        HRESULT(STDMETHODCALLTYPE *GetClipList)(LPDIRECTDRAWCLIPPER self, RECT *rect, LPRGNDATA clip_list, DWORD *size);
        HRESULT(STDMETHODCALLTYPE *GetHWnd)(LPDIRECTDRAWCLIPPER self, HWND *out_hwnd);
        HRESULT(STDMETHODCALLTYPE *Initialize)(LPDIRECTDRAWCLIPPER self, LPDIRECTDRAW ddraw, DWORD flags);
        HRESULT(STDMETHODCALLTYPE *IsClipListChanged)(LPDIRECTDRAWCLIPPER self, BOOL *out_changed);
        HRESULT(STDMETHODCALLTYPE *SetClipList)(LPDIRECTDRAWCLIPPER self, LPRGNDATA clip_list, DWORD flags);
        HRESULT(STDMETHODCALLTYPE *SetHWnd)(LPDIRECTDRAWCLIPPER self, DWORD flags, HWND hwnd);
    };

    struct IDirectDrawClipper
    {
        IDirectDrawClipperVtbl *lpVtbl;
    };

    DDRAW_API HRESULT WINAPI DirectDrawCreate(const GUID *lp_guid, LPDIRECTDRAW *out_ddraw, IUnknown *outer_unknown);
    DDRAW_API HRESULT WINAPI DirectDrawCreateEx(const GUID *lp_guid, void **out_obj, const GUID *iid, IUnknown *outer_unknown);
    DDRAW_API HRESULT WINAPI DirectDrawCreateClipper(DWORD flags, LPDIRECTDRAWCLIPPER *out_clipper, IUnknown *outer_unknown);

#ifdef __cplusplus
}
#endif
