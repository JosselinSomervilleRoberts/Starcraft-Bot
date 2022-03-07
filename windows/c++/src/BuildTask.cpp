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



    // =============================== TRAIN =============================== //
    if ((m_toBuild.getID() != BWAPI::UnitTypes::Enum::None) && !(m_toBuild.isBuilding())) {
        switch (m_state) {

            case State::initialize:
                // TODO: Make checks and stuff...
                m_state = State::reserveResources; // go to next state
                break;

            case State::reserveResources:
                if (m_manager->reserveRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice(), enoughMinerals, enoughGas))
                    m_state = State::startBuild; // go to next state
                break;
        
            case State::startBuild: {
                // Train unit
                BWAPI::Unit trainBuilding = nullptr;
                std::vector<BWAPI::Unit> trainBuildings = Tools::GetUnitsOfType(m_toBuild.whatBuilds().first);

                for (int i = 0; i < trainBuildings.size(); i++) {
                    BWAPI::Unit building = trainBuildings[i];

                    if (!(building->isResearching()) && !(building->isConstructing()) && !(building->isTraining())) {
                        trainBuilding = building;
                        i = trainBuildings.size();
                    }
                }

                if (trainBuilding && trainBuilding->train(m_toBuild)) {
                    m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
                    m_state = State::waitForUnit; // go to next state
                }
                break; }

            case State::waitForUnit:
                if (m_buildingUnit != nullptr) {
                    m_state = State::finalize; // go to next state
                }
                break;

            case State::finalize:
                // At this state, this build task can be removed from the queue.
                break;

            default:
                throw std::logic_error("Unknown BuildTask::State!");
        }
    }



    // =============================== BUILD =============================== //
    else if ((m_toBuild.getID() != BWAPI::UnitTypes::Enum::None) && (m_toBuild.isBuilding())) {

        switch (m_state) {

            case State::initialize:
                // TODO: Make checks and stuff...
                m_state = State::reserveResources; // go to next state
                break;

            case State::reserveResources: {
                auto requirement = m_toBuild.whatBuilds();
                if (Tools::CountUnitsOfType(requirement.first, BWAPI::Broodwar->self()->getUnits()) >= requirement.second) { // Can build
                    if (m_manager->reserveRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice(), enoughMinerals, enoughGas)) {
                        m_state = State::acquireWorker; // go to next state
                    }
                }
            } break;

            case State::acquireWorker: {
                    m_worker = m_manager->acquireWorker();// m_toBuild.whatBuilds().first);// , Position(m_position));
                    if (m_worker != nullptr) {
                        // Go to next state
                        if (!m_toBuild.requiresPsi() || Tools::GetUnitOfType(pylonType))
                            m_state = State::moveToPosition;
                    }
                } break;

            case State::moveToPosition:
                if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTING);

                if (!m_allocatedBuildPosition)
                    m_buildPosition = m_exactPosition ? m_position : BWAPI::Broodwar->getBuildLocation(m_toBuild, m_position);
                
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

            case State::startBuild: {
                    // Construct building
                    if (BWAPI::Broodwar->canBuildHere(m_buildPosition, m_toBuild, m_worker)) {
                        if (m_worker->build(m_toBuild, m_buildPosition)) {
                            m_state = State::waitForUnit; // go to next state
                        }
                    }
                    else {
                        m_allocatedBuildPosition = false;
                        m_state = State::moveToPosition; // go back and try again
                    }
                } break;

            case State::waitForUnit:
                if (m_buildingUnit != nullptr) {
                    m_state = State::building; // go to next state
                    m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
                    m_manager->releaseWorker(m_worker);
                    if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTING);
                }
                break;

            case State::building:
                progress = 100 - (100 * m_buildingUnit->getRemainingBuildTime() / m_toBuild.buildTime());
                break;

            case State::finalize:
                if (m_toBuild == refineryType) m_manager->setRefineryState(BuildingState::CONSTRUCTED);
                break;

            default:
                throw std::logic_error("Unknown BuildTask::State!");
        }
    }



    // =============================== UPGRADE =============================== //
    else if(m_toUpgrade.getID() != BWAPI::UpgradeTypes::Enum::None){ // If it is an upgrade
        switch (m_state) {
        case State::initialize:
            // TODO: Make checks and stuff...
            m_state = State::reserveResources; // go to next state
            break;
        case State::reserveResources:
            if (m_manager->reserveRessources(m_toUpgrade.mineralPrice(), m_toUpgrade.gasPrice(), enoughMinerals, enoughGas))
                m_state = State::startBuild; // go to next state
            break;

        case State::startBuild: {

            const BWAPI::Unit upgradeBuilding = Tools::GetUnitOfType(m_toUpgrade.whatUpgrades());
            if (upgradeBuilding && !upgradeBuilding->isResearching() && !upgradeBuilding->isUpgrading()) {
                upgradeBuilding->upgrade(m_toUpgrade); // TODO: fix the check
                 m_manager->releaseRessources(m_toUpgrade.mineralPrice(), m_toUpgrade.gasPrice());
                 m_buildingUnit = upgradeBuilding;
                 m_state = State::building;
            }
            break;
        }
             
        case State::building:
            progress = 100 - (100 * m_buildingUnit->getRemainingUpgradeTime() / m_toUpgrade.upgradeTime());
            if (BWAPI::Broodwar->self()->getUpgradeLevel(m_toUpgrade) > 0)
                m_state = State::finalize;
            break;

        case State::finalize:
            break;
        default:
            throw std::logic_error("Unknown BuildTask::State!");
        }
    }


    // =============================== TECH  =============================== //
    else { // If it is an upgrade
        switch (m_state) {
        case State::initialize:
            // TODO: Make checks and stuff...
            m_state = State::reserveResources; // go to next state
            break;
        case State::reserveResources:
            if (m_manager->reserveRessources(m_techUpgrade.mineralPrice(), m_techUpgrade.gasPrice(), enoughMinerals, enoughGas))
                m_state = State::startBuild; // go to next state
            break;

        case State::startBuild:
            {
            const BWAPI::Unit upgradeBuilding = Tools::GetUnitOfType(m_techUpgrade.whatResearches());
            if (upgradeBuilding && !upgradeBuilding->isResearching() && !upgradeBuilding->isUpgrading()) {
                upgradeBuilding->research(m_techUpgrade); // TODO: fix the check
                m_manager->releaseRessources(m_techUpgrade.mineralPrice(), m_techUpgrade.gasPrice());
                m_buildingUnit = upgradeBuilding;
                m_state = State::building;
            }

            break;
            }
         
            case State::building:
                progress = 100 - (100 * m_buildingUnit->getRemainingResearchTime() / m_techUpgrade.researchTime());
                if (BWAPI::Broodwar->self()->hasResearched(m_techUpgrade))
                    m_state = State::finalize;
                break;

            case State::finalize:
                // At this state, this build task can be removed from the queue.
                break;
            default:
                throw std::logic_error("Unknown BuildTask::State!");
        }
    }
}

bool BuildTask::onUnitCreated(const BWAPI::Unit& unit) {
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

void BuildTask::onUnitCompleted(const BWAPI::Unit& unit) {
    if ((m_state == State::building) && (unit == m_buildingUnit)) {
        m_state = State::finalize;
    }

    if (unit->getType() == BWAPI::Broodwar->self()->getRace().getRefinery()) {
        if (unit->getType() == m_toBuild) {
            m_state = State::finalize;
            m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
            m_manager->releaseWorker(m_worker);
        }
    }
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
        return "Building (" + std::to_string(progress) + ")";
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

