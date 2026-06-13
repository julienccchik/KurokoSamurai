#pragma once
#include <string>

class Entity {
protected:
    int id;
    std::string name;
    int health;
    int maxHealth;
public:
 Entity(int id, std::string name, int health);
 virtual ~Entity() {}
 void takeDamage(int amount);
 void heal(int amount);
 bool isAlive() const;
 int getHealth() const;
 int getMaxHealth() const;
 std::string getName() const;
};