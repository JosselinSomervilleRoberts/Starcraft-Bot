#pragma once

#include <BWAPI.h>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

#include "Tools.h"

class BaseManager;


class ArmyManager {

public:
    enum class Mode {
        defense, 
        normal,
        attack
    };

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
    void attack(std::vector<BWAPI::Unit> soldier, BWAPI::Unit threat);
    std::vector<BWAPI::Unit> attackSoldiers;
    std::vector<BWAPI::Unit> defenseSoldiers;
    std::vector<BWAPI::Unit> patrolSoldiers;
    void patrol(BWAPI::Position position, std::vector<BWAPI::Unit> soldiers);
    
private:
    BaseManager* base;
    std::vector<BWAPI::Unit> soldiers;
    
    std::vector<BWAPI::Unit> ennemies;

    Mode               mode = Mode::defense;
    int nbSoldiersTotal = 0;

};
