#include "Player.h"

#include <fstream>
#include <iostream>

// выделение пам€ти дл€ texts
json Player::texts;

// загрузка текстов из файла
void Player::loadTexts() {
  std::ifstream file("data/texts.json");
  if (file.is_open()) {
    file >> texts;
    file.close();
  }
}

// конструктор игрока
Player::Player(int id, std::string name, int health, int startLootChance,
               int startRadiationResist, int fistBaseDamage,
               int damageItemDuration)
    : Entity(id, name, health),
      radiationResist(startRadiationResist),
      lootChance(startLootChance),
      fistBaseDamage(fistBaseDamage),
      damageBoost(0),
      damageBoostTurns(0),
      damageItemDuration(damageItemDuration) {
  if (texts.empty()) loadTexts();
}

// добавление предмета в инвентарь
void Player::addItem(Item item) {
  for (auto& existing : inventory) {
    if (existing.id == item.id) {
      existing.quantity++;
      return;
    }
  }
  item.quantity = 1;
  inventory.push_back(item);
}

// использование предмета
void Player::useItem(int index) {
  if (index < 0 || index >= static_cast<int>(inventory.size())) {
    std::cout << texts["messages"]["invalid_index"] << "\n";
    return;
  }  // проверка корректности индекса

  Item& item = inventory[index];

  if (item.type == "heal") {
    heal(item.value);
    std::cout << texts["messages"]["used_item"] << item.name
              << texts["messages"]["used_item_heal"] << item.value
              << texts["messages"]["used_item_heal_suffix"] << health << "\n";
  } else if (item.type == "damage") {
    damageBoost += item.value;
    damageBoostTurns = damageItemDuration;
    std::cout << texts["messages"]["damage_item_used"] << item.value
              << texts["messages"]["damage_item_used_suffix"]
              << damageItemDuration
              << texts["messages"]["damage_item_used_duration"];
  } else if (item.type == "radiation") {
    radiationResist += item.value;
    std::cout << texts["messages"]["used_item_resist"] << item.value
              << texts["messages"]["used_item_resist_suffix"];
  } else {
    std::cout << texts["messages"]["will_be_useful_later"] << "\n";
    return;
  }

  item.quantity--;
  if (item.quantity <= 0) {
    inventory.erase(inventory.begin() + index);
  }
}

// показать инвентарь
void Player::showInventory() const {
  if (inventory.empty()) {
    std::cout << texts["messages"]["inventory_empty"] << "\n";
    return;
  }  // если инвентарь пуст

  std::cout << "\n" << texts["messages"]["inventory_title"] << "\n";
  for (size_t i = 0; i < inventory.size(); ++i) {
    const auto& item = inventory[i];
    std::cout << i + 1 << ". " << item.name << " x" << item.quantity << " Ч "
              << item.description << "\n";
    if (item.type == "heal") {
      std::cout << " (Ћечит " << item.value << " HP)";
    } else if (item.type == "damage") {
      std::cout << " (+" << item.value << " к урону на 3 бо€)";
    } else if (item.type == "radiation") {
      std::cout << " (+" << item.value << "% резист к радиации)";
    }
    std::cout << "\n";
  }
}

// геттеры и сеттеры
void Player::setLootChance(int chance) { lootChance = chance; }
int Player::getLootChance() const { return lootChance; }
int Player::getFistBaseDamage() const { return fistBaseDamage; }
int Player::getCurrentDamageBoost() const { return damageBoost; }
void Player::setRadiationResist(int resist) { radiationResist = resist; }

// бафф к урону
void Player::applyDamageBoost(int boost) {
  damageBoost += boost;
  damageBoostTurns = 3;
}

void Player::decreaseDamageBoost() {
  if (damageBoostTurns > 0) {
    damageBoostTurns--;
    if (damageBoostTurns == 0) {
      damageBoost = 0;
      std::cout << texts["messages"]["damage_boost_ended"];
    }
  }
}

// получение урона от радиации
int Player::takeRadiationDamage(int damage, int resist) {
  int actualResist = radiationResist + resist;
  if (actualResist > 100) actualResist = 100;
  if (actualResist > 0) {
    damage = damage * (100 - actualResist) / 100;
    std::cout << texts["messages"]["radiation_shield"] << "\n";
  }
  takeDamage(damage);
  return damage;
}
