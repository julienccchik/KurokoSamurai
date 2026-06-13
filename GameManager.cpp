#include "GameManager.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

// вспомогательный метод для форматирования
static std::string t(const nlohmann::json& j) { return j.get<std::string>(); }

// конструктор готовит всё для запуска игры
GameManager::GameManager() : currentLayer(0), currentLocationIndex(-1) {
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  loadConfig();
  loadLocations();
  loadDialogues();
  loadItems();
  loadTexts();
  loadEnemies();
  loadNPCs();
  initPlayer();
  visitedLocationsLayer1.assign(3, false);
  visitedLocationsLayer2.assign(3, false);
  damageItemDuration = config["game_settings"]["damage_item_duration"];
}

// загрузка JSON-файлов
void GameManager::loadConfig() {
  std::ifstream file("C:/ALL/C++/KurokoSamurai/data/config.json");
  if (!file.is_open()) {
    std::cerr << "Ошибка: не удалось загрузить config.json\n";
    exit(1);
  }

  file >> config;
  file.close();
}

// загрузка локаций
void GameManager::loadLocations() {
  std::ifstream file("C:/ALL/C++/KurokoSamurai/data/locations.json");
  if (file.is_open()) {
    file >> locationsData;
    file.close();
  } else {
    std::cerr << "Ошибка, не удалось загрузить locations.json\n";
    exit(1);
  }
}

// загрузка диалогов
void GameManager::loadDialogues() {
  std::ifstream file("C:/ALL/C++/KurokoSamurai/data/dialogues.json");
  if (file.is_open()) {
    file >> dialoguesData;
    file.close();
  } else {
    std::cerr << "Ошибка, не удалось загрузить dialogues.json\n";
    exit(1);
  }
}

// загрузка предметов
void GameManager::loadItems() {
  std::ifstream file("C:/ALL/C++/KurokoSamurai/data/items.json");
  if (file.is_open()) {
    file >> itemsData;
    file.close();
  } else {
    std::cerr << "Ошибка, не удалось загрузить items.json\n";
    exit(1);
  }
}

// загрузка текстов
void GameManager::loadTexts() {
  std::ifstream file("C:/ALL/C++/KurokoSamurai/data/texts.json");
  if (file.is_open()) {
    file >> textsData;
    file.close();
  } else {
    std::cerr << "Ошибка, не удалось загрузить texts.json\n";
    exit(1);
  }
}
// загрузка врагов
void GameManager::loadEnemies() {
  auto& enemiesJson = config["enemies"];
  for (auto it = enemiesJson.begin(); it != enemiesJson.end(); ++it) {
    std::string key = it.key();  // записывает имя врага
    auto value = it.value();     // записывает содержимое врага
    enemies.emplace(key,
                    Enemy(0, value["name"], value["health"], value["damage"]));
  }
}

// загрузка нпс
void GameManager::loadNPCs() {
  auto& locations = locationsData["layers"];
  for (int layer = 0; layer < 2; layer++) {  // пока только из первых двух слоёв
    for (auto& loc : locations[layer]["locations"]) {
      std::string npcKey = loc["npc"];
      std::string dialogue = dialoguesData[npcKey];
      npcs.emplace(npcKey, NPC(npcKey, dialogue));
    }
  }
  npcs.emplace(
      "dragon",
      NPC("Дракон",
          dialoguesData["dragon_start"]));  // а здесь добавляем дракона
}

// инициализация игрока
void GameManager::initPlayer() {
  int maxHealth = config["player"]["max_health"];
  int startLootChance = config["player"]["start_loot_chance"];
  int startRadiationResist = config["player"]["start_radiation_resist"];
  int fistBaseDamage = config["player"]["base_damage_fist"];
  int damageItemDuration = config["game_settings"]["damage_item_duration"];

  player = std::make_unique<Player>(1, "Самурай", maxHealth, startLootChance,
                                    startRadiationResist, fistBaseDamage,
                                    damageItemDuration);
}

// реализуем рандом
int GameManager::getRandomNumber(int min, int max) {
  return min + (std::rand() % (max - min + 1));
}

// реализация боя
void GameManager::combat(Enemy& enemy) {
  std::cout << t(textsData["messages"]["combat_start"]) << enemy.getName()
            << "!\n";
  std::cout << t(textsData["messages"]["combat_enemy_hp"]) << enemy.getHealth()
            << "\n\n";

  while (player->isAlive() && enemy.isAlive()) {
    std::cout << t(textsData["messages"]["combat_choice_title"]) << "\n";
    std::cout << t(textsData["combat_menu"]["fist"]) << "\n";
    std::cout << t(textsData["combat_menu"]["sword"]) << "\n";
    std::cout << t(textsData["combat_menu"]["item"]) << "\n";

    std::string choice;
    std::cin >> choice;
    std::cout << "\n---\n\n";

    int fistCrit = config["game_settings"]["fist_crit_threshold"];
    int fistMiss = config["game_settings"]["fist_miss_threshold"];
    int fistBaseDamage = player->getFistBaseDamage();

    if (choice == "1") {  // кулачный бой
      int roll = getRandomNumber(1, 20);
      std::cout << t(textsData["messages"]["fist_roll"]) << roll << "\n";

      if (roll == fistCrit) {
        std::cout << t(textsData["messages"]["fist_crit"]) << "\n";
        enemy.takeDamage(enemy.getHealth());
      } else if (roll == fistMiss) {
        std::cout << t(textsData["messages"]["fist_miss"]) << "\n";
      } else {
        int damage = fistBaseDamage + roll + player->getCurrentDamageBoost();
        enemy.takeDamage(damage);
        std::cout << t(textsData["messages"]["fist_damage"]) << damage
                  << t(textsData["messages"]["fist_damage_suffix"]) << "\n";
      }
    } else if (choice == "2") {  // мечевой бой
      int swordMin = config["game_settings"]["sword_range_min"];
      int swordMax = config["game_settings"]["sword_range_max"];
      int critDiff = config["game_settings"]["sword_critical_diff"];
      int halfDiff = config["game_settings"]["sword_half_diff"];

      int secret =
          getRandomNumber(swordMin, swordMax);  // сохраняем загаданное число
      std::cout << t(textsData["messages"]["sword_prompt"]);
      std::string guessStr;
      std::cin >> guessStr;
      int guess = std::stoi(guessStr);
      int diff = std::abs(guess - secret);

      if (diff <= critDiff) {
        std::cout << t(textsData["messages"]["sword_crit"]) << "\n";
        enemy.takeDamage(enemy.getHealth());
      } else if (diff <= halfDiff) {
        int damage = enemy.getHealth() / 2;
        enemy.takeDamage(damage);
        std::cout << t(textsData["messages"]["sword_half"]) << damage
                  << t(textsData["messages"]["sword_half_suffix"]) << "\n";
      } else {
        std::cout << t(textsData["messages"]["sword_miss"]) << "\n";
      }
    } else if (choice == "3") {  // открытие инвентаря
      handleInventory();
      continue;
    } else {  // сообщение об ошибке
      std::cout << t(textsData["messages"]["invalid_choice"]);
      continue;
    }

    // если монстр жив - он атакует
    if (enemy.isAlive()) {
      int enemyDamage = enemy.getDamage();
      player->takeDamage(enemyDamage);
      std::cout << enemy.getName() << t(textsData["messages"]["enemy_attack"])
                << enemyDamage
                << t(textsData["messages"]["enemy_attack_suffix"]) << "\n";
      std::cout << t(textsData["messages"]["player_hp_status"])
                << player->getHealth() << "/" << player->getMaxHealth() << "\n";
    }
  }

  // проверка на живость персонажа
  if (player->isAlive()) {
    std::cout << t(textsData["messages"]["victory"]) << enemy.getName()
              << "!\n";
    player->decreaseDamageBoost();
  } else {
    std::cout << t(textsData["messages"]["game_over_combat"]) << "\n";
    exit(0);
  }
}

// использование инвентаря
void GameManager::handleInventory() {
  player->showInventory();
  if (player->getInventory().empty()) {
    return;
  }
  std::cout << t(textsData["messages"]["inventory_use_prompt"]);
  int index;
  std::cin >> index;
  if (index > 0 && index <= static_cast<int>(player->getInventory().size())) {
    player->useItem(index - 1);
  }
}

// вход в помещение
void GameManager::enterRoom() {
  auto& layers = locationsData["layers"];
  auto& currentLayerData = layers[currentLayer];
  auto& location = currentLayerData["locations"][currentLocationIndex];

  std::cout << t(textsData["messages"]["enter_room"]) << location["name"]
            << "\n";
  std::cout << location["description"] << "\n";

  int healAmount = config["game_settings"]["room_heal"];
  player->heal(healAmount);
  std::cout << t(textsData["messages"]["enter_room_heal"]) << healAmount
            << t(textsData["messages"]["enter_room_heal_unit"])
            << player->getHealth() << "\n";

  if (currentLayer == 0) {
    visitedLocationsLayer1[currentLocationIndex] = true;
  } else if (currentLayer == 1) {
    visitedLocationsLayer2[currentLocationIndex] = true;
  }

  std::string npcName = location["npc"];
  std::string enemyName = location["enemy"];

  if (npcs.find(npcName) != npcs.end()) {
    npcs[npcName].talk();
  }

  if (enemies.find(enemyName) != enemies.end()) {
    std::cout << "\n" << t(textsData["messages"]["enemy_appears"]) << "\n";
    Enemy enemyCopy = enemies[enemyName];  // создаём копию
    combat(enemyCopy);                     // и сражаемся с копией
  }

  std::cout << t(textsData["messages"]["exit_room"]) << "\n";
  checkLayerTransition();
}

// проход вперёд
void GameManager::walkForward() {
  int radiationDamage = config["game_settings"]["radiation_damage_base"];

  int actualDamage = player->takeRadiationDamage(radiationDamage, 0);
  std::cout << t(textsData["messages"]["walk_forward"]) << "\n";
  std::cout << t(textsData["messages"]["radiation_damage"]) << actualDamage
            << t(textsData["messages"]["radiation_damage_unit"]) << "\n";
  std::cout << t(textsData["messages"]["radiation_current_hp"])
            << player->getHealth() << "/" << player->getMaxHealth() << "\n";

  if (!player->isAlive()) {
    std::cout << t(textsData["messages"]["game_over_radiation"]) << "\n";
    exit(0);
  }

  int lootRoll = getRandomNumber(1, 100);
  if (lootRoll <= player->getLootChance()) {
    // нашли предмет — выдаём и сбрасываем шанс
    int itemIndex = getRandomNumber(0, itemsData.size() - 1);
    int i = 0;
    for (auto it = itemsData.begin(); it != itemsData.end(); ++it) {
      if (i == itemIndex) {
        Item item;
        item.id = it.key();
        item.name = it.value()["name"];
        item.type = it.value()["type"];
        item.value = it.value()["value"];
        item.description = it.value()["description"].get<std::string>();
        item.quantity = 1;
        player->addItem(item);
        std::cout << t(textsData["messages"]["found_item"]) << item.name
                  << t(textsData["messages"]["found_item_suffix"]) << "\n";
        break;
      }
      i++;
    }
    // сброс после находки
    player->setLootChance(config["game_settings"]["loot_base_chance"]);
  } else {
    // не нашли — увеличиваем шанс на следующий шаг
    int lootStep = config["game_settings"]["loot_chance_step"];
    int newChance = player->getLootChance() + lootStep;
    if (newChance > 100) newChance = 100;
    player->setLootChance(newChance);
  }

  checkLayerTransition();
}

// проверка не пора ли перейти на другой уровень
void GameManager::checkLayerTransition() {
  bool allVisited = true;
  if (currentLayer == 0) {
    for (bool visited : visitedLocationsLayer1) {
      if (!visited) {
        allVisited = false;
        break;
      }
    }
    if (allVisited && currentLocationIndex == -1) {
      std::cout << t(textsData["messages"]["layer_transition_vibration"])
                << "\n";
      std::cout << t(textsData["messages"]["layer_transition_hole"]) << "\n";
      std::cout << t(textsData["messages"]["layer_transition_descend"]) << "\n";
      currentLayer = 1;
      currentLocationIndex = -1;
    }
  } else if (currentLayer == 1) {
    for (bool visited : visitedLocationsLayer2) {
      if (!visited) {
        allVisited = false;
        break;
      }
    }
    if (allVisited && currentLocationIndex == -1) {
      std::cout << t(textsData["messages"]["layer_transition_roar"]) << "\n";
      std::cout << t(textsData["messages"]["layer_transition_passage"]) << "\n";
      std::cout << t(textsData["messages"]["layer_transition_fate"]) << "\n";
      currentLayer = 2;
      currentLocationIndex = 0;
      finalBossEncounter();
    }
  }
}

// ux/ui
void GameManager::showStatus() {
  std::cout << t(textsData["messages"]["status_separator"]) << "\n";
  std::cout << t(textsData["messages"]["status_hp"]) << player->getHealth()
            << t(textsData["messages"]["status_hp_separator"])
            << player->getMaxHealth();
  std::cout << t(textsData["messages"]["status_loot_chance"])
            << player->getLootChance()
            << t(textsData["messages"]["status_loot_percent"]) << "\n";

  auto& layers = locationsData["layers"];
  std::cout << t(textsData["messages"]["status_layer"]) << " "
            << layers[currentLayer]["name"] << "\n";

  if (currentLayer == 0) {
    int visited = 0;
    for (bool v : visitedLocationsLayer1)
      if (v) visited++;
    std::cout << t(textsData["messages"]["status_visited"]) << visited
              << t(textsData["messages"]["status_visited_separator"]) << "\n";
  } else if (currentLayer == 1) {
    int visited = 0;
    for (bool v : visitedLocationsLayer2)
      if (v) visited++;
    std::cout << t(textsData["messages"]["status_visited"]) << visited
              << t(textsData["messages"]["status_visited_separator"]) << "\n";
  }
  std::cout << t(textsData["messages"]["status_separator_end"]);
}

// меню действий
void GameManager::explore() {
  auto& layers = locationsData["layers"];
  auto& currentLayerData = layers[currentLayer];
  auto& locations = currentLayerData["locations"];

  std::cout << "\n" << t(textsData["messages"]["explore_street"]) << std::endl;
  std::cout << t(textsData["messages"]["explore_available"]) << std::endl;

  for (int i = 0; i < static_cast<int>(locations.size()); i++) {
    std::cout << i + 1 << ". " << locations[i]["name"].get<std::string>();
    bool visited = false;
    if (currentLayer == 0 && visitedLocationsLayer1[i]) visited = true;
    if (currentLayer == 1 && visitedLocationsLayer2[i]) visited = true;
    if (visited) std::cout << t(textsData["messages"]["explore_visited_mark"]);
    std::cout << std::endl;
  }

  std::cout << t(textsData["messages"]["explore_walk_option"]) << std::endl;
  std::cout << t(textsData["messages"]["explore_inventory_option"])
            << std::endl;
  std::cout << t(textsData["messages"]["explore_save_option"]) << std::endl;
  std::cout << t(textsData["messages"]["explore_load_option"]) << std::endl;
  std::cout << t(textsData["messages"]["explore_prompt"]);

  std::string choice;
  std::cin >> choice;
  std::cout << "\n---\n\n";

  if (choice == "1" || choice == "2" || choice == "3") {
    int index = std::stoi(choice) - 1;
    if (index >= 0 && index < static_cast<int>(locations.size())) {
      currentLocationIndex = index;
      enterRoom();
      currentLocationIndex = -1;
    }
  } else if (choice == "4") {
    walkForward();
  } else if (choice == "5") {
    handleInventory();
  } else if (choice == "6") {
    saveGame();
  } else if (choice == "7") {
    loadGame();
  } else {
    std::cout << t(textsData["messages"]["invalid_choice"]);
  }
}

// сохранение игры
void GameManager::saveGame() {
  json saveData;

  saveData["current_layer"] = currentLayer;
  saveData["current_location_index"] = currentLocationIndex;

  for (size_t i = 0; i < visitedLocationsLayer1.size(); i++) {
    saveData["visited_layer1"][i] = visitedLocationsLayer1[i];
  }

  for (size_t i = 0; i < visitedLocationsLayer2.size(); i++) {
    saveData["visited_layer2"][i] = visitedLocationsLayer2[i];
  }

  saveData["player"]["health"] = player->getHealth();
  saveData["player"]["max_health"] = player->getMaxHealth();
  saveData["player"]["radiation_resist"] = player->getRadiationResist();
  saveData["player"]["loot_chance"] = player->getLootChance();
  saveData["player"]["damage_boost"] = player->getCurrentDamageBoost();

  for (const auto& item : player->getInventory()) {
    json itemJson;
    itemJson["id"] = item.id;
    itemJson["name"] = item.name;
    itemJson["type"] = item.type;
    itemJson["value"] = item.value;
    itemJson["quantity"] = item.quantity;
    saveData["inventory"].push_back(itemJson);
  }

  std::ofstream file("savegame.json");
  if (file.is_open()) {
    file << saveData.dump(4);  //  форматирование с отступами
    file.close();
    std::cout << "\nИгра сохранена!\n";
  } else {
    std::cout << "\nОшибка сохранения!\n";
  }
}

// загрузка игры
void GameManager::loadGame() {
  std::ifstream file("savegame.json");
  if (!file.is_open()) {
    std::cout << "\nНет сохранённой игры. Начинаем новую игру.\n";
    return;
  }

  json saveData;
  file >> saveData;
  file.close();

  currentLayer = saveData["current_layer"];
  currentLocationIndex = saveData["current_location_index"];

  for (size_t i = 0; i < visitedLocationsLayer1.size(); i++) {
    visitedLocationsLayer1[i] = saveData["visited_layer1"][i];
  }
  for (size_t i = 0; i < visitedLocationsLayer2.size(); i++) {
    visitedLocationsLayer2[i] = saveData["visited_layer2"][i];
  }

  int maxHealth = saveData["player"]["max_health"];
  int health = saveData["player"]["health"];
  int radiationResist = saveData["player"]["radiation_resist"];
  int lootChance = saveData["player"]["loot_chance"];
  int damageBoost = saveData["player"]["damage_boost"];

  int startLootChance = config["player"]["start_loot_chance"];
  int startRadiationResist = config["player"]["start_radiation_resist"];
  int fistBaseDamage = config["player"]["base_damage_fist"];
  int damageItemDuration = config["game_settings"]["damage_item_duration"];

  player = std::make_unique<Player>(1, "Самурай", maxHealth, startLootChance,
                                    startRadiationResist, fistBaseDamage,
                                    damageItemDuration);

  player->heal(health - player->getHealth());
  player->setRadiationResist(radiationResist);
  player->setLootChance(lootChance);

  for (const auto& itemJson : saveData["inventory"]) {
    Item item;
    item.id = itemJson["id"];
    item.name = itemJson["name"];
    item.type = itemJson["type"];
    item.value = itemJson["value"];
    item.quantity = itemJson["quantity"];

    for (int i = 0; i < item.quantity; i++) {
      Item singleItem = item;
      singleItem.quantity = 1;
      player->addItem(singleItem);
    }
  }

  std::cout << "\nИгра загружена!\n";
}

// финальный босс
void GameManager::finalBossEncounter() {
  std::cout << t(textsData["messages"]["final_title"]) << "\n";
  std::cout << t(textsData["messages"]["final_boss_title"]) << "\n";
  std::cout << t(textsData["messages"]["final_title"]) << "\n\n";

  std::cout << dialoguesData["dragon_start"] << "\n\n";
  std::cout << dialoguesData["dragon_choice"] << "\n";

  std::string choice;
  std::cin >> choice;

  std::cout << textsData["messages"]["final_title"] << "\n";
  if (choice == "1") {
    std::cout << dialoguesData["ending_kill"] << "\n";
  } else {
    std::cout << dialoguesData["ending_leave"] << "\n";
  }
  std::cout << t(textsData["messages"]["final_title"]) << "\n";
  std::cout << t(textsData["messages"]["final_game_ended"]) << "\n";
  std::cout << t(textsData["messages"]["final_title"]) << "\n\n";

  exit(0);
}

// основной цикл
void GameManager::run() {
  std::cout << textsData["messages"]["game_welcome"] << "\n";
  std::cout << textsData["messages"]["game_description"] << "\n";

  while (true) {
    showStatus();
    explore();
  }
}
