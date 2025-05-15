#include "Particle.h"
#include "Constants.h"

Particle::Particle(int id, glm::vec2 position, glm::vec2 velocity): position(position), velocity(velocity), id(id){}

glm::vec2 Particle::getPosition() const{
    return position;
}

glm::vec2 Particle::getVelocity() const{
    return velocity;
}

int Particle::getId() const {
    return id;
}

void Particle::update(float deltaTime) {
    position += velocity * deltaTime;
}

void Particle::checkWallCollision() {
    float radius = constants::PARTICLE_RADIUS;
    if (position.x - radius < 0.1 || position.x + radius > constants::SCREEN_WIDTH - 0.1)
        velocity.x *= -1;
    if (position.y - radius < 0.1 || position.y + radius > constants::SCREEN_HEIGHT - 0.1)
        velocity.y *= -1;
}

bool Particle::resolveCollision(Particle &other) {
    const glm::vec2 delta = position - other.position;
    const float distSquared = dot(delta, delta);
    constexpr float minDist = 2*constants::PARTICLE_RADIUS;
    constexpr float minDistSquared = minDist * minDist;

    if (distSquared < minDistSquared && distSquared > 0.0f) {
        const float dist = sqrt(distSquared);  // solo ora uso sqrt
        const glm::vec2 normal = delta / dist; // normalizzato manualmente
        const glm::vec2 relativeVel = velocity - other.velocity;
        const float velAlongNormal = dot(relativeVel, normal);

        if (velAlongNormal > 0) return false; // si stanno allontanando

        constexpr float restitution = 1.0f;
        const float impulseMag = -(1 + restitution) * velAlongNormal / (2/constants::INVERSE_PARTICLE_MASS);

        const glm::vec2 impulse = impulseMag * normal;
        velocity += impulse * constants::INVERSE_PARTICLE_MASS;
        other.velocity -= impulse * constants::INVERSE_PARTICLE_MASS;

        // Risolvi penetrazione (correzione di posizione)
        const float penetration = minDist - dist;
        const glm::vec2 correction = normal * (penetration / 2.0f);
        position += correction;
        other.position -= correction;

        return true;
    }
    return false;
}