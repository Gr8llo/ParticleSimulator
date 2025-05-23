#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#include <SDL2/SDL.h>
#include <vector>

void initializeSLD2(SDL_Window* &window, SDL_Renderer* &renderer);

void resizeWindow(SDL_Window* window, SDL_Renderer* renderer, float zoom);

void renderParticles(SDL_Renderer* renderer, const std::vector<SDL_Point> &ptsCollided, const std::vector<SDL_Point> &ptsNotCollided);

void computeFps(unsigned short int &frameCount, unsigned short int &renderCount, unsigned int currentTime, unsigned int &fpsTime, short int &skipValue, short int &skipCount);

struct AppState {
    bool running = true;
    unsigned int lastTime = SDL_GetTicks();
    unsigned int fpsTime = lastTime;
    unsigned short int frameCount = 0;
    unsigned short int renderCount = 0;
    float timeMultiplier = 1.f;
    short int skipCount = 0;
    short int skipValue = 0;
    float zoom = 1.0f;
};
#endif //RENDERUTILS_H
