#pragma once

#include <BWAPI.h>
#include <vector>
#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

#include "Tools.h"

class BaseManager;

class WorkerManager {

    public:
        WorkerManager(BaseManager* base_);

        void findAvailableWorkers(int nbWanted);
        BWAPI::Unit getAvailableWorker();
        void addWorker(BWAPI::Unit worker);
        void update();

        void setGasWorker(BWAPI::Unit worker);
        void setCristalWorker(BWAPI::Unit worker);

        void computeNeed();
        void computeRepartition();

        void setAim(int cristal, int gas) { cristalAim = cristal; gasAim = gas; };
        void setCristalAim(int cristal) { cristalAim = cristal; };
        void setGasAim(int gas) { gasAim = gas; };

    private:
        BaseManager* base;
        std::vector<BWAPI::Unit> workers;
        std::vector<BWAPI::Unit> workersCristal;
        std::vector<BWAPI::Unit> workersGas; 
        std::vector<BWAPI::Unit> workersAvailable;
        int nbWorkersTotal = 0;
        int nbWorkersCristalWanted = 0;
        int nbWorkersGasWanted = 0;

        int cristalAim = 0;
        int gasAim = 0;
        int cristalNeed = 0;
        int gasNeed = 0;

        bool hasRefinery = false;
};
