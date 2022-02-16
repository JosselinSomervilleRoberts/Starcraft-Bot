#pragma once

#include <BWAPI.h>
#include <string>
#include "BuildTask.h"
#include <vector>



    class GlobalManager;

    class BuildQueue {
    public:
        
        BuildQueue(GlobalManager* manager);
        void addTask(BWAPI::UnitType toBuild, int priority = 0, BWAPI::TilePosition position = BWAPI::Broodwar->self()->getStartLocation());
        BuildTask* getTask(int Position);
        // Called every KBot::onFrame().
        void update();
        void clearAll();
        std::string toString() const;

        void unitCompleted(BWAPI::Unit unit);
        void computeNeed();

    private:
        std::vector<BuildTask*> m_buildQueue;
        GlobalManager* m_manager;
        
    };

 // namespace
