#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
 constexpr definisce il valore della variabile durante la compilazione (massima ottimizzazione)
 mentre const può essere calcolato a runtime
*/

namespace constants {
    constexpr int SCREEN_WIDTH = 2560;
    constexpr int SCREEN_HEIGHT = 1440;
    constexpr unsigned int DEFAULT_NUM_PARTICLES = 1e4;
    constexpr float MAX_PARTICLES_VELOCITY = 150.0f;
    constexpr float PARTICLE_MASS = 2000.0f;
    constexpr float INVERSE_PARTICLE_MASS = 1.0f / PARTICLE_MASS;
    constexpr float PARTICLE_RADIUS = 5.0f;
    constexpr unsigned int CONTAINER_INITIAL_WIDTH = 2560;
    constexpr unsigned int CONTAINER_INITIAL_HEIGHT = 1440;
    constexpr float PARTICLES_CONTROL_DISTANCE = 4.f;
    constexpr int CHUNK_OVERLAP = 0;
    constexpr int TARGET_FPS = 60;
}

/*
 TESTS:
 5000 particles: 35-40 fps;
 10000 particles: 30 fps;
 50000 particles: 5-6 fps;

 OTTIMIZZAZIONI:
 1. cmake -DCMAKE_BUILD_TYPE=Release . (molto importante, evidenti migloramenti)
 2. disegnare separatamente le particelle verdi e rosse per evitare di cambiare sempre il colore di render (poco importante aumenta qualche fps)
 3. disegnare tutti i punti assieme e non singolarmente tramite il metodo apposito SDL_RenderDrawPoints
 4. programamzione multithread
 5. frame skipping con molte particelle non porta benefici, a meno che si abbassino molto i target fps, ma a fps bassi rende la simulazione più realistica
 6. ottimizzata la funzione per la gestone delle collisioni

 FALLIMENTI:
 1. usare l'insertion sort
 2. multitheading assegnado i chunck una sola volta

 COSE AGGIUNTE:
 1. Resize della finestra
 2. Velocizzazione o Rallenatmento del tempo con le frecce destra e sinistra (faccio variare il dt)
 3. Fermare il tempo con barra spaziatrice

TESTS - FINALI:
 100000 particles: 70 fps;
 aumentando lo spazio si potrebbe aumentare di molto il numero di particelle, il problema principale sono le troppe collisioni
*/

#endif //CONSTANTS_H
