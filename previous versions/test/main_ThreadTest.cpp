#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <condition_variable>
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

const short unsigned int numThreads = std::thread::hardware_concurrency();
std::mutex mtx;
std::condition_variable cv;

void singleThreadParticleComputation(std::atomic<bool> check[], std::vector<Particle> &particles,
                                     const int lowerChuckBound, const int upperChuckBound, const std::atomic<float> &dt,
                                     std::atomic<int> &finishedCount, std::atomic_bool &ready) {
    while(true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&ready]{return ready.load();});
        }
        for (int i = lowerChuckBound; i < upperChuckBound && i<particles.size(); ++i) {
            particles[i].update(dt.load());
            particles[i].checkWallCollision();
            const float xI = particles[i].getPosition().x;
            for (int j = i + 1; j < upperChuckBound && j < particles.size(); j++) {
                const float xJ = particles[j].getPosition().x;
                if (xJ - xI > constants::PARTICLES_CONTROL_DISTANCE) break;
                if (particles[i].resolveCollision(particles[j])) {
                    check[i].store(true, std::memory_order_relaxed);
                    check[j].store(true, std::memory_order_relaxed);
                }
            } //Check and resolve collision
        }
        finishedCount.store(finishedCount.load() + 1, std::memory_order_release);
        if(finishedCount.load() >= numThreads) {
            ready.store(false, std::memory_order_release);
            cv.notify_one();
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
    } //Generating casual position and velocity for each particle
    std::vector<SDL_Point> ptsCollided(N);
    std::vector<SDL_Point> ptsNotCollided(N);
    std::atomic<bool> check[N];
    std::atomic<float> dt(0.f);

    const short unsigned int chunk = N/numThreads;
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    std::atomic<int> finishedCount(0);
    std::atomic_bool ready(false);
    for(int t=0; t < numThreads; t++) {
        int startIndex = (t==0) ? 0 : (t*chunk - constants::CHUNK_OVERLAP)  ;
        int endIndex = (t==numThreads-1) ? static_cast<int>(N) : ((t+1)*chunk + constants::CHUNK_OVERLAP);
        threads.emplace_back([&check, startIndex, endIndex, &particles, &dt, &finishedCount, &ready]() {
            singleThreadParticleComputation(check, particles, startIndex, endIndex, dt, finishedCount, ready);
        });
    }

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
        }


        unsigned int currentTime = SDL_GetTicks();
        dt.store(static_cast<float>(currentTime - lastTime)/1000.0f * timeMultiplier);
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
        } //fps and frameSkipping computation


        {
            int windowWidth, windowHeight;
            // Ottieni le dimensioni attuali della finestra
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            // Imposta la scala del rendering per adattarsi alla dimensione della finestra
            SDL_RenderSetScale(renderer, static_cast<float>(windowWidth) / constants::SCREEN_WIDTH,
                               static_cast<float>(windowHeight) / constants::SCREEN_HEIGHT);
        } //Resize Window

        ptsCollided.clear();
        ptsNotCollided.clear();
        for(int i=0; i<N; i++) {
            check[i].store(false, std::memory_order_relaxed);
        } //Reset check to false
        std::sort(particles.begin(), particles.end(), [](const Particle &a, const Particle &b) {
            return a.getPosition().x < b.getPosition().x;
        });

        finishedCount.store(0, std::memory_order_release);
        ready.store(true, std::memory_order_release);
        cv.notify_all();
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&ready] {
                return !ready.load();
            });
        }

        for (int i = 0; i < N; i++) {
            glm::vec2 pos = particles[i].getPosition();
            SDL_Point p(static_cast<int>(pos.x), static_cast<int>(pos.y));
            if (check[i].load()) {
                ptsCollided.push_back(p);
            } else {
                ptsNotCollided.push_back(p);
            }
        }
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
        }//Render
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
