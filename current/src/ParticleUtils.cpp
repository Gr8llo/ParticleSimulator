#include "../include/ParticleUtils.h"

#include <random>
#include "../include/Constants.h"

inline float random(const float min, const float max) {
    static std::random_device r;
    static std::default_random_engine engine(r());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}

void initializeRandomParticles(std::vector<Particle> &particles, const unsigned int N) {
    constexpr float maxVelocity = particle::MAX_PARTICLES_VELOCITY;
    constexpr float maxHeightLimit = screen::SCREEN_HEIGHT/2.f + screen::CONTAINER_INITIAL_HEIGHT/2.f;
    constexpr float minHeightLimit = maxHeightLimit - screen::CONTAINER_INITIAL_HEIGHT;
    constexpr float maxWidthLimit = screen::SCREEN_WIDTH/2.f + screen::CONTAINER_INITIAL_WIDTH/2.f;
    constexpr float minWidthLimit = maxWidthLimit - screen::CONTAINER_INITIAL_WIDTH;
    for (int i = 0; i < N; i++) { //non si può usare particle.size() perchè ho usato reserve
        Particle p(
            i, glm::vec2(random(minWidthLimit, maxWidthLimit), random(minHeightLimit, maxHeightLimit)),
            glm::vec2(random(-maxVelocity, maxVelocity), random(-maxVelocity, maxVelocity)));
        particles.push_back(p);
    } //Generating casual position and velocity for each particle
}

void gridInitialization(vector<vector<vector<int>>> &grid, const vector<Particle> &particles, const unsigned int N) {
    for (int x = 0; x < screen::GRID_X; ++x) {
        for (int y = 0; y < screen::GRID_Y; ++y) {
            grid[x][y].clear();
        }
    }
    for (int i = 0; i < N; i++) {
        const int xIndex = static_cast<int>(particles[i].getPosition().x) / screen::CELL_SIZE;
        const int yIndex = static_cast<int>(particles[i].getPosition().y) / screen::CELL_SIZE;

        if (xIndex >= 0 && xIndex < screen::GRID_X && yIndex >= 0 && yIndex < screen::GRID_Y) {
            grid[xIndex][yIndex].push_back(i);
        }
    }
}
