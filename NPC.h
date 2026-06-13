#pragma once
#include <string>

class NPC {
 private:
  std::string name;
  std::string dialogue;

 public:
  NPC();
  NPC(std::string name, std::string dialogue);
  void talk() const;
  std::string getName() const;
};
