#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <stdio.h>

class SDLApp {
   public:
    SDLApp() {}
    void init();
    void update_framebuffer(const uint8_t* pixels);
    bool poll_input();  // updates inputs and also quit events

    ~SDLApp() {
        SDL_DestroyTexture(gSDLTexture);
        SDL_DestroyRenderer(gSDLRenderer);
        SDL_DestroyWindow(gSDLWindow);
        SDL_Quit();
    }

   private:
    SDL_Window* gSDLWindow;
    SDL_Renderer* gSDLRenderer;
    SDL_Texture* gSDLTexture;
    static int gDone;
    // int tex_width = 160;
    // int tex_height = 144;
    int tex_width = 32 * 8;
    int tex_height = 32 * 8;
};

void SDLApp::update_framebuffer(const uint8_t* pixels) {
    char* sdl_pixels;
    int pitch;

    SDL_LockTexture(gSDLTexture, NULL, (void**)&sdl_pixels, &pitch);
    memcpy(sdl_pixels, pixels, tex_width * tex_height * 4);  // 4 bytes per pixel

    SDL_UnlockTexture(gSDLTexture);
    SDL_RenderTexture(gSDLRenderer, gSDLTexture, NULL, NULL);
    SDL_RenderPresent(gSDLRenderer);
    // SDL_Delay(1);
}
bool SDLApp::poll_input() {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE) {
            return false;
        }
    }
    return true;
}

void SDLApp::init() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        throw std::runtime_error("Failed to initiate SDL!");
    }

    gSDLWindow = SDL_CreateWindow("SDL3 window", tex_width * 2, tex_height * 2, 0);
    gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
    gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_ABGR8888,
                                    SDL_TEXTUREACCESS_STREAMING, tex_width, tex_height);
    // SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_NEAREST);
    // SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureScaleMode(gSDLTexture, SDL_SCALEMODE_LINEAR);

    if (!gSDLWindow || !gSDLRenderer || !gSDLTexture) {
        throw std::runtime_error("Failed to initiate SDL rendering pipeline!");
    }
}