#pragma once

#include "BuildTask.h"
#include "WorkerManager.h"
#include <BWAPI.h>
#include <vector>



    class StarterBot;

    class Manager {
    public:
        Manager(StarterBot& bot);

        // Prohibit copy & move. There is only one manager.
        Manager(const Manager&) = delete;
        Manager(Manager&&) = delete;
        Manager& operator=(const Manager&) = delete;
        Manager& operator=(Manager&&) = delete;

        // Called every KBot::onFrame().
        void update();

        // Transfer ownership of a unit to manager.
        void giveOwnership(const BWAPI::Unit& unit);

        // Take ownership of a unit from manager (forcibly).
        void takeOwnership(const BWAPI::Unit& unit);

        void addBuildTask(const BuildTask& buildTask);
        // const auto &getBuildQueue() const { return m_buildQueue; }

        // Notify BuildTasks
        void buildTaskOnUnitCreatedOrMorphed(const BWAPI::Unit& unit);
        void buildTaskOnUnitDestroyed(const BWAPI::Unit& unit);
        void buildTaskOnUnitCompleted(const BWAPI::Unit& unit);

        // Interface for BuildTask
        int  getAvailableMinerals() const;
        int  getAvailableGas() const;
        bool acquireResources(int minerals, int gas, BuildTask::Priority priority);
        void releaseResources(int minerals, int gas);
        BWAPI::Unit acquireWorker(const BWAPI::UnitType& workerType,
            const BWAPI::Position& nearPosition);
        void releaseWorker(const BWAPI::Unit& worker);

    private:
        StarterBot& m_bot;
        std::vector<BuildTask>   m_buildQueue;
        int                      m_reservedMinerals = 0;
        int                      m_reservedGas = 0;
        std::vector<BWAPI::Unit> m_workers; // for build tasks

        WorkerManager workerManager;
    };
