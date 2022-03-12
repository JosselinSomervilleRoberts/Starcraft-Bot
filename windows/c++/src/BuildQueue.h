#pragma once

#include <BWAPI.h>
#include <string>
#include "BuildTask.h"
#include <vector>



    class GlobalManager;

    class BuildQueue {
    public:
        
        BuildQueue(GlobalManager* manager, int index_base_);

        void addTask(std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> toBuild, int priority = 0, BWAPI::TilePosition position = BWAPI::Broodwar->self()->getStartLocation(), bool unique = false, bool exactPos = false);
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
        void unitDestroyed(BWAPI::Unit unit);

        int getPriorityOfGas();

        int countUnitTypeInQueue(BWAPI::UnitType type);
        int countUnitTypeInTotal(BWAPI::UnitType type);

    private:
        // Idea : put defense cannon all around 1st base chokepoint, maybe 2nd base ? put attack cannon close to the enemy base to destroy from the beginning 
        std::vector<BWAPI::TilePosition> pylonPos = {BWAPI::TilePosition(49,109)};
        std::vector<BWAPI::TilePosition> cannonPos = {BWAPI::TilePosition(47,107)}; 
        std::vector<BuildTask*> m_buildQueue;
        GlobalManager* m_manager;
        int index_base = 0;
    };

 // namespace
