#include <stdio.h>

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_test_assert.h"
#include "SDL3/SDL_test_harness.h"

int video_getWindowSurface();

int main(void) {
    return video_getWindowSurface();
}

int video_getWindowSurface()
{
    const char *title = "video_getWindowSurface Test Window";
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    const char *renderer_name = NULL;
    int result;

    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "dummy") == 0) {
        renderer_name = SDL_SOFTWARE_RENDERER;
    }

    /* Make sure we're testing interaction with an accelerated renderer */
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");

    window = SDL_CreateWindow(title, 320, 320, 0);
    SDLTest_AssertPass("Call to SDL_CreateWindow('Title',320,320,0)");
    SDLTest_AssertCheck(window != NULL, "Validate that returned window is not NULL");

    surface = SDL_GetWindowSurface(window);
    SDLTest_AssertPass("Call to SDL_GetWindowSurface(window)");
    SDLTest_AssertCheck(surface != NULL, "Validate that returned surface is not NULL");
    SDLTest_AssertCheck(SDL_WindowHasSurface(window), "Validate that window has a surface");

    result = SDL_UpdateWindowSurface(window);
    SDLTest_AssertPass("Call to SDL_UpdateWindowSurface(window)");
    SDLTest_AssertCheck(result == true, "Verify return value; expected: true, got: %d", result);

    /* We shouldn't be able to create a renderer on a window with a surface */
    renderer = SDL_CreateRenderer(window, renderer_name);
    SDLTest_AssertPass("Call to SDL_CreateRenderer(window, %s)", renderer_name);
    SDLTest_AssertCheck(renderer == NULL, "Validate that returned renderer is NULL");

    result = SDL_DestroyWindowSurface(window);
    SDLTest_AssertPass("Call to SDL_DestroyWindowSurface(window)");
    SDLTest_AssertCheck(result == true, "Verify return value; expected: true, got: %d", result);
    SDLTest_AssertCheck(!SDL_WindowHasSurface(window), "Validate that window does not have a surface");

    /* We should be able to create a renderer on the window now */
    renderer = SDL_CreateRenderer(window, renderer_name);
    SDLTest_AssertPass("Call to SDL_CreateRenderer(window, %s)", renderer_name);
    SDLTest_AssertCheck(renderer != NULL, "Validate that returned renderer is not NULL");

    /* We should not be able to create a window surface now, unless it was created by the renderer */
    if (!SDL_WindowHasSurface(window)) {
        surface = SDL_GetWindowSurface(window);
        SDLTest_AssertPass("Call to SDL_GetWindowSurface(window)");
        SDLTest_AssertCheck(surface == NULL, "Validate that returned surface is NULL");
    }

    SDL_DestroyRenderer(renderer);
    SDLTest_AssertPass("Call to SDL_DestroyRenderer(renderer)");
    SDLTest_AssertCheck(!SDL_WindowHasSurface(window), "Validate that window does not have a surface");

    /* We should be able to create a window surface again */
    surface = SDL_GetWindowSurface(window);
    SDLTest_AssertPass("Call to SDL_GetWindowSurface(window)");
    SDLTest_AssertCheck(surface != NULL, "Validate that returned surface is not NULL");
    SDLTest_AssertCheck(SDL_WindowHasSurface(window), "Validate that window has a surface");

    /* Clean up */
    SDL_DestroyWindow(window);

    return TEST_COMPLETED;
}
