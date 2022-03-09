#pragma once

#include <BWAPI.h>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort
#include <variant>

#include "Tools.h"
#include "BuildQueue.h"



struct Requirement {
    std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> toBuild;
    int quantity = 1;
    int quantity_current = 0;
    bool satisfied = false;
};


class SquadComposition {

public:
    SquadComposition(BuildQueue* queue_);

    void updateRequirements();
    bool checkRequirements();
    bool checkMinimalRequirements();
    void fixMissingRequirements(int priority);
    void setComposition();

    bool trainUnit(std::vector<BWAPI::Unit>& squad_units, int priority = 5);
    void scaleUp();
    void scaleDown();

    int getMultiplier() { return multiplier; };

public:
    BuildQueue* queue;

    std::vector<Requirement> requirements;
    std::string name;
    std::vector<BWAPI::UnitType> squadTypes;
    std::vector<float> squadProportions;
    int multiplier = 1;
    int level = 1;
};
