#include "BuildTask.h"

#include "GlobalManager.h"
#include <cassert>
#include <stdexcept>
#include <type_traits>



BuildTask::BuildTask(GlobalManager* manager, BWAPI::UnitType toBuild, int priority, BWAPI::TilePosition position,
    bool exactPosition)
: m_manager(manager), m_toBuild(std::move(toBuild)), m_priority(priority),
    m_position(std::move(position)), m_exactPosition(exactPosition) {}
BuildTask::BuildTask(GlobalManager* manager, BWAPI::UpgradeType toUpgrade, int priority, BWAPI::TilePosition position,
    bool exactPosition)
    : m_manager(manager), m_toUpgrade(std::move(toUpgrade)), m_priority(priority),
    m_position(std::move(position)), m_exactPosition(exactPosition) {}
BuildTask::BuildTask(GlobalManager* manager, BWAPI::TechType techUpgrade, int priority, BWAPI::TilePosition position,
    bool exactPosition)
    : m_manager(manager), m_techUpgrade(std::move(techUpgrade)), m_priority(priority),
    m_position(std::move(position)), m_exactPosition(exactPosition) {}

void BuildTask::update(bool& enoughMinerals, bool& enoughGas) {
    // ----- Prevent spamming -----------------------------------------------
    // Everything below is executed only occasionally and not on every frame.
    //if (BWAPI::Broodwar->getFrameCount() % BWAPI::Broodwar->getLatencyFrames() != 0)
    //    return;

    //std::cout << "Build task update " << this->toString() << std::endl;
    const auto refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    const auto pylonType = BWAPI::UnitTypes::Protoss_Pylon;
    if (m_toBuild.getID() != BWAPI::UnitTypes::Enum::None) {

<<<<<<< HEAD
        switch (m_state) {
        case State::initialize:
            // TODO: Make checks and stuff...
            m_state = State::reserveResources; // go to next state
            break;
        case State::reserveResources:
            if (m_manager->reserveRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice()))
                m_state = State::acquireWorker; // go to next state
            break;
        case State::acquireWorker:
            if (m_toBuild.isBuilding()) {
                m_worker = m_manager->acquireWorker();// m_toBuild.whatBuilds().first);// , Position(m_position));
                if (m_worker != nullptr) {
                    // Go to next state
                    if (!m_toBuild.requiresPsi() || Tools::GetUnitOfType(pylonType))
                        m_state = State::moveToPosition;
                }
            }
            else
                m_state = State::startBuild;
            break;

        case State::moveToPosition: 
            m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
            if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTING);
            
            if (!m_allocatedBuildPosition) {
                m_buildPosition =
                    m_exactPosition ? m_position : BWAPI::Broodwar->getBuildLocation(m_toBuild, m_position);                
            }
            if (m_buildPosition.isValid()) {
                m_allocatedBuildPosition = true;
                assert(m_worker != nullptr);
                const BWAPI::Position movePosition =
                    BWAPI::Position(m_buildPosition) + BWAPI::Position(m_toBuild.tileSize()) / 2;

                // DEBUG
                BWAPI::Broodwar->registerEvent([worker = m_worker, movePosition](BWAPI::Game*) {
                    BWAPI::Broodwar->drawLineMap(worker->getPosition(), movePosition, BWAPI::Colors::Purple);
                    BWAPI::Broodwar->drawTextMap(worker->getPosition(), "Distance: %d",
                        worker->getDistance(movePosition));
                    },
                    nullptr, BWAPI::Broodwar->getLatencyFrames());

                if (m_worker->getOrder() != BWAPI::Orders::Move ||
                    m_worker->getOrderTargetPosition() != movePosition)
                    m_worker->move(movePosition);
                if (m_worker->getDistance(movePosition) < 100) // TODO!
                    m_state = State::startBuild;               // go to next state
            }
            break;
        
        case State::startBuild:
            if (m_toBuild.isBuilding()) {
                // Construct building
                if (BWAPI::Broodwar->canBuildHere(m_buildPosition, m_toBuild, m_worker)) {
                    if (m_worker->build(m_toBuild, m_buildPosition))
                        m_state = State::waitForUnit; // go to next state
                }
                else {
                    m_allocatedBuildPosition = false;
                    m_state = State::moveToPosition; // go back and try again
                }
            }
            else if (true) {
                // Train unit
                const BWAPI::Unit trainBuilding = Tools::GetUnitOfType(m_toBuild.whatBuilds().first);
                
                if (trainBuilding && trainBuilding->train(m_toBuild)) {
                    m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
=======
    switch (m_state) {
    case State::initialize:
        // TODO: Make checks and stuff...
        m_state = State::reserveResources; // go to next state
        break;
    case State::reserveResources:
        if (m_manager->reserveRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice(), enoughMinerals, enoughGas))
            m_state = State::acquireWorker; // go to next state
        break;
    case State::acquireWorker:
        if (m_toBuild.isBuilding()) {
            m_worker = m_manager->acquireWorker();// m_toBuild.whatBuilds().first);// , Position(m_position));
            if (m_worker != nullptr) {
                // Go to next state
                m_state = State::moveToPosition;
            }
        }
        else
            m_state = State::startBuild;
        break;

    case State::moveToPosition: {
        if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTING);
        if (!m_allocatedBuildPosition) {
            m_buildPosition =
                m_exactPosition ? m_position : BWAPI::Broodwar->getBuildLocation(m_toBuild, m_position);
            m_allocatedBuildPosition = true;
        }
        assert(m_worker != nullptr);
        const BWAPI::Position movePosition =
            BWAPI::Position(m_buildPosition) + BWAPI::Position(m_toBuild.tileSize()) / 2;

        // DEBUG
        BWAPI::Broodwar->registerEvent([worker = m_worker, movePosition](BWAPI::Game*) {
            BWAPI::Broodwar->drawLineMap(worker->getPosition(), movePosition, BWAPI::Colors::Purple);
            BWAPI::Broodwar->drawTextMap(worker->getPosition(), "Distance: %d",
                worker->getDistance(movePosition));
            },
            nullptr, BWAPI::Broodwar->getLatencyFrames());

        if (m_worker->getOrder() != BWAPI::Orders::Move ||
            m_worker->getOrderTargetPosition() != movePosition)
            m_worker->move(movePosition);
        if (m_worker->getDistance(movePosition) < 100) // TODO!
            m_state = State::startBuild;               // go to next state
        break;
    }
    case State::startBuild:
        if (m_toBuild.isBuilding()) {
            // Construct building
            if (BWAPI::Broodwar->canBuildHere(m_buildPosition, m_toBuild, m_worker)) {
                if (m_worker->build(m_toBuild, m_buildPosition))
>>>>>>> origin/main
                    m_state = State::waitForUnit; // go to next state
                }
            }
            else {
                
                /*auto units = BWAPI::PlayerInterface::getUnits();
                for (auto unit : units) {
                    if(unit.canUpgrade(m_toUpgrade))

                }*/
            }
            break;
        case State::waitForUnit:
            if (m_buildingUnit != nullptr) {
                m_state = State::building; // go to next state
            }
            break;
        case State::building:
            if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTED);
            if (m_toBuild.isBuilding()) {
                m_manager->releaseWorker(m_worker);
            }
            m_state = State::finalize; // go to next state
            break;
        case State::finalize:
            // At this state, this build task can be removed from the queue.
            break;
        default:
            throw std::logic_error("Unknown BuildTask::State!");
        }
    }
    else if(m_toUpgrade.getID() != BWAPI::UpgradeTypes::Enum::None){ // If it is an upgrade
        switch (m_state) {
        case State::initialize:
            // TODO: Make checks and stuff...
            m_state = State::reserveResources; // go to next state
            break;
        case State::reserveResources:
            if (m_manager->reserveRessources(m_toUpgrade.mineralPrice(), m_toUpgrade.gasPrice()))
                m_state = State::startBuild; // go to next state
            break;

        case State::startBuild:
            if (true) {
                const BWAPI::Unit upgradeBuilding = Tools::GetUnitOfType(m_toUpgrade.whatUpgrades());
                if (upgradeBuilding && !upgradeBuilding->isResearching() && !upgradeBuilding->isUpgrading()) {
                    if (upgradeBuilding->upgrade(m_toUpgrade)) {
                        m_manager->releaseRessources(m_toUpgrade.mineralPrice(), m_toUpgrade.gasPrice());
                        m_state = State::finalize;
                    }
                }
                // Train unit
                /* auto units = BWAPI::PlayerInterface::getUnits();
                for (auto unit : units) {
                    if (unit.canUpgrade(m_toUpgrade))
                        unit->upgrade(m_toUpgrade);

                }*/
            
            }
            else {
                /*auto units = BWAPI::PlayerInterface::getUnits();
                for (auto unit : units) {
                    if(unit.canUpgrade(m_toUpgrade))

                }*/
            }
            break;
        case State::waitForUnit:
            if (m_buildingUnit != nullptr) {
                m_state = State::building; // go to next state
            }
            break;
        case State::building:
            if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTED);
            if (m_toBuild.isBuilding()) {
                m_manager->releaseWorker(m_worker);
            }
            m_state = State::finalize; // go to next state
            break;
        case State::finalize:
            // At this state, this build task can be removed from the queue.
            break;
        default:
            throw std::logic_error("Unknown BuildTask::State!");
        }
<<<<<<< HEAD
    }
    else { // If it is an upgrade
        switch (m_state) {
            case State::initialize:
                // TODO: Make checks and stuff...
                m_state = State::reserveResources; // go to next state
                break;
            case State::reserveResources:
                if (m_manager->reserveRessources(m_techUpgrade.mineralPrice(), m_techUpgrade.gasPrice()))
                    m_state = State::startBuild; // go to next state
                break;

            case State::startBuild:
                if (true) {
                    const BWAPI::Unit upgradeBuilding = Tools::GetUnitOfType(m_techUpgrade.whatResearches());
                    if (upgradeBuilding && !upgradeBuilding->isResearching() && !upgradeBuilding->isUpgrading()) {
                        if (upgradeBuilding->research(m_techUpgrade)) {
                            m_manager->releaseRessources(m_techUpgrade.mineralPrice(), m_techUpgrade.gasPrice());
                            m_state = State::finalize;
                        }
                    }
                    // Train unit
                    /* auto units = BWAPI::PlayerInterface::getUnits();
                    for (auto unit : units) {
                        if (unit.canUpgrade(m_toUpgrade))
                            unit->upgrade(m_toUpgrade);

                    }*/

                }
                else {
                    /*auto units = BWAPI::PlayerInterface::getUnits();
                    for (auto unit : units) {
                        if(unit.canUpgrade(m_toUpgrade))

                    }*/
                }
                break;
            case State::waitForUnit:
                if (m_buildingUnit != nullptr) {
                    m_state = State::building; // go to next state
                }
                break;
            case State::building:                
                m_state = State::finalize; // go to next state
                break;
            case State::finalize:
                // At this state, this build task can be removed from the queue.
                break;
            default:
                throw std::logic_error("Unknown BuildTask::State!");
        }
=======
        break;
    case State::building:
        if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTED);
        if (m_toBuild.isBuilding()) {
            m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
            m_manager->releaseWorker(m_worker);
        }
        m_state = State::finalize; // go to next state
        break;
    case State::finalize:
        m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
        // At this state, this build task can be removed from the queue.
        break;
    default:
        throw std::logic_error("Unknown BuildTask::State!");
>>>>>>> origin/main
    }
}

bool BuildTask::onUnitCreatedOrMorphed(const BWAPI::Unit& unit) {
    // Accept new unit if we are waiting for one.
    if (m_state != State::waitForUnit)
        return false;

    // Make sure we don't already have a unit and it's the right type.
    if (m_buildingUnit == nullptr && unit->getType() == m_toBuild) {
        m_buildingUnit = unit;
        return true;
    }

    return false;
}

bool BuildTask::onUnitDestroyed(const BWAPI::Unit& unit) {
    if (unit == m_worker) {
        // FIXME!
        return true;
    }
    if (unit == m_buildingUnit) {
        // FIXME!
        return true;
    }
    return false;


    const auto refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::NOT_BUILT);

}
std::string BuildTask::getName() const {
    if (m_toBuild.getID() != BWAPI::UnitTypes::Enum::None) {
        return m_toBuild.getName();
    }
    return m_toUpgrade.getName();
}

std::string BuildTask::toString() const {
    switch (m_state) {
    case State::initialize:
        return "Initialization";
    case State::reserveResources:
        return "Reserving resources...";
    case State::acquireWorker:
        return "Acquiring worker...";
    case State::moveToPosition:
        return "Moving to position...";
    case State::startBuild:
    case State::waitForUnit:
        return "Start building...";
    case State::building: {
        const int progress =
            100 - (100 * m_buildingUnit->getRemainingBuildTime() / m_toBuild.buildTime());
        return m_toBuild.getName() + " (" + std::to_string(progress) + " %)";
    }
    case State::finalize:
        return "Finalization";
    default:
        throw std::logic_error("Unknown BuildTask::State!");
    }
}

std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> BuildTask::getObject() {
    
    if (m_toBuild.getID() != BWAPI::UnitTypes::Enum::None) {
        return m_toBuild;
    }
    else if (m_toBuild.getID() != BWAPI::UpgradeTypes::Enum::None) {
        return m_toUpgrade;
    }
    return m_techUpgrade;
}

