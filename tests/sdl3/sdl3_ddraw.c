#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "ddraw.h"

static uint8_t wave(uint32_t t, uint32_t phase)
{
    uint32_t x = (t + phase) % 512u;
    return (uint8_t)(x < 256u ? x : (511u - x));
}

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("dxlib::ddraw SDL3 test", 960, 540, SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    LPDIRECTDRAW ddraw = NULL;
    HRESULT hr = DirectDrawCreate(NULL, &ddraw, NULL);
    if (hr != DD_OK || ddraw == NULL)
    {
        fprintf(stderr, "DirectDrawCreate failed: 0x%08x\n", (unsigned int)hr);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    hr = ddraw->lpVtbl->SetCooperativeLevel(ddraw, (HWND)window, 0u);
    if (hr != DD_OK)
    {
        fprintf(stderr, "SetCooperativeLevel failed: 0x%08x\n", (unsigned int)hr);
        ddraw->lpVtbl->Release(ddraw);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    DDSURFACEDESC primary_desc;
    SDL_memset(&primary_desc, 0, sizeof(primary_desc));
    primary_desc.dwSize = sizeof(primary_desc);
    primary_desc.dwFlags = DDSD_CAPS;
    primary_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    LPDIRECTDRAWSURFACE primary_surface = NULL;
    hr = ddraw->lpVtbl->CreateSurface(ddraw, &primary_desc, &primary_surface, NULL);
    if (hr != DD_OK || primary_surface == NULL)
    {
        fprintf(stderr, "CreateSurface failed: 0x%08x\n", (unsigned int)hr);
        ddraw->lpVtbl->Release(ddraw);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    uint32_t start_ticks = SDL_GetTicks();
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
            {
                running = false;
            }
        }

        uint32_t t = SDL_GetTicks() - start_ticks;
        if (t > 3000u)
        {
            running = false;
        }

        uint8_t r = wave(t / 4u, 0u);
        uint8_t g = wave(t / 4u, 170u);
        uint8_t b = wave(t / 4u, 341u);

        DDSURFACEDESC locked;
        SDL_memset(&locked, 0, sizeof(locked));
        locked.dwSize = sizeof(locked);
        hr = primary_surface->lpVtbl->Lock(primary_surface, NULL, &locked, DDLOCK_WAIT, NULL);
        if (hr != DD_OK || locked.lpSurface == NULL)
        {
            fprintf(stderr, "Surface Lock failed: 0x%08x\n", (unsigned int)hr);
            running = false;
            break;
        }

        uint32_t *pixels = (uint32_t *)locked.lpSurface;
        uint32_t pitch_pixels = (uint32_t)(locked.lPitch / (LONG)sizeof(uint32_t));
        uint32_t color = (0xFFu << 24u) | ((uint32_t)r << 16u) | ((uint32_t)g << 8u) | (uint32_t)b;

        for (uint32_t y = 0; y < locked.dwHeight; ++y)
        {
            for (uint32_t x = 0; x < locked.dwWidth; ++x)
            {
                pixels[y * pitch_pixels + x] = color;
            }
        }

        hr = primary_surface->lpVtbl->Unlock(primary_surface, NULL);
        if (hr != DD_OK)
        {
            fprintf(stderr, "Surface Unlock failed: 0x%08x\n", (unsigned int)hr);
            running = false;
            break;
        }

        SDL_Delay(16);
    }

    primary_surface->lpVtbl->Release(primary_surface);
    ddraw->lpVtbl->Release(ddraw);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
