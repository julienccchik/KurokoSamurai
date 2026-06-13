#pragma once
#include "Entity.h"

class Enemy : public Entity {
 private:
  int damage;

 public:
  Enemy();
  Enemy(int id, std::string name, int health, int damage);
  int getDamage() const;
};
