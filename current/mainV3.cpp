#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

#include "include/Constants.h"
#include "include/Particle.h"
#include "include/RenderUtils.h"
#include "include/ParticleUtils.h"

using namespace std;

struct GridBounds {
    int left, right, up, down;
    GridBounds(const int x, const int y, const int maxX, const int maxY) {
        this->left = max(0, x-1);
        this->right = min(x+1, maxX);
        this->up = max(0, y-1);
        this->down = min(y+1, maxY);
    }
};

void singleThreadParticleComputation(atomic<bool> check[], vector<Particle> &particles,
                                     const int lowerChuckBound, const int upperChuckBound,
                                     const vector<vector<vector<int>>> &grid, const float dt,
                                     const int &numGridX, const int &numGridY) {
    for (int x = lowerChuckBound; x < upperChuckBound && x < numGridX; x++) {
        for (int y = 0; y < grid[x].size(); y++) {
            for (const auto a: grid[x][y]) {
                particles[a].update(dt);
                particles[a].checkWallCollision();
                const GridBounds currentGrid(x, y, numGridX - 1, numGridY - 1);

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

    constexpr unsigned int numGridX = (screen::SCREEN_WIDTH / screen::CELL_SIZE);
    constexpr unsigned int numGridY = (screen::SCREEN_HEIGHT / screen::CELL_SIZE);
    vector<vector<vector<int>>> grid(numGridX, vector<vector<int>>(numGridY));

    // const short unsigned int numThreads = thread::hardware_concurrency();
    const short unsigned int numThreads = thread::hardware_concurrency();
    const short unsigned int chunk = numGridX/numThreads;
    vector<thread> threads;
    threads.reserve(numThreads);

    vector<Particle> particles;
    particles.reserve(N);
    initializeRandomParticles(particles, N);
    vector<SDL_Point> ptsCollided(N);
    vector<SDL_Point> ptsNotCollided(N);
    atomic<bool> check[N];

    AppState appState;

    while (appState.running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) appState.running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) appState.timeMultiplier = 0.2f;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LEFT) appState.timeMultiplier = 1.f;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) appState.timeMultiplier = 5.f;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RIGHT) appState.timeMultiplier = 1;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) appState.timeMultiplier = 0;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) appState.timeMultiplier = 1;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_PLUS) appState.zoom = min(appState.zoom + 0.1f, 10.0f);
                else if (event.key.keysym.sym == SDLK_MINUS) appState.zoom = max(appState.zoom - 0.1f, 1.f);
            }
        }

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
        gridInitialization(grid, particles, numGridX, numGridY, N);

        for(int t=0; t < numThreads; t++) {
            int startIndex = (t==0) ? 0 : t*chunk;
            int endIndex = (t==numThreads-1) ? static_cast<int>(N) : (t+1)*chunk;
            threads.emplace_back([&check, startIndex, endIndex, &particles, &grid, dt, numGridX, numGridY]() {
                singleThreadParticleComputation(check, particles, startIndex, endIndex, grid, dt, numGridX, numGridY);
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