#pragma once

#include <BWAPI.h>
#include <vector>

#include "WorkerManager.h"

class BaseManager {

public:
    BaseManager();
    BaseManager(int baseNumber_, BWAPI::Unit worker);
    void constructCommandCenter(BWAPI::Unit worker);


private:
    WorkerManager workerManager;
    int baseNumber;
    std::vector<BWAPI::Unit> buildings;
};
