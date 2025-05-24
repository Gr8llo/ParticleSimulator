#ifndef PARTICLEUTILS_H
#define PARTICLEUTILS_H

#include <vector>
#include "Particle.h"

void initializeRandomParticles(std::vector<Particle> &particles, unsigned int N);

using namespace std;

void gridInitialization(vector<vector<vector<int>>> &grid, const std::vector<Particle> &particles, unsigned int N);

#endif //PARTICLEUTILS_H
