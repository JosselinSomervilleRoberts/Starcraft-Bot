#include "BaseManager.h"
#include "GlobalManager.h"

BaseManager::BaseManager(GlobalManager* manager_) : workerManager(this), armyManager(this), queue(manager_), manager(manager_) {
    const BWAPI::UnitType commandCenterType = BWAPI::Broodwar->self()->getRace().getCenter();
    const BWAPI::Unit commandCenter = Tools::GetUnitOfType(commandCenterType);
    initialize(commandCenter);
   
}


BaseManager::BaseManager(GlobalManager* manager_, BWAPI::Unit commandCenter) : workerManager(this), armyManager(this), queue(manager_), manager(manager_) {
    initialize(commandCenter);
}



BaseManager::BaseManager(GlobalManager* manager_, int baseNumber_, BWAPI::Unit worker) : workerManager(this), armyManager(this), queue(manager_), manager(manager_) {
    baseNumber = baseNumber_;
    this->constructCommandCenter(worker);

}

void BaseManager::initialize(BWAPI::Unit commandCenter) {
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

void BaseManager::initializeQueue(std::vector<std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType>> unitQueue, std::vector<int> priorityQueue) {
    queue.clearAll();
    for (int i = 0; i < unitQueue.size(); i++) {
        std::cout << "initialize " << i << std::endl;
        queue.addTask(unitQueue[i], priorityQueue[i]);
    }

    std::cout << "QUEUE after Init" << queue.toString() << std::endl;
}
void BaseManager::update() {
    queue.update();
    bool isEnemy = false;
    for (auto enemy : BWAPI::Broodwar->enemy()->getUnits()) {
        if (enemy->getPosition().getDistance(basePosition) < 1000) {
            armyManager.onAttack(enemy);
            isEnemy = true;
            break;
        }

    }
    if (!isEnemy)
        armyManager.noAttack();

    workerManager.update();
    armyManager.update();

}

void BaseManager::setBuildOrder(std::vector<BWAPI::Unit> buildOrder) {
    // TODO
}


void BaseManager::constructCommandCenter(BWAPI::Unit worker) {
    BWAPI::UnitType centerType = BWAPI::Broodwar->self()->getRace().getCenter();
    // TODO : construct command center (without using the queue)



}


void BaseManager::newWorker(int importance) {
    auto workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    auto supplyType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    queue.addTask(workerType, importance, BWAPI::Broodwar->self()->getStartLocation(), true);
    if (BWAPI::Broodwar->self()->supplyUsed() + 1 >= BWAPI::Broodwar->self()->supplyTotal()) {
        queue.addTask(supplyType, importance + 1, BWAPI::Broodwar->self()->getStartLocation(), true);
        
    }
}



void BaseManager::transmit_expansion() {
    // TODO : transmit to GlobalManager to ask for a new base
}


BWAPI::Unit BaseManager::acquireWorker() {
    workerManager.findAvailableWorkers(1);
    return workerManager.getAvailableWorker();
}



void BaseManager::unitCreated(BWAPI::Unit unit) {
    queue.unitCreated(unit);
}

void BaseManager::unitDestroyed(BWAPI::Unit unit) {
    if (unit->getType().isBuilding()) {
        std::remove(buildings.begin(), buildings.end(), unit);

        // Here decide what we do
        // TODO : change to have more clever behaviour
        BWAPI::UnitType type = unit->getType();
        if (type.isBuilding())
            queue.addTask(type, 100);
    }
    else if (unit->getType() == BWAPI::Broodwar->self()->getRace().getWorker()) {
        queue.addTask(unit->getType(), 50);
        workerManager.onUnitDestroyed(unit);
    }
    else {
        armyManager.onUnitDestroyed(unit);
        queue.addTask(unit->getType(), 75);
    }
}

void BaseManager::unitCompleted(BWAPI::Unit unit) {
    BWAPI::UnitType type = unit->getType();

    BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    if (type == workerType)
        workerManager.addWorker(unit);
    else if (unit->canAttack())
        armyManager.addSoldier(unit);
    if (type.isBuilding()) {
        buildings.push_back(unit);
    }

    queue.unitCompleted(unit);
}


void  BaseManager::setRessourceAim(int cristalAim, int gasAim) {
    // TODO :change for several bases
    workerManager.setAim(cristalAim, gasAim);
    workerManager.computeNeed();
}

void BaseManager::setRefineryState(BuildingState state) {
    // TODO :change for several bases
    workerManager.setRefineryState(state);
}


void BaseManager::constructRefinery(int additionalPriority) {
    int priority = queue.getPriorityOfGas() + additionalPriority;
    auto refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    queue.addTask(refineryType, priority, BWAPI::Broodwar->self()->getStartLocation(), true);
    setRefineryState(BuildingState::WAITING_CONSTRUCTION);
}