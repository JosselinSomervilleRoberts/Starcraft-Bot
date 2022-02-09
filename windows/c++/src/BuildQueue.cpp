#include "BuildQueue.h"


#include <cassert>
#include <stdexcept>
#include <type_traits>



using namespace BWAPI;

BuildQueue::BuildQueue(GlobalManager& manager)
    : m_manager(&manager) {}


void BuildQueue::addTask(UnitType toBuild, int place = 0, TilePosition position = BWAPI::Broodwar->self()->getStartLocation(),
    bool setPlaceInQueue = false) {

    BuildTask* buildtask = new BuildTask(m_manager, toBuild, position);
    if (!setPlaceInQueue) {
        m_buildQueue.push_back(buildtask);
    }
    else {
        auto Pos = m_buildQueue.begin() + place;
        m_buildQueue.insert(Pos, buildtask);
    }

}

void BuildQueue::addTask(UnitType toBuild, int priority = 0, TilePosition position = BWAPI::Broodwar->self()->getStartLocation()) {

    BuildTask* buildtask = new BuildTask(m_manager, toBuild, priority, position);
    auto Pos = m_buildQueue.begin();
    for (int i = 0; i < m_buildQueue.size(); i++) {
        if (m_buildQueue[i]->getPriority() <= priority) {
            m_buildQueue.insert(Pos+i, buildtask); // Insert at the right position
            i = m_buildQueue.size(); // To end the for loop
        }
    }

}

BuildTask* BuildQueue::getTask(int i) {
    return m_buildQueue[i];
}


void BuildQueue::update() {
    for (auto& buildTask : m_buildQueue)
        buildTask->update();
}

