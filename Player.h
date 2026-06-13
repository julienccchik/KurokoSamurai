#pragma once
#include <string>
#include <vector>

#include "Entity.h"
#include "json.hpp"

using json = nlohmann::json;

// структура состоит из простых данных
struct Item {
  std::string id;
  std::string name;
  std::string type;
  int value;
  int quantity;
  std::string description;
};

class Player : public Entity {
 private:
  int radiationResist;
  int lootChance;
  int fistBaseDamage;
  int damageBoost;       // временный бафф к урону
  int damageBoostTurns;  // сколько боёв осталось действовать баффу
  int damageItemDuration;
  std::vector<Item> inventory;
  static json texts;
  void loadTexts();

 public:
  Player(int id, std::string name, int health, int startLootChance,
         int startRadiationResist, int fistBaseDamage, int damageItemDuration);
  void addItem(Item item);
  void useItem(int index);
  void showInventory() const;
  void setLootChance(int chance);
  int getLootChance() const;
  int getFistBaseDamage() const;
  int getCurrentDamageBoost() const;
  void applyDamageBoost(int boost);
  void decreaseDamageBoost();
  int takeRadiationDamage(int damage, int resist);
  void setRadiationResist(int resist);
  std::vector<Item>& getInventory() { return inventory; }
  static json& getTexts() { return texts; }
  int getRadiationResist() const { return radiationResist; }
};
