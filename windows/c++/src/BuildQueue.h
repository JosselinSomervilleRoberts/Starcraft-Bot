#pragma once

#include <BWAPI.h>
#include <string>
#include "BuildTask.h"
#include <vector>



    class GlobalManager;

    class BuildQueue {
    public:
        
        BuildQueue(GlobalManager* manager);
        void addTask(std::variant<BWAPI::UnitType, BWAPI::UpgradeType> toBuild, int priority = 0, BWAPI::TilePosition position = BWAPI::Broodwar->self()->getStartLocation(), bool unique = false);
        BuildTask* getTask(int Position);
        // Called every KBot::onFrame().
        void update();
        void clearAll();
        std::string toString() const;

        void unitCompleted(BWAPI::Unit unit);
        void computeNeed();

        int getPriorityOfGas();

    private:
        std::vector<BuildTask*> m_buildQueue;
        GlobalManager* m_manager;
        
    };

 // namespace
