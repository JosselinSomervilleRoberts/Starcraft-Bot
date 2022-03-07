#pragma once

#include <BWAPI.h>
#include <vector>

#include "WorkerManager.h"
#include "ArmyManager.h"
#include "BuildQueue.h"
#include "Tools.h"


class GlobalManager;

class BaseManager {

public:
    BaseManager(GlobalManager* manager_);
    BaseManager(GlobalManager* manager_, BWAPI::Unit commandCenter);
    BaseManager(GlobalManager* manager_, int baseNumber_, BWAPI::Unit worker);

    void update();
    void setBuildOrder(std::vector<BWAPI::Unit> buildOrder);

    void constructCommandCenter(BWAPI::Unit worker);
    void newWorker(int importance = 0);
    BWAPI::Unit acquireWorker();
    
    void transmit_expansion();
    void unitCreated(BWAPI::Unit unit);
    void unitDestroyed(BWAPI::Unit unit);
    void unitCompleted(BWAPI::Unit unit);
    void initializeQueue(std::vector<std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType>> unitQueue, std::vector<int> priorityQueue);
    void initialize(BWAPI::Unit commandCenter);

    void setRessourceAim(int cristalAim, int gasAim);
    void setRefineryState(BuildingState state);
    void constructRefinery(int additionalPriority);

    BuildQueue* getQueue() { return &this->queue; };

private:
    WorkerManager workerManager;
    ArmyManager armyManager;
    int baseNumber;
    BWAPI::Position basePosition = Tools::GetDepot()->getPosition();
    std::vector<BWAPI::Unit> buildings;
    BuildQueue queue;
    GlobalManager* manager;
};
