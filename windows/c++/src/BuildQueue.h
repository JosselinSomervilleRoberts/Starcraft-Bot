#pragma once

#include <BWAPI.h>
#include <string>
#include "BuildTask.h"
#include <vector>



    class GlobalManager;

    class BuildQueue {
    public:
        
        BuildQueue(GlobalManager* manager, int index_base_);

        void addTask(std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> toBuild, int priority = 0, BWAPI::TilePosition position = BWAPI::Broodwar->self()->getStartLocation(), bool unique = false);
        void addTask(BWAPI::UnitType toBuild, int priority = 0, bool unique = false);
        void addTask(BWAPI::UpgradeType toBuild, int priority = 0, bool unique = false);
        void addTask(BWAPI::TechType toBuild, int priority = 0, bool unique = false);
        BuildTask* getTask(int Position);

        // Called every KBot::onFrame().
        void update();
        void clearAll();
        std::string toString() const;

        void unitCompleted(BWAPI::Unit unit);
        void unitCreated(BWAPI::Unit unit);
        void computeNeed(bool once = false);

        int getPriorityOfGas();

        int countUnitTypeInQueue(BWAPI::UnitType type);
        int countUnitTypeInTotal(BWAPI::UnitType type);

    private:
        std::vector<BuildTask*> m_buildQueue;
        GlobalManager* m_manager;
        int index_base = 0;
    };

 // namespace
