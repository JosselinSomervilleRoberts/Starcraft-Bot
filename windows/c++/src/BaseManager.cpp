#include "BaseManager.h"

BaseManager::BaseManager() {
	baseNumber = 0;

    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
            workerManager.addWorker(unit);
    }
}


BaseManager::BaseManager(int baseNumber_, BWAPI::Unit worker) {
    baseNumber = baseNumber_;
    this->constructCommandCenter(worker);
}


void BaseManager::constructCommandCenter(BWAPI::Unit worker) {
    BWAPI::UnitType center = BWAPI::Broodwar->self()->getRace().getCenter();
    // TODO
}