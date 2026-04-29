#include "ddraw_impl.h"

#include <SDL3/SDL_render.h>

#include <new>

namespace
{

    HRESULT PresentSurface(DirectDrawSurfaceImpl *surface)
    {
        if (surface == nullptr || surface->owner == nullptr || !surface->is_primary)
        {
            return DD_OK;
        }

        DirectDrawImpl *owner = surface->owner;
        if (owner->renderer == nullptr)
        {
            return E_FAIL;
        }

        int width = static_cast<int>(surface->width);
        int height = static_cast<int>(surface->height);
        if (width <= 0 || height <= 0)
        {
            return E_INVALIDARG;
        }

        if (owner->primary_texture == nullptr || owner->texture_width != width || owner->texture_height != height)
        {
            if (owner->primary_texture != nullptr)
            {
                SDL_DestroyTexture(owner->primary_texture);
                owner->primary_texture = nullptr;
            }

            owner->primary_texture = SDL_CreateTexture(owner->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
            if (owner->primary_texture == nullptr)
            {
                return E_FAIL;
            }

            owner->texture_width = width;
            owner->texture_height = height;
        }

        if (!SDL_UpdateTexture(owner->primary_texture, nullptr, surface->pixels.data(), static_cast<int>(surface->pitch)))
        {
            return E_FAIL;
        }

        if (!SDL_RenderClear(owner->renderer))
        {
            return E_FAIL;
        }

        if (!SDL_RenderTexture(owner->renderer, owner->primary_texture, nullptr, nullptr))
        {
            return E_FAIL;
        }

        if (!SDL_RenderPresent(owner->renderer))
        {
            return E_FAIL;
        }

        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE DDrawSurfaceQueryInterface(LPDIRECTDRAWSURFACE self, const GUID *riid, void **out_obj)
    {
        if (self == nullptr || out_obj == nullptr)
        {
            return E_INVALIDARG;
        }

        if (riid == nullptr || IsEqualGuid(riid, &kIidIUnknown) || IsEqualGuid(riid, &kIidIDirectDrawSurface))
        {
            *out_obj = self;
            AsSurfaceImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed);
            return DD_OK;
        }

        *out_obj = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE DDrawSurfaceAddRef(LPDIRECTDRAWSURFACE self)
    {
        if (self == nullptr)
        {
            return 0;
        }
        return AsSurfaceImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    ULONG STDMETHODCALLTYPE DDrawSurfaceRelease(LPDIRECTDRAWSURFACE self)
    {
        if (self == nullptr)
        {
            return 0;
        }

        DirectDrawSurfaceImpl *impl = AsSurfaceImpl(self);
        ULONG remaining = impl->ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0)
        {
            if (impl->clipper != nullptr)
            {
                impl->clipper->iface.lpVtbl->Release(&impl->clipper->iface);
                impl->clipper = nullptr;
            }
            if (impl->owner != nullptr)
            {
                impl->owner->iface.lpVtbl->Release(&impl->owner->iface);
                impl->owner = nullptr;
            }
            delete impl;
        }
        return remaining;
    }

    HRESULT STDMETHODCALLTYPE DDrawSurfaceLock(LPDIRECTDRAWSURFACE self, RECT *rect, LPDDSURFACEDESC desc, DWORD /*flags*/, LPVOID /*event_handle*/)
    {
        if (self == nullptr || desc == nullptr)
        {
            return E_INVALIDARG;
        }

        if (rect != nullptr)
        {
            return E_NOTIMPL;
        }

        DirectDrawSurfaceImpl *impl = AsSurfaceImpl(self);

        desc->dwSize = sizeof(*desc);
        desc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
        desc->dwWidth = impl->width;
        desc->dwHeight = impl->height;
        desc->lPitch = static_cast<LONG>(impl->pitch);
        desc->lpSurface = impl->pixels.data();
        desc->ddsCaps.dwCaps = impl->is_primary ? DDSCAPS_PRIMARYSURFACE : DDSCAPS_OFFSCREENPLAIN;
        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE DDrawSurfaceUnlock(LPDIRECTDRAWSURFACE self, LPVOID /*rect_or_surface*/)
    {
        if (self == nullptr)
        {
            return E_INVALIDARG;
        }

        return PresentSurface(AsSurfaceImpl(self));
    }

    HRESULT STDMETHODCALLTYPE DDrawSurfaceSetClipper(LPDIRECTDRAWSURFACE self, LPDIRECTDRAWCLIPPER clipper)
    {
        if (self == nullptr)
        {
            return E_INVALIDARG;
        }

        DirectDrawSurfaceImpl *impl = AsSurfaceImpl(self);

        if (clipper != nullptr)
        {
            clipper->lpVtbl->AddRef(clipper);
        }
        if (impl->clipper != nullptr)
        {
            impl->clipper->iface.lpVtbl->Release(&impl->clipper->iface);
        }
        impl->clipper = AsClipperImpl(clipper);
        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE DDrawSurfaceGetClipper(LPDIRECTDRAWSURFACE self, LPDIRECTDRAWCLIPPER *out_clipper)
    {
        if (self == nullptr || out_clipper == nullptr)
        {
            return E_INVALIDARG;
        }

        DirectDrawSurfaceImpl *impl = AsSurfaceImpl(self);
        if (impl->clipper == nullptr)
        {
            *out_clipper = nullptr;
            return DD_OK;
        }

        impl->clipper->iface.lpVtbl->AddRef(&impl->clipper->iface);
        *out_clipper = &impl->clipper->iface;
        return DD_OK;
    }

    IDirectDrawSurfaceVtbl kDdrawSurfaceVtable = {
        &DDrawSurfaceQueryInterface,
        &DDrawSurfaceAddRef,
        &DDrawSurfaceRelease,
        &DDrawSurfaceLock,
        &DDrawSurfaceUnlock,
        &DDrawSurfaceSetClipper,
        &DDrawSurfaceGetClipper,
    };

} // namespace

HRESULT STDMETHODCALLTYPE DDrawCreateSurface(LPDIRECTDRAW self, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE *out_surface, IUnknown *outer_unknown)
{
    if (self == nullptr || desc == nullptr || out_surface == nullptr)
    {
        return E_INVALIDARG;
    }

    if (outer_unknown != nullptr)
    {
        return E_NOTIMPL;
    }

    DirectDrawImpl *owner = AsImpl(self);
    bool is_primary = (desc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) != 0;

    uint32_t width = desc->dwWidth;
    uint32_t height = desc->dwHeight;
    if (is_primary && (width == 0 || height == 0) && owner->window != nullptr)
    {
        int w = 0;
        int h = 0;
        if (SDL_GetWindowSize(owner->window, &w, &h) && w > 0 && h > 0)
        {
            width = static_cast<uint32_t>(w);
            height = static_cast<uint32_t>(h);
        }
    }

    if (width == 0 || height == 0)
    {
        return E_INVALIDARG;
    }

    auto *surface = new (std::nothrow) DirectDrawSurfaceImpl{};
    if (surface == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    surface->iface.lpVtbl = &kDdrawSurfaceVtable;
    surface->ref_count.store(1, std::memory_order_relaxed);
    surface->owner = owner;
    surface->clipper = nullptr;
    surface->is_primary = is_primary;
    surface->width = width;
    surface->height = height;
    surface->pitch = width * 4u;

    try
    {
        surface->pixels.resize(static_cast<size_t>(surface->pitch) * static_cast<size_t>(height), 0u);
    }
    catch (std::bad_alloc &)
    {
        delete surface;
        return E_OUTOFMEMORY;
    }

    owner->iface.lpVtbl->AddRef(&owner->iface);

    desc->dwWidth = width;
    desc->dwHeight = height;
    desc->lPitch = static_cast<LONG>(surface->pitch);
    desc->lpSurface = surface->pixels.data();

    *out_surface = &surface->iface;
    return DD_OK;
}
