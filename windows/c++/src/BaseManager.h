#pragma once

#include <BWAPI.h>
#include <vector>

#include "WorkerManager.h"

class BuildQueue;

class BaseManager {

public:
    BaseManager();
    BaseManager(BWAPI::Unit commandCenter);
    BaseManager(int baseNumber_, BWAPI::Unit worker);

    void update();
    void setBuildOrder(std::vector<BWAPI::Unit> buildOrder);

    void constructCommandCenter(BWAPI::Unit worker);
    void constructRefinery(int importance = 0);
    void newWorker();
    BWAPI::Unit acquireWorker();
    
    void transmit_expansion();
    void unitCreated(BWAPI::Unit unit);
    void unitDestroyed(BWAPI::Unit unit);
    void unitCompleted(BWAPI::Unit unit);


private:
    WorkerManager workerManager;
    int baseNumber;
    std::vector<BWAPI::Unit> buildings;
    BuildQueue queue;
};
