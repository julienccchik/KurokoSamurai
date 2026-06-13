#include <windows.h>

#include "GameManager.h"

int main() {
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  setlocale(LC_ALL, ".UTF8");

  GameManager game;
  game.run();
  return 0;
}
