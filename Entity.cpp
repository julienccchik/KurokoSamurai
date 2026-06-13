#include "Entity.h"

Entity::Entity(int id, std::string name, int health)
    : id(id), name(name), health(health), maxHealth(health) {}

// получение урона
void Entity::takeDamage(int amount) {
  health -= amount;
  if (health < 0) health = 0;
}

// получение лечения
void Entity::heal(int amount) {
  health += amount;
  if (health > maxHealth) health = maxHealth;
}

// геттеры
bool Entity::isAlive() const { return health > 0; }
int Entity::getHealth() const { return health; }
int Entity::getMaxHealth() const { return maxHealth; }
std::string Entity::getName() const { return name; }
