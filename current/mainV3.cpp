#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <thread>

#include "include/Constants.h"
#include "include/Particle.h"
#include "include/RenderUtils.h"
#include "include/ParticleUtils.h"
#include "include/InputHandler.h"

using namespace std;

void singleThreadParticleComputation(atomic<bool> check[], vector<Particle> &particles,
                                     const int lowerChuckBound, const int upperChuckBound,
                                     const vector<vector<vector<int>>> &grid, const float dt) {
    for (int x = lowerChuckBound; x < upperChuckBound && x < screen::GRID_X; x++) {
        for (int y = 0; y < grid[x].size(); y++) {
            for (const auto a: grid[x][y]) {
                particles[a].update(dt);
                particles[a].checkWallCollision();
                const GridBounds currentGrid(x, y, screen::GRID_X - 1, screen::GRID_Y - 1);

                for (int x2 = currentGrid.left; x2 <= currentGrid.right; x2++) {
                    for (int y2 = currentGrid.up; y2 <= currentGrid.down; y2++) {
                        for (const auto b: grid[x2][y2]) {
                            if (a == b) continue;
                            if (glm::distance(particles[a].getPosition(), particles[b].getPosition()) <
                                particle::PARTICLES_CONTROL_DISTANCE) {
                                particles[a].resolveCollision(particles[b]);
                                check[a].store(true, memory_order_relaxed);
                                check[b].store(true, memory_order_relaxed);
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(const int argc, char *argv[]) {
    unsigned int N;
    if (argc < 2) {
        N = particle::DEFAULT_NUM_PARTICLES;
    } else {
        N = atoi(argv[1]); // NOLINT(*-err34-c) così non segnala il warning
    } //Assign particles value

    SDL_Window *window;
    SDL_Renderer *renderer;
    initializeSLD2(window, renderer);

    vector<vector<vector<int>>> grid(screen::GRID_X, vector<vector<int>>(screen::GRID_Y));

    // const short unsigned int numThreads = thread::hardware_concurrency();
    const short unsigned int numThreads = thread::hardware_concurrency();
    const short unsigned int chunk = screen::GRID_X/numThreads;
    vector<thread> threads;
    threads.reserve(numThreads);

    vector<Particle> particles;
    particles.reserve(N);
    initializeRandomParticles(particles, N);
    vector<SDL_Point> ptsCollided(N);
    vector<SDL_Point> ptsNotCollided(N);
    // ReSharper disable once CppTooWideScope
    atomic<bool> check[N];

    AppState appState;

    while (appState.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) handleInput(appState, event);

        const unsigned int currentTime = SDL_GetTicks();
        float dt = static_cast<float>(currentTime - appState.lastTime) / 1000.0f;
        dt *= appState.timeMultiplier;
        appState.lastTime = currentTime;

        appState.frameCount++;
        if (currentTime - appState.fpsTime >= 500) computeFps(appState.frameCount, appState.renderCount, currentTime, appState.fpsTime, appState.skipValue, appState.skipCount); //Fps and frameSkipping computation
        resizeWindow(window, renderer, appState.zoom);

        ptsCollided.clear();
        ptsNotCollided.clear();
        for (int i = 0; i < N; i++) check[i].store(false, memory_order_relaxed);
        gridInitialization(grid, particles, N);

        for(int t=0; t < numThreads; t++) {
            int startIndex = (t==0) ? 0 : t*chunk;
            int endIndex = (t==numThreads-1) ? static_cast<int>(N) : (t+1)*chunk;
            threads.emplace_back([&check, startIndex, endIndex, &particles, &grid, dt]() {
                singleThreadParticleComputation(check, particles, startIndex, endIndex, grid, dt);
            });
        } //Assign each tread its works
        for(auto &t: threads) {
            t.join();
        } //Wait all thread
        threads.clear();

        for (int i = 0; i < N; i++) {
            const glm::vec2 pos = particles[i].getPosition();
            SDL_Point p(static_cast<int>(pos.x), static_cast<int>(pos.y));
            if (check[i].load()) {
                ptsCollided.push_back(p);
            } else {
                ptsNotCollided.push_back(p);
            }
        } //Assign values to ptsCollide and ptsNotCollided

        if (appState.skipCount > 0) appState.skipCount--; //Frame skipping
        else{
            renderParticles(renderer, ptsCollided, ptsNotCollided);
            appState.renderCount++;
            appState.skipCount = appState.skipValue; //ripristina skipCount dopo aver renderizzato il frame
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}