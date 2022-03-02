#pragma once

#include <BWAPI.h>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

#include "Tools.h"

class BaseManager;


class ArmyManager {

public:
    ArmyManager(BaseManager* base_);

    void findAvailableSoldiers(int nbWanted);
    BWAPI::Unit getAvailableSoldier();
    void addSoldier(BWAPI::Unit soldier);
    void update();
    void onUnitDestroyed(BWAPI::Unit unit);
    void computeRepartition();
    void checkRepartition();
    void onAttack(BWAPI::Unit threat);
    void attack(BWAPI::Unit soldier, BWAPI::Unit threat);


private:
    BaseManager* base;
    std::vector<BWAPI::Unit> soldiers;
    std::vector<BWAPI::Unit> soldiersAvailable;
    std::vector<BWAPI::Unit> ennemies;

    int nbSoldiersTotal = 0;

};
