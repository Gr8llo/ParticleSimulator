#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL_render.h>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

class Particle {
private:
    glm::vec2 position;
    glm::vec2 velocity;
    // float radius;  //aggiunti tra le costanti
    // float mass;
    int id;

public:
    Particle(int id, glm::vec2 pos, glm::vec2 vel);

    [[nodiscard]] glm::vec2 getPosition() const; //se la funzione viene chiamata ed il valore ignorato il compialtore segnala errore
    [[nodiscard]] glm::vec2 getVelocity() const;
    [[nodiscard]] int getId() const;

    void update(float dt);
    void checkWallCollision();
    bool resolveCollision(Particle &other);
};
#endif //PARTICLE_H
