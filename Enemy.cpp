#include "Enemy.h"

Enemy::Enemy() : Entity(0, "", 0), damage(0) {}

Enemy::Enemy(int id, std::string name, int health, int damage)
    : Entity(id, name, health), damage(damage) {}

int Enemy::getDamage() const { return damage; }
