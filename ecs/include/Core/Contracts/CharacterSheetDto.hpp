#pragma once
#include <string>

struct CharacterSheetDto {
    std::string Name;
    std::string Weapon;
    std::string Skill;
    int Health;
    int Mana;
    int Strength;
    int Intelligence;

    CharacterSheetDto(std::string name, std::string weapon, std::string skill, 
                      int health, int mana, int strength, int intelligence)
        : Name(std::move(name)), 
          Weapon(std::move(weapon)), 
          Skill(std::move(skill)), 
          Health(health), 
          Mana(mana), 
          Strength(strength), 
          Intelligence(intelligence) {}
};
