#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <thread>
#include <algorithm>

#include "Constants.h"
#include "Particle.h"

inline float random(const float min, const float max) {
    static std::random_device r;
    static std::default_random_engine engine(r());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}
inline void initializeRandomParticles(std::vector<Particle> &particles, const unsigned int N) {
    constexpr float maxVelocity = constants::MAX_PARTICLES_VELOCITY;
    constexpr float maxHeightLimit = constants::SCREEN_HEIGHT/2.f + constants::CONTAINER_INITIAL_HEIGHT/2.f;
    constexpr float minHeightLimit = maxHeightLimit - constants::CONTAINER_INITIAL_HEIGHT;
    constexpr float maxWidthLimit = constants::SCREEN_WIDTH/2.f + constants::CONTAINER_INITIAL_WIDTH/2.f;
    constexpr float minWidthLimit = maxWidthLimit - constants::CONTAINER_INITIAL_WIDTH;
    for (int i = 0; i < N; i++) { //non si può usare particle.size() perchè ho usato reserve
        Particle p(
            i, glm::vec2(random(minWidthLimit, maxWidthLimit), random(minHeightLimit, maxHeightLimit)),
            glm::vec2(random(-maxVelocity, maxVelocity), random(-maxVelocity, maxVelocity)));
        particles.push_back(p);
    } //Generating casual position and velocity for each particle
}

struct GridBounds {
    int left, right, up, down;
    GridBounds(const int x, const int y, const int maxX, const int maxY) {
        this->left = std::max(0, x-1);
        this->right = std::min(x+1, maxX);
        this->up = std::max(0, y-1);
        this->down = std::min(y+1, maxY);
    }
};

void singleThreadParticleComputation(std::atomic<bool> check[], std::vector<Particle> &particles,
                                     const int lowerChuckBound, const int upperChuckBound,
                                     const std::vector<std::vector<std::vector<int>>> &grid, const float dt,
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
                                constants::PARTICLES_CONTROL_DISTANCE) {
                                particles[a].resolveCollision(particles[b]);
                                check[a].store(true, std::memory_order_relaxed);
                                check[b].store(true, std::memory_order_relaxed);
                            }
                        }
                    }
                }
            }
        }
    }
}

inline void renderParticles(SDL_Renderer* renderer, const std::vector<SDL_Point> &ptsCollided, const std::vector<SDL_Point> &ptsNotCollided){
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

int main(const int argc, char *argv[]) {
    unsigned int N;
    if (argc < 2) {
        N = constants::DEFAULT_NUM_PARTICLES;
    } else {
        N = atoi(argv[1]); // NOLINT(*-err34-c) così non segnala il warning
    } //Assign particles value

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Ideal Gas Simulator", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (SDL_Surface* icon = SDL_LoadBMP("../icon.bmp")) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    } //Set icon
    else std::cerr << "Failed to load icon: " << SDL_GetError() << std::endl;

    constexpr unsigned int numGridX = (constants::SCREEN_WIDTH / constants::CELL_SIZE);
    constexpr unsigned int numGridY = (constants::SCREEN_HEIGHT / constants::CELL_SIZE);
    std::vector<std::vector<std::vector<int>>> grid(numGridX, std::vector<std::vector<int>>(numGridY));

    const short unsigned int numThreads = std::thread::hardware_concurrency();
    const short unsigned int chunk = numGridX/numThreads;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    std::vector<Particle> particles;
    particles.reserve(N);
    initializeRandomParticles(particles, N);
    std::vector<SDL_Point> ptsCollided(N);
    std::vector<SDL_Point> ptsNotCollided(N);
    std::atomic<bool> check[N];

    bool running = true;
    unsigned int lastTime = SDL_GetTicks();
    unsigned int fpsTime = lastTime;
    unsigned int frameCount = 0;
    float timeMultiplier = 1.f;
    short int skipCount = 0;
    short int skipValue = 0;
    short int renderCount = 0;
    float zoom = 1.0f;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) timeMultiplier = 0.2f;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LEFT) timeMultiplier = 1.f;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) timeMultiplier = 5.f;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_RIGHT) timeMultiplier = 1;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) timeMultiplier = 0;
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) timeMultiplier = 1;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_PLUS) { // '+'
                    zoom = std::min(zoom + 0.1f, 10.0f);
                } else if (event.key.keysym.sym == SDLK_MINUS) { // '-'
                    zoom = std::max(zoom - 0.1f, 1.f);
                }
            }
        }

        unsigned int currentTime = SDL_GetTicks();
        float dt = static_cast<float>(currentTime - lastTime) / 1000.0f;
        dt *= timeMultiplier;
        lastTime = currentTime;

        frameCount++;
        if (currentTime - fpsTime >= 500) {
            float fps = static_cast<float>(frameCount) / static_cast<float>(currentTime - fpsTime)*1000;
            float fpsRender = static_cast<float>(renderCount) / static_cast<float>(currentTime - fpsTime)*1000;
            std::cout << "FPS: " << fps << "\t" << "RENDER: " << fpsRender << "\n"; //più efficente di std::endl
            skipValue = static_cast<short>(static_cast<int>(fps / constants::TARGET_FPS) - 1);
            skipCount = skipValue;
            //l'idea e calcolare più fps possibili, ma renderizzarne soltanto un target(es: 60), quindi ad esempio se faccio 120 fps avrò 120/60 -1 = 1 quindi dovrò skipapre un fps
            frameCount = 0;
            renderCount = 0;
            fpsTime = currentTime;
        }//Fps and frameSkipping computation

        {
            int windowWidth, windowHeight;
            // Ottieni le dimensioni attuali della finestra
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            // Imposta la scala del rendering per adattarsi alla dimensione della finestra
            SDL_RenderSetScale(renderer, static_cast<float>(windowWidth) / constants::SCREEN_WIDTH * zoom,
                               static_cast<float>(windowHeight) / constants::SCREEN_HEIGHT * zoom);
        } //Resize Window

        {
            ptsCollided.clear();
            ptsNotCollided.clear();
            for (int i = 0; i < N; i++) {
                check[i].store(false, std::memory_order_relaxed);
            }
            // std::sort(particles.begin(), particles.end(), [](const Particle &a, const Particle &b) {
            //     return a.getPosition().x < b.getPosition().x;
            // });
            for (int x = 0; x < numGridX; ++x) {
                for (int y = 0; y < numGridY; ++y) {
                    grid[x][y].clear();
                }
            }
            for (int i = 0; i < N; i++) {
                int xIndex = static_cast<int>(particles[i].getPosition().x) / constants::CELL_SIZE;
                int yIndex = static_cast<int>(particles[i].getPosition().y) / constants::CELL_SIZE;

                if (xIndex >= 0 && xIndex < numGridX && yIndex >= 0 && yIndex < numGridY) {
                    grid[xIndex][yIndex].push_back(i);
                }
            }
        } //Vector initialization

        for(int t=0; t < numThreads; t++) {
            int startIndex = (t==0) ? 0 : (t*chunk - constants::CHUNK_OVERLAP)  ;
            int endIndex = (t==numThreads-1) ? static_cast<int>(N) : ((t+1)*chunk + constants::CHUNK_OVERLAP);
            threads.emplace_back([&check, startIndex, endIndex, &particles, &grid, dt, numGridX, numGridY]() {
                singleThreadParticleComputation(check, particles, startIndex, endIndex, grid, dt, numGridX, numGridY);
            });
        } //Assign each tread its works

        for(auto &t: threads) {
            t.join();
        } //Wait all thread

        threads.clear();

        for (int i = 0; i < N; i++) {
            glm::vec2 pos = particles[i].getPosition();
            SDL_Point p(static_cast<int>(pos.x), static_cast<int>(pos.y));
            if (check[i].load()) {
                ptsCollided.push_back(p);
            } else {
                ptsNotCollided.push_back(p);
            }
        } //Assign values to ptsCollide and ptsNotCollided

        if (skipCount > 0) skipCount--; //Frame skipping
        else{
            renderParticles(renderer, ptsCollided, ptsNotCollided);
            renderCount++;
            skipCount = skipValue; //ripristina skipCount dopo aver renderizzatoil frame
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
