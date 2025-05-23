#include "../include/RenderUtils.h"

#include <iostream>
#include <iomanip>
#include "../include/Constants.h"

using namespace std;

void initializeSLD2(SDL_Window *&window, SDL_Renderer *&renderer) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Ideal Gas Simulator", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, screen::SCREEN_WIDTH, screen::SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (SDL_Surface* icon = SDL_LoadBMP("../current/icon.bmp")) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    } //Set icon
    else cerr << "Failed to load icon: " << SDL_GetError() << endl;
}


void renderParticles(SDL_Renderer* renderer, const std::vector<SDL_Point> &ptsCollided, const std::vector<SDL_Point> &ptsNotCollided){
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderClear(renderer);

    //Particelle con collisioni avvenute
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    if (!ptsCollided.empty()) {
        SDL_RenderDrawPoints(renderer, ptsCollided.data(), static_cast<int>(ptsCollided.size()));
        //*.data() restituisce un puntatore al primo elemento del vettore, così si può accedere ad un vector come se fosse un array
    }

    //Particelle senza collisioni avvenute
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    if (!ptsNotCollided.empty()) {
        SDL_RenderDrawPoints(renderer, ptsNotCollided.data(), static_cast<int>(ptsNotCollided.size()));
    }
    SDL_RenderPresent(renderer);
}

void computeFps(unsigned short int &frameCount, unsigned short int &renderCount, const unsigned int currentTime, unsigned int &fpsTime, short int &skipValue, short int &skipCount) {
    const float fps = static_cast<float>(frameCount) / static_cast<float>(currentTime - fpsTime)*1000;
    const float fpsRender = static_cast<float>(renderCount) / static_cast<float>(currentTime - fpsTime)*1000;

    cout << fixed << setprecision(0); //fixed + precison si riferisci solo alle cifre decimali
    cout << "\rFPS: " << setw(6) << left << fps << "RENDER: " << setw(6) << left << fpsRender << flush; //stew obbliga ad usare alemno 6 caratteri per scrivere, left allinea a sinistra e flush "obbliga" la scrittura

    skipValue = static_cast<short>(static_cast<int>(fps / screen::TARGET_FPS) - 1);
    skipCount = skipValue;
    //l'idea e calcolare più fps possibili, ma renderizzarne soltanto un target(es: 60), quindi ad esempio se faccio 120 fps avrò 120/60 -1 = 1 quindi dovrò skipapre un fps
    frameCount = 0;
    renderCount = 0;
    fpsTime = currentTime;
}

void resizeWindow(SDL_Window* window, SDL_Renderer* renderer, const float zoom) {
    int windowWidth, windowHeight;
    // Ottieni le dimensioni attuali della finestra
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    // Imposta la scala del rendering per adattarsi alla dimensione della finestra
    SDL_RenderSetScale(renderer, static_cast<float>(windowWidth) / screen::SCREEN_WIDTH * zoom,
                       static_cast<float>(windowHeight) / screen::SCREEN_HEIGHT * zoom);
}