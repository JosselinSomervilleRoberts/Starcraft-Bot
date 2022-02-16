#pragma once

#include <BWAPI.h>
#include <string>



    class GlobalManager;

    class BuildTask {
    public:
        enum class State {
            initialize,
            reserveResources,
            acquireWorker,
            moveToPosition,
            startBuild,
            waitForUnit,
            building,
            finalize
        };

        // The main purpose of priority is to handle resource allocation of build tasks. The allocation
        // of resources for tasks with the same priority is "greedy". If a task is able to allocate all
        // its resources, it will do so. For tasks with different priorities, the lesser tasks will
        // always have to wait for the higher tasks to allocate their resources.

    public:
        BuildTask(GlobalManager* manager, BWAPI::UnitType toBuild, int priority = 0,
            BWAPI::TilePosition position = BWAPI::Broodwar->self()->getStartLocation(),
            bool                exactPosition = false);

        // Called every KBot::onFrame().
        void update();

        bool onUnitCreatedOrMorphed(const BWAPI::Unit& unit);
        bool onUnitDestroyed(const BWAPI::Unit& unit);

        State       getState() const { return m_state; }
        int    getPriority() const { return m_priority; }
        std::string toString() const;

    private:
        GlobalManager* m_manager;
        BWAPI::UnitType     m_toBuild;
        int            m_priority;
        BWAPI::TilePosition m_position;
        bool                m_exactPosition;

        State               m_state = State::initialize;
        BWAPI::Unit         m_worker = nullptr;
        bool                m_allocatedBuildPosition = false;
        BWAPI::TilePosition m_buildPosition;
        BWAPI::Unit         m_buildingUnit = nullptr;
    };

// namespace
