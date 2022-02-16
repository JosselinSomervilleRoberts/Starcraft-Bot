#include "BuildQueue.h"
#include "GlobalManager.h"


#include <cassert>
#include <stdexcept>
#include <type_traits>



using namespace BWAPI;

BuildQueue::BuildQueue(GlobalManager* manager)
    : m_manager(manager) {
    std::cout << "COnstructor buildQueue" << std::endl;
}



void BuildQueue::addTask(UnitType toBuild, int priority, TilePosition position) {
    std::cout << "Add Task : toBuild " << toBuild << " - " << m_buildQueue.size() << std::endl;
    BuildTask* buildtask = new BuildTask(m_manager, toBuild, priority, position);
    

    if (m_buildQueue.size() == 0 || priority <= m_buildQueue[m_buildQueue.size() - 1]->getPriority()) {
        m_buildQueue.push_back(buildtask);
    }
    else {
        auto Pos = m_buildQueue.begin();
        for (int i = 0; i < m_buildQueue.size(); i++) {
            if (m_buildQueue[i]->getPriority() <= priority) {
                m_buildQueue.insert(Pos + i, buildtask); // Insert at the right position
                i = m_buildQueue.size(); // To end the for loop
            }
        }
    }

}

BuildTask* BuildQueue::getTask(int i) {
    return m_buildQueue[i];
}


void BuildQueue::update() {
    for (auto& buildTask : m_buildQueue)
        buildTask->update();

    for (int i = 0; i < m_buildQueue.size(); i++) {
        if (m_buildQueue[i]->getState() == BuildTask::State::finalize) {
            m_buildQueue.erase(m_buildQueue.begin() + i);
            i--;
        }
    }
}
void BuildQueue::clearAll() {
    for (auto& item : m_buildQueue)
        delete[] item;
    m_buildQueue.clear();
}
std::string BuildQueue::toString() const {
    std::string building_list = "";
    for (auto building : m_buildQueue)
        building_list += building->getName() + " ";
    return "BuildQueue: " + building_list;
}



void BuildQueue::unitCompleted(BWAPI::Unit unit) {
    for (int i = 0; i < m_buildQueue.size(); i++) {
        m_buildQueue[i]->onUnitCreatedOrMorphed(unit);
    }
}
