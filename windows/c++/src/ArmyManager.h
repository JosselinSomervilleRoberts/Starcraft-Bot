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

    void computeRepartition();
    void checkRepartition();


private:
    BaseManager* base;
    std::vector<BWAPI::Unit> soldiers;
    std::vector<BWAPI::Unit> soldiersAvailable;
    int nbSoldiersTotal = 0;

};
