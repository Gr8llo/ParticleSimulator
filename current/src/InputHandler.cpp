#include "../include/InputHandler.h"
#include <iostream>

void handleInput(AppState &appState, const SDL_Event &event) {
    if (event.type == SDL_QUIT) appState.running = false;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) appState.timeMultiplier = 0.2f;
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LEFT) appState.timeMultiplier = 1.f;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) appState.timeMultiplier = 5.f;
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RIGHT) appState.timeMultiplier = 1;
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) appState.timeMultiplier = 0;
    if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) appState.timeMultiplier = 1;
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_PLUS) appState.zoom = std::min(appState.zoom + 0.1f, 10.0f);
        else if (event.key.keysym.sym == SDLK_MINUS) appState.zoom = std::max(appState.zoom - 0.1f, 1.f);
    }
}
