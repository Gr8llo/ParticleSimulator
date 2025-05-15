#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <thread>

#include "Constants.h"
#include "Particle.h"

float random(float lim) {
    static std::random_device r;
    static std::default_random_engine engine(r());
    std::uniform_real_distribution<float> dist(0, lim);
    return dist(engine);
}

float randomV(float lim) {
    static std::random_device r;
    static std::default_random_engine engine(r());
    std::uniform_real_distribution<float> dist(lim * -1, lim);
    return dist(engine);
}

void singleThreadParticleComputation(std::atomic<bool> check[], std::vector<Particle> &particles, const int lowerChuckBound, const int upperChuckBound, const float dt) {
    for (int i = lowerChuckBound; i < upperChuckBound && i<particles.size(); i++) {
        particles[i].update(dt);
        particles[i].checkWallCollision();
        const float x_i = particles[i].getPosition().x;
        for (int j = i + 1; j < upperChuckBound && j < particles.size(); j++) {
            float x_j = particles[j].getPosition().x;
            if (x_j - x_i > constants::PARTICLES_CONTROL_DISTANCE) {
                break;
            }
            if (particles[i].resolveCollision(particles[j])) {
                check[i].store(true, std::memory_order_relaxed);
                check[j].store(true, std::memory_order_relaxed);
            }
        }
    }
}

int main(const int argc, char *argv[]) {
    unsigned int N;
    if (argc < 2) {
        N = constants::DEFAULT_NUM_PARTICLES;
    } else {
        N = atoi(argv[1]);
    } //Assign particles value

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Collisione di particelle", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::vector<Particle> particles;
    particles.reserve(N);
    for (int i = 0; i < N; i++) {
        Particle p(
            i, glm::vec2(random(constants::CONTAINER_INITIAL_WIDTH), random(constants::CONTAINER_INITIAL_HEIGHT)),
            glm::vec2(randomV(constants::MAX_PARTICLES_VELOCITY), randomV(constants::MAX_PARTICLES_VELOCITY)));
        particles.push_back(p);
    }//Generating casual position and velocity for each particle
    std::vector<SDL_Point> ptsCollided(N);
    std::vector<SDL_Point> ptsNotCollided(N);
    std::atomic<bool> check[N];

    const short unsigned int numThreads = std::thread::hardware_concurrency();
    const short unsigned int chunk = N/numThreads;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    bool running = true;
    unsigned int lastTime = SDL_GetTicks();
    unsigned int fpsTime = lastTime;
    unsigned int frameCount = 0;
    float timeMultiplier = 1.f;
    int skipCount = 0;
    int skipValue = 0;
    int renderCount = 0;

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
        }

        unsigned int currentTime = SDL_GetTicks();
        float dt = static_cast<float>(currentTime - lastTime) / 1000.0f;
        dt *= timeMultiplier;
        lastTime = currentTime;

        frameCount++;
        if (currentTime - fpsTime >= 500) {
            float fps = static_cast<float>(frameCount) / static_cast<float>(currentTime - fpsTime)*1000;
            float fpsRender = static_cast<float>(renderCount) / static_cast<float>(currentTime - fpsTime)*1000;
            std::cout << "FPS: " << fps << "\t" << "RENDER: " << fpsRender << std::endl;
            skipValue = (static_cast<int>(fps) / constants::TARGET_FPS) - 1;
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
            SDL_RenderSetScale(renderer, static_cast<float>(windowWidth) / constants::SCREEN_WIDTH,
                           static_cast<float>(windowHeight) / constants::SCREEN_HEIGHT);
        } //Resize Window

        {
            ptsCollided.clear();
            ptsNotCollided.clear();
            for(int i=0; i<N; i++) {
                check[i].store(false, std::memory_order_relaxed);
            }
        } //Vector initialization
        std::sort(particles.begin(), particles.end(), [](const Particle &a, const Particle &b) {
                return a.getPosition().x < b.getPosition().x;
            });

        for(int t=0; t < numThreads; t++) {
            int startIndex = (t==0) ? 0 : (t*chunk - constants::CHUNK_OVERLAP)  ;
            int endIndex = (t==numThreads-1) ? static_cast<int>(N) : ((t+1)*chunk + constants::CHUNK_OVERLAP);
            threads.emplace_back([&check, startIndex, endIndex, &particles, dt]() {
                singleThreadParticleComputation(check, particles, startIndex, endIndex, dt);
            });
        } //Assign each tread its works

       /* int startIndex = (numThreads-1)*chunk;
          threads.emplace_back([&check, startIndex, N, &particles, dt]() {
          singleThreadParticleComputation(check, particles, startIndex, N, dt);
       }); //effetto diffusione figo, un bug che diventa un feature */

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

        if (skipCount > 0) {
            skipCount--;
            continue; //va al prossimo ciclo del while
        } //Frame skipping
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            //Particelle con collisioni avvenute
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
            if (!ptsCollided.empty()) {
                SDL_RenderDrawPoints(renderer, ptsCollided.data(), static_cast<int>(ptsCollided.size()));
                //*.data() restituisce un puntaore al primo elemento del vettore, così si può accedere ad un vector come se fosse un array
            }

            //Particelle senza collisioni avvenute
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            if (!ptsNotCollided.empty()) {
                SDL_RenderDrawPoints(renderer, ptsNotCollided.data(), static_cast<int>(ptsNotCollided.size()));
            }
            SDL_RenderPresent(renderer);

            renderCount++;
            skipCount = skipValue; //ripristina skipCount dopo aver renderizzatoil frame
        } //Render
        }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
