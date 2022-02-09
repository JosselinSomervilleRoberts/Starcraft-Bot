#pragma once

#include <BWAPI.h>
#include <string>
#include "BuildTask.h"
#include <vector>
#include "GlobalManager.h"


namespace BWAPI {

    class GlobalManager;

    class BuildQueue {
    public:
    public:
        
        BuildQueue(GlobalManager& manager);
        void addTask(UnitType toBuild, int priority = 0, TilePosition position = BWAPI::Broodwar->self()->getStartLocation());
        BuildTask* getTask(int Position);
        // Called every KBot::onFrame().
        void update();

        std::string toString() const;

    private:
        std::vector<BuildTask*> m_buildQueue;
        GlobalManager* m_manager;
        
    };

} // namespace
