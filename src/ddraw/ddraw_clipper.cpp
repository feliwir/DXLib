#include "ddraw_impl.h"

#include <new>

namespace
{

    HRESULT STDMETHODCALLTYPE ClipperQueryInterface(LPDIRECTDRAWCLIPPER self, const GUID *riid, void **out_obj)
    {
        if (self == nullptr || out_obj == nullptr)
        {
            return E_INVALIDARG;
        }

        if (riid == nullptr || IsEqualGuid(riid, &kIidIUnknown) || IsEqualGuid(riid, &kIidIDirectDrawClipper))
        {
            *out_obj = self;
            AsClipperImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed);
            return DD_OK;
        }

        *out_obj = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE ClipperAddRef(LPDIRECTDRAWCLIPPER self)
    {
        if (self == nullptr)
        {
            return 0;
        }
        return AsClipperImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    ULONG STDMETHODCALLTYPE ClipperRelease(LPDIRECTDRAWCLIPPER self)
    {
        if (self == nullptr)
        {
            return 0;
        }

        DirectDrawClipperImpl *impl = AsClipperImpl(self);
        ULONG remaining = impl->ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0)
        {
            delete impl;
        }
        return remaining;
    }

    HRESULT STDMETHODCALLTYPE ClipperGetClipList(LPDIRECTDRAWCLIPPER /*self*/, RECT * /*rect*/, LPRGNDATA /*clip_list*/, DWORD * /*size*/)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE ClipperGetHWnd(LPDIRECTDRAWCLIPPER self, HWND *out_hwnd)
    {
        if (self == nullptr || out_hwnd == nullptr)
        {
            return E_INVALIDARG;
        }

        *out_hwnd = AsClipperImpl(self)->hwnd;
        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE ClipperInitialize(LPDIRECTDRAWCLIPPER self, LPDIRECTDRAW /*ddraw*/, DWORD /*flags*/)
    {
        if (self == nullptr)
        {
            return E_INVALIDARG;
        }
        // Already initialized at creation time; subsequent calls return an error per DDraw spec.
        return E_FAIL;
    }

    HRESULT STDMETHODCALLTYPE ClipperIsClipListChanged(LPDIRECTDRAWCLIPPER self, BOOL *out_changed)
    {
        if (self == nullptr || out_changed == nullptr)
        {
            return E_INVALIDARG;
        }

        DirectDrawClipperImpl *impl = AsClipperImpl(self);
        *out_changed = impl->clip_list_changed ? 1 : 0;
        impl->clip_list_changed = false;
        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE ClipperSetClipList(LPDIRECTDRAWCLIPPER /*self*/, LPRGNDATA /*clip_list*/, DWORD /*flags*/)
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE ClipperSetHWnd(LPDIRECTDRAWCLIPPER self, DWORD /*flags*/, HWND hwnd)
    {
        if (self == nullptr)
        {
            return E_INVALIDARG;
        }

        DirectDrawClipperImpl *impl = AsClipperImpl(self);
        impl->hwnd = hwnd;
        impl->clip_list_changed = true;
        return DD_OK;
    }

    IDirectDrawClipperVtbl kClipperVtable = {
        &ClipperQueryInterface,
        &ClipperAddRef,
        &ClipperRelease,
        &ClipperGetClipList,
        &ClipperGetHWnd,
        &ClipperInitialize,
        &ClipperIsClipListChanged,
        &ClipperSetClipList,
        &ClipperSetHWnd,
    };

} // namespace

HRESULT DDraw_CreateClipperInstance(LPDIRECTDRAWCLIPPER *out_clipper)
{
    if (out_clipper == nullptr)
    {
        return E_INVALIDARG;
    }

    auto *impl = new (std::nothrow) DirectDrawClipperImpl{};
    if (impl == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    impl->iface.lpVtbl = &kClipperVtable;
    impl->ref_count.store(1, std::memory_order_relaxed);
    impl->hwnd = nullptr;
    impl->clip_list_changed = false;

    *out_clipper = &impl->iface;
    return DD_OK;
}

HRESULT WINAPI DirectDrawCreateClipper(DWORD /*flags*/, LPDIRECTDRAWCLIPPER *out_clipper, IUnknown *outer_unknown)
{
    if (outer_unknown != nullptr)
    {
        return E_NOTIMPL;
    }

    return DDraw_CreateClipperInstance(out_clipper);
}
