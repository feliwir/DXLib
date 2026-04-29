#pragma once

#include "ddraw.h"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <vulkan/vulkan.h>

#include <atomic>
#include <cstring>
#include <vector>

struct DirectDrawClipperImpl;

struct DirectDrawImpl
{
    IDirectDraw iface;
    std::atomic<ULONG> ref_count;
    SDL_Window *window;
    bool owns_window;
    SDL_Renderer *renderer;
    SDL_Texture *primary_texture;
    int texture_width;
    int texture_height;
    VkInstance vk_instance;
};

struct DirectDrawSurfaceImpl
{
    IDirectDrawSurface iface;
    std::atomic<ULONG> ref_count;
    DirectDrawImpl *owner;
    DirectDrawClipperImpl *clipper;
    bool is_primary;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    std::vector<uint8_t> pixels;
};

struct DirectDrawClipperImpl
{
    IDirectDrawClipper iface;
    std::atomic<ULONG> ref_count;
    HWND hwnd;
    bool clip_list_changed;
};

inline DirectDrawImpl *AsImpl(LPDIRECTDRAW self)
{
    return reinterpret_cast<DirectDrawImpl *>(self);
}

inline DirectDrawSurfaceImpl *AsSurfaceImpl(LPDIRECTDRAWSURFACE self)
{
    return reinterpret_cast<DirectDrawSurfaceImpl *>(self);
}

inline DirectDrawClipperImpl *AsClipperImpl(LPDIRECTDRAWCLIPPER self)
{
    return reinterpret_cast<DirectDrawClipperImpl *>(self);
}

inline bool IsEqualGuid(const GUID *a, const GUID *b)
{
    if (a == nullptr || b == nullptr)
    {
        return false;
    }
    return std::memcmp(a, b, sizeof(GUID)) == 0;
}

constexpr GUID kIidIUnknown = {
    0x00000000,
    0x0000,
    0x0000,
    {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46},
};

constexpr GUID kIidIDirectDraw = {
    0x6C14DB80,
    0xA733,
    0x11CE,
    {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60},
};

constexpr GUID kIidIDirectDrawSurface = {
    0x6C14DB81,
    0xA733,
    0x11CE,
    {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60},
};

constexpr GUID kIidIDirectDrawClipper = {
    0x593817A0,
    0x7DB3,
    0x11CF,
    {0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56},
};

// Implemented in ddraw_clipper.cpp
HRESULT DDraw_CreateClipperInstance(LPDIRECTDRAWCLIPPER *out_clipper);

// Implemented in ddraw_surface.cpp
HRESULT STDMETHODCALLTYPE DDrawCreateSurface(LPDIRECTDRAW self, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE *out_surface, IUnknown *outer_unknown);
