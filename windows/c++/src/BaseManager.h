#pragma once

#include <BWAPI.h>
#include <vector>

#include "WorkerManager.h"
#include "BuildQueue.h"

class GlobalManager;

class BaseManager {

public:
    BaseManager(GlobalManager* manager_);
    BaseManager(GlobalManager* manager_, BWAPI::Unit commandCenter);
    BaseManager(GlobalManager* manager_, int baseNumber_, BWAPI::Unit worker);

    void update();
    void setBuildOrder(std::vector<BWAPI::Unit> buildOrder);

    void constructCommandCenter(BWAPI::Unit worker);
    void constructRefinery(int importance = 0);
    void newWorker(int importance = 0);
    BWAPI::Unit acquireWorker();
    
    void transmit_expansion();
    void unitCreated(BWAPI::Unit unit);
    void unitDestroyed(BWAPI::Unit unit);
    void unitCompleted(BWAPI::Unit unit);
    void initializeQueue(std::vector<BWAPI::UnitType> unitQueue, std::vector<int> priorityQueue);
    void initialize(BWAPI::Unit commandCenter);


private:
    WorkerManager workerManager;
    int baseNumber;
    std::vector<BWAPI::Unit> buildings;
    BuildQueue queue;
    GlobalManager* manager;
};
