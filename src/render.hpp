#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <stdio.h>

class SDLApp {
   public:
    SDLApp() {}
    void init();
    void update_framebuffer(uint8_t pixels, int width, int height);
    bool poll_input();  // updates inputs and also quit events

    ~SDLApp() {
        SDL_DestroyTexture(gSDLTexture);
        SDL_DestroyRenderer(gSDLRenderer);
        SDL_DestroyWindow(gSDLWindow);
        SDL_Quit();
    }

   private:
    int* gFrameBuffer;
    SDL_Window* gSDLWindow;
    SDL_Renderer* gSDLRenderer;
    SDL_Texture* gSDLTexture;
    static int gDone;
    const int WINDOW_WIDTH = 1920 / 2;
    const int WINDOW_HEIGHT = 1080 / 2;
};

void SDLApp::update_framebuffer(uint8_t pixels, int width, int height) {
    char* sdl_pixels;
    int pitch;

    SDL_LockTexture(gSDLTexture, NULL, (void**)&sdl_pixels, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < WINDOW_HEIGHT; i++, dp += WINDOW_WIDTH, sp += pitch)
        memcpy(sdl_pixels + sp, gFrameBuffer + dp, WINDOW_WIDTH * 4);

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

    gFrameBuffer = new int[WINDOW_WIDTH * WINDOW_HEIGHT];
    gSDLWindow = SDL_CreateWindow("SDL3 window", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    gSDLRenderer = SDL_CreateRenderer(gSDLWindow, NULL);
    gSDLTexture = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_ABGR8888,
                                    SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!gFrameBuffer || !gSDLWindow || !gSDLRenderer || !gSDLTexture) {
        throw std::runtime_error("Failed to initiate SDL rendering pipeline!");
    }
}