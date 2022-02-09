#pragma once

#include <BWAPI.h>
#include <vector>

#include "BaseManager.h"
#include "Tools.h"


class GlobalManager {

public:
    GlobalManager();

    void initiate();
    void update();

    void reserveGas(int gas_);
    void reserveMinerals(int minerals_);
    void reserveRessources(int minerals_, int gas_);
    void releaseGas(int gas_);
    void releaseMinerals(int minerals_);
    void releaseRessources(int minerals_, int gas_);
    int getAvailableGas();
    int getAvailableMinerals();

    BWAPI::Unit acquireWorker();


    void onUnitCreate(BWAPI::Unit unit);
    void onUnitDestroy(BWAPI::Unit unit);
    void onUnitComplete(BWAPI::Unit unit);


private:
    std::vector<BaseManager> bases;
    int reservedGas = 0;
    int reservedMinerals = 0;
};
