#include "../include/Particle.h"
#include "../include/Constants.h"

Particle::Particle(const int id, const glm::vec2 pos, const glm::vec2 vel): position(pos), velocity(vel), id(id){}

glm::vec2 Particle::getPosition() const{
    return position;
}

glm::vec2 Particle::getVelocity() const{
    return velocity;
}

int Particle::getId() const {
    return id;
}

void Particle::update(const float dt) {
    position += velocity * dt;
}

void Particle::checkWallCollision() {
    constexpr float radius = particle::PARTICLE_RADIUS;
    if (position.x - radius < 0.1 || position.x + radius > screen::SCREEN_WIDTH - 0.1)
        velocity.x *= -1;
    if (position.y - radius < 0.1 || position.y + radius > screen::SCREEN_HEIGHT - 0.1)
        velocity.y *= -1;
}

bool Particle::resolveCollision(Particle &other) {
    const glm::vec2 delta = position - other.position;
    const float distSquared = dot(delta, delta);

    if (distSquared > particle::MIN_PARTICLE_DIST_SQUARED || distSquared < 0.0f) return false;

    const float dist = sqrt(distSquared); // solo ora uso sqrt
    const glm::vec2 normal = glm::normalize(delta);
    const glm::vec2 relativeVel = velocity - other.velocity;
    const float velAlongNormal = dot(relativeVel, normal);

    if (velAlongNormal > 0.f) return false; // si stanno allontanando

    constexpr float restitution = 1.f;
    const float impulseMag = -(1 + restitution) * velAlongNormal / (particle::PARTICLE_MASS * 2); //ottimizzata

    const glm::vec2 impulse = impulseMag * normal;
    velocity += impulse * particle::INVERSE_PARTICLE_MASS;
    other.velocity -= impulse * particle::INVERSE_PARTICLE_MASS;

    // Risolvi penetrazione (correzione di posizione)
    const float penetration = particle::MIN_PARTICLE_DIST - dist;
    const glm::vec2 correction = normal * (penetration / 2.f);
    position += correction;
    other.position -= correction;
    return true;
}