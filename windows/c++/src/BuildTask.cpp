#include "BuildTask.h"

#include "GlobalManager.h"
#include <cassert>
#include <stdexcept>
#include <type_traits>



BuildTask::BuildTask(GlobalManager* manager, BWAPI::UnitType toBuild, int priority, BWAPI::TilePosition position,
    bool exactPosition)
: m_manager(manager), m_toBuild(std::move(toBuild)), m_priority(priority),
    m_position(std::move(position)), m_exactPosition(exactPosition) {}

void BuildTask::update() {
    // ----- Prevent spamming -----------------------------------------------
    // Everything below is executed only occasionally and not on every frame.
    if (BWAPI::Broodwar->getFrameCount() % BWAPI::Broodwar->getLatencyFrames() != 0)
        return;

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
        m_worker = m_manager->acquireWorker();// m_toBuild.whatBuilds().first);// , Position(m_position));
        if (m_worker != nullptr) {
            // Go to next state
            if (m_toBuild.isBuilding())
                m_state = State::moveToPosition;
            else
                m_state = State::startBuild;
        }
        break;
    case State::moveToPosition: {
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
                    m_state = State::waitForUnit; // go to next state
            }
            else {
                m_allocatedBuildPosition = false;
                m_state = State::moveToPosition; // go back and try again
            }
        }
        else {
            // Train unit
            if (m_worker->train(m_toBuild))
                m_state = State::waitForUnit; // go to next state
        }
        break;
    case State::waitForUnit:
        if (m_buildingUnit != nullptr) {
            m_manager->releaseRessources(m_toBuild.mineralPrice(), m_toBuild.gasPrice());
            m_state = State::building; // go to next state
        }
        break;
    case State::building:
        assert(m_buildingUnit != nullptr);
        if (m_buildingUnit->isCompleted()) {
            m_manager->releaseWorker(m_worker);
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
}

std::string BuildTask::toString() const {
    switch (m_state) {
    case State::initialize:
        return m_toBuild.getName() + ": Initialization";
    case State::reserveResources:
        return m_toBuild.getName() + ": Reserving resources...";
    case State::acquireWorker:
        return m_toBuild.getName() + ": Acquiring worker...";
    case State::moveToPosition:
        return m_toBuild.getName() + ": Moving to position...";
    case State::startBuild:
    case State::waitForUnit:
        return m_toBuild.getName() + ": Start building...";
    case State::building: {
        const int progress =
            100 - (100 * m_buildingUnit->getRemainingBuildTime() / m_toBuild.buildTime());
        return m_toBuild.getName() + " (" + std::to_string(progress) + " %)";
    }
    case State::finalize:
        return m_toBuild.getName() + ": Finalization";
    default:
        throw std::logic_error("Unknown BuildTask::State!");
    }
}

