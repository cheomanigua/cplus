#pragma once
#include <string>

struct CharacterSheetDto {
    std::string Name;
    std::string Weapon;
    std::string Skill;
    int Strength;
    int Intelligence;
    int Dexterity;
    int Charisma;
    int Health;
    int Mana;

    CharacterSheetDto(std::string name, std::string weapon, std::string skill, 
                      int strength, int intelligence, int dexterity, int charisma, int health, int mana)
        : Name(std::move(name)), 
          Weapon(std::move(weapon)), 
          Skill(std::move(skill)), 
          Strength(strength), 
          Intelligence(intelligence),
          Dexterity(dexterity),
          Charisma(charisma), 
          Health(health), 
          Mana(mana) {}
};
