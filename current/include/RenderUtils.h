#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#include <SDL2/SDL.h>
#include <vector>

void initializeSLD2(SDL_Window* &window, SDL_Renderer* &renderer);

void resizeWindow(SDL_Window* window, SDL_Renderer* renderer, float zoom);

void renderParticles(SDL_Renderer* renderer, const std::vector<SDL_Point> &ptsCollided, const std::vector<SDL_Point> &ptsNotCollided);

void computeFps(unsigned short int &frameCount, unsigned short int &renderCount, unsigned int currentTime, unsigned int &fpsTime, short int &skipValue, short int &skipCount);

#endif //RENDERUTILS_H
