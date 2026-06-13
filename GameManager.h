#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Enemy.h"
#include "NPC.h"
#include "Player.h"
#include "json.hpp"

using json = nlohmann::json;

class GameManager {
 private:
  std::unique_ptr<Player> player;
  int currentLayer;
  int currentLocationIndex;
  std::vector<bool> visitedLocationsLayer1;
  std::vector<bool> visitedLocationsLayer2;
  json config;
  json locationsData;
  json dialoguesData;
  json itemsData;
  json textsData;
  std::map<std::string, Enemy> enemies;
  std::map<std::string, NPC> npcs;
  int damageItemDuration;

  void loadConfig();
  void loadLocations();
  void loadDialogues();
  void loadItems();
  void loadTexts();
  void loadEnemies();
  void loadNPCs();
  void initPlayer();
  void showStatus();
  void combat(Enemy& enemy);
  void enterRoom();
  void explore();
  void handleInventory();
  void walkForward();
  void checkLayerTransition();
  void finalBossEncounter();
  void saveGame();
  void loadGame();

 public:
  GameManager();
  void run();
  int getRandomNumber(int min, int max);
};
