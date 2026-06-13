#include "NPC.h"

#include <iostream>

NPC::NPC() : name(""), dialogue("") {}

NPC::NPC(std::string name, std::string dialogue)
    : name(name), dialogue(dialogue) {}

void NPC::talk() const { std::cout << dialogue << "\n"; }

std::string NPC::getName() const { return name; }
