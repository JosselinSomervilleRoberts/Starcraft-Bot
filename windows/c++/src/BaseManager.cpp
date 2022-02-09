#include "BaseManager.h"
#include "GlobalManager.h"

BaseManager::BaseManager(GlobalManager* manager_) : workerManager(this), queue(manager_), manager(manager_) {
    const BWAPI::UnitType commandCenterType = BWAPI::Broodwar->self()->getRace().getCenter();
    const BWAPI::Unit commandCenter = Tools::GetUnitOfType(commandCenterType);
    BaseManager(manager_, commandCenter);
}


BaseManager::BaseManager(GlobalManager* manager_, BWAPI::Unit commandCenter) : workerManager(this), queue(manager_), manager(manager_) {
    baseNumber = 0;
    this->buildings.push_back(commandCenter);
    std::cout << "CONSTRUCTOR" << std::endl;

    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
            workerManager.addWorker(unit);
    }
}

BaseManager::BaseManager(GlobalManager* manager_, int baseNumber_, BWAPI::Unit worker) : workerManager(this), queue(manager_), manager(manager_) {
    baseNumber = baseNumber_;
    this->constructCommandCenter(worker);
}

void BaseManager::update() {
    queue.update();
    workerManager.update();


    // TO TEST
    if (manager->getAvailableMinerals() > 300) {
        BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
        queue.addTask(workerType, 200);
    }
}

void BaseManager::setBuildOrder(std::vector<BWAPI::Unit> buildOrder) {
    // TODO
}


void BaseManager::constructCommandCenter(BWAPI::Unit worker) {
    BWAPI::UnitType centerType = BWAPI::Broodwar->self()->getRace().getCenter();
    // TODO : construct command center (without using the queue)
}


void BaseManager::newWorker() {
    BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    //queue.addTask(workerType, 10);
}

void BaseManager::constructRefinery(int importance) {
    BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    queue.addTask(refineryType, 20);
    workerManager.refineryState = BuildingState::CONSTRUCTING;
}



void BaseManager::transmit_expansion() {
    // TODO : transmit to GlobalManager to ask for a new base
}


BWAPI::Unit BaseManager::acquireWorker() {
    workerManager.findAvailableWorkers(1);
    return workerManager.getAvailableWorker();
}



void BaseManager::unitCreated(BWAPI::Unit unit) {
}

void BaseManager::unitDestroyed(BWAPI::Unit unit) {
    if (unit->getType().isBuilding()) {
        std::remove(buildings.begin(), buildings.end(), unit);

        // Here decide what we do
        // TODO : change to have more clever behaviour
        BWAPI::UnitType type = unit->getType();
        queue.addTask(type, 100);
    }
}

void BaseManager::unitCompleted(BWAPI::Unit unit) {
    BWAPI::UnitType type = unit->getType();

    BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    if (type == workerType)
        workerManager.addWorker(unit);

    if (type.isBuilding()) {
        buildings.push_back(unit);

        BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
        if(type == refineryType)
            workerManager.refineryState = BuildingState::CONSTRUCTED;
    }
}