#include "ddraw_impl.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_vulkan.h>

#include <new>
#include <string>

namespace
{

    SDL_Window *WindowFromHwnd(HWND hwnd, bool *owns_window)
    {
#if defined(_WIN32)
        if (owns_window == nullptr)
        {
            return nullptr;
        }

        SDL_PropertiesID props = SDL_CreateProperties();
        if (props == 0)
        {
            return nullptr;
        }

        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, hwnd);
        SDL_Window *window = SDL_CreateWindowWithProperties(props);
        SDL_DestroyProperties(props);
        *owns_window = (window != nullptr);
        return window;
#else
        if (owns_window != nullptr)
        {
            *owns_window = false;
        }
        return reinterpret_cast<SDL_Window *>(hwnd);
#endif
    }

    HRESULT InitVulkanInstance(DirectDrawImpl *impl)
    {
        if (impl == nullptr)
        {
            return E_INVALIDARG;
        }

        unsigned int ext_count = 0;
        char const *const *ext_names = SDL_Vulkan_GetInstanceExtensions(&ext_count);
        if (ext_names == nullptr || ext_count == 0)
        {
            return E_FAIL;
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "dxlib-ddraw";
        app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.pEngineName = "dxlib";
        app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.apiVersion = VK_API_VERSION_1_1;

        std::vector<char const *> extensions(ext_names, ext_names + ext_count);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        VkResult result = vkCreateInstance(&create_info, nullptr, &impl->vk_instance);
        if (result != VK_SUCCESS)
        {
            impl->vk_instance = VK_NULL_HANDLE;
            return E_FAIL;
        }

        return DD_OK;
    }

    void DestroyImpl(DirectDrawImpl *impl)
    {
        if (impl == nullptr)
        {
            return;
        }

        if (impl->primary_texture != nullptr)
        {
            SDL_DestroyTexture(impl->primary_texture);
            impl->primary_texture = nullptr;
        }

        if (impl->renderer != nullptr)
        {
            SDL_DestroyRenderer(impl->renderer);
            impl->renderer = nullptr;
        }

        if (impl->vk_instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(impl->vk_instance, nullptr);
            impl->vk_instance = VK_NULL_HANDLE;
        }

        if (impl->owns_window && impl->window != nullptr)
        {
            SDL_DestroyWindow(impl->window);
        }

        delete impl;
    }

    HRESULT STDMETHODCALLTYPE DDrawQueryInterface(LPDIRECTDRAW self, const GUID *riid, void **out_obj)
    {
        if (self == nullptr || out_obj == nullptr)
        {
            return E_INVALIDARG;
        }

        if (riid == nullptr || IsEqualGuid(riid, &kIidIUnknown) || IsEqualGuid(riid, &kIidIDirectDraw))
        {
            *out_obj = self;
            AsImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed);
            return DD_OK;
        }

        *out_obj = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE DDrawAddRef(LPDIRECTDRAW self)
    {
        if (self == nullptr)
        {
            return 0;
        }
        return AsImpl(self)->ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    ULONG STDMETHODCALLTYPE DDrawRelease(LPDIRECTDRAW self)
    {
        if (self == nullptr)
        {
            return 0;
        }

        DirectDrawImpl *impl = AsImpl(self);
        ULONG remaining = impl->ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (remaining == 0)
        {
            DestroyImpl(impl);
        }
        return remaining;
    }

    HRESULT STDMETHODCALLTYPE DDrawSetCooperativeLevel(LPDIRECTDRAW self, HWND hwnd, DWORD /*flags*/)
    {
        if (self == nullptr || hwnd == nullptr)
        {
            return E_INVALIDARG;
        }

        DirectDrawImpl *impl = AsImpl(self);

        if (impl->primary_texture != nullptr)
        {
            SDL_DestroyTexture(impl->primary_texture);
            impl->primary_texture = nullptr;
        }

        if (impl->renderer != nullptr)
        {
            SDL_DestroyRenderer(impl->renderer);
            impl->renderer = nullptr;
        }

#if !defined(_WIN32)
        impl->window = reinterpret_cast<SDL_Window *>(hwnd);
        impl->owns_window = false;
#else
        bool owns_window = false;
        SDL_Window *window = WindowFromHwnd(hwnd, &owns_window);
        if (window == nullptr)
        {
            return E_FAIL;
        }

        if (impl->owns_window && impl->window != nullptr && impl->window != window)
        {
            SDL_DestroyWindow(impl->window);
        }

        impl->window = window;
        impl->owns_window = owns_window;
#endif

        impl->renderer = SDL_CreateRenderer(impl->window, nullptr);
        if (impl->renderer == nullptr)
        {
            return E_FAIL;
        }

        impl->texture_width = 0;
        impl->texture_height = 0;
        return DD_OK;
    }

    HRESULT STDMETHODCALLTYPE DDrawSetDisplayMode(LPDIRECTDRAW /*self*/, DWORD /*width*/, DWORD /*height*/, DWORD /*bpp*/)
    {
        // Display mode switching is intentionally deferred for now.
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE DDrawCreateClipper(LPDIRECTDRAW self, DWORD /*flags*/, LPDIRECTDRAWCLIPPER *out_clipper, IUnknown *outer_unknown)
    {
        if (self == nullptr)
        {
            return E_INVALIDARG;
        }
        if (outer_unknown != nullptr)
        {
            return E_NOTIMPL;
        }
        return DDraw_CreateClipperInstance(out_clipper);
    }

    IDirectDrawVtbl kDdrawVtable = {
        &DDrawQueryInterface,
        &DDrawAddRef,
        &DDrawRelease,
        &DDrawSetCooperativeLevel,
        &DDrawSetDisplayMode,
        &DDrawCreateSurface,
        &DDrawCreateClipper,
    };

    HRESULT CreateDirectDrawInstance(LPDIRECTDRAW *out_ddraw)
    {
        if (out_ddraw == nullptr)
        {
            return E_INVALIDARG;
        }

        if (!SDL_WasInit(SDL_INIT_VIDEO) && !SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            return E_FAIL;
        }

        auto *impl = new (std::nothrow) DirectDrawImpl{};
        if (impl == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        impl->iface.lpVtbl = &kDdrawVtable;
        impl->ref_count.store(1, std::memory_order_relaxed);
        impl->window = nullptr;
        impl->owns_window = false;
        impl->renderer = nullptr;
        impl->primary_texture = nullptr;
        impl->texture_width = 0;
        impl->texture_height = 0;
        impl->vk_instance = VK_NULL_HANDLE;

        *out_ddraw = &impl->iface;
        return DD_OK;
    }

} // namespace

HRESULT WINAPI DirectDrawCreate(const GUID * /*lp_guid*/, LPDIRECTDRAW *out_ddraw, IUnknown *outer_unknown)
{
    if (outer_unknown != nullptr)
    {
        return E_NOTIMPL;
    }

    HRESULT hr = CreateDirectDrawInstance(out_ddraw);
    if (hr != DD_OK)
    {
        return hr;
    }

    HRESULT vk_hr = InitVulkanInstance(AsImpl(*out_ddraw));
    if (vk_hr != DD_OK)
    {
        (*out_ddraw)->lpVtbl->Release(*out_ddraw);
        *out_ddraw = nullptr;
        return vk_hr;
    }

    return DD_OK;
}

HRESULT WINAPI DirectDrawCreateEx(const GUID *lp_guid, void **out_obj, const GUID *iid, IUnknown *outer_unknown)
{
    if (out_obj == nullptr)
    {
        return E_INVALIDARG;
    }

    *out_obj = nullptr;

    LPDIRECTDRAW ddraw = nullptr;
    HRESULT hr = DirectDrawCreate(lp_guid, &ddraw, outer_unknown);
    if (hr != DD_OK)
    {
        return hr;
    }

    hr = ddraw->lpVtbl->QueryInterface(ddraw, iid, out_obj);
    ddraw->lpVtbl->Release(ddraw);
    return hr;
}