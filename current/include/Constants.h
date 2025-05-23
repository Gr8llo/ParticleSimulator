#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
 constexpr definisce il valore della variabile durante la compilazione (massima ottimizzazione)
 mentre const può essere calcolato a runtime
*/

namespace screen {
    constexpr int SCREEN_WIDTH = 1920;
    constexpr int SCREEN_HEIGHT = 1080;
    constexpr unsigned int CONTAINER_INITIAL_WIDTH = 800;
    constexpr unsigned int CONTAINER_INITIAL_HEIGHT = 450;
    constexpr int TARGET_FPS = 60;
    constexpr int CELL_SIZE = 4;
}

namespace particle {
    constexpr unsigned int DEFAULT_NUM_PARTICLES = 1e4;
    constexpr float MAX_PARTICLES_VELOCITY = 500.0f;
    constexpr float PARTICLE_RADIUS = 1.f; //un raggio molto piccolo dovrebbe essere molto realistico, ed evita i cluster di particelle che erano dovuti al troppo poco spazio
    constexpr float PARTICLE_DENSITY = 10.0f;
    constexpr float PARTICLE_MASS = M_PI * PARTICLE_RADIUS * PARTICLE_DENSITY; //le particelle sono un disco
    constexpr float INVERSE_PARTICLE_MASS = 1.0f / PARTICLE_MASS;
    constexpr float MIN_PARTICLE_DIST = 2*PARTICLE_RADIUS;
    constexpr float MIN_PARTICLE_DIST_SQUARED = MIN_PARTICLE_DIST*MIN_PARTICLE_DIST;
    constexpr float PARTICLES_CONTROL_DISTANCE = 2*PARTICLE_RADIUS + 1.f;
}

#endif //CONSTANTS_H
