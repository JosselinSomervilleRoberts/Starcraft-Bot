#include "BuildQueue.h"
#include "GlobalManager.h"


#include <cassert>
#include <stdexcept>
#include <type_traits>



using namespace BWAPI;

BuildQueue::BuildQueue(GlobalManager* manager)
    : m_manager(manager) {
    std::cout << "Constructor buildQueue" << std::endl;
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
    computeNeed();
}

BuildTask* BuildQueue::getTask(int i) {
    return m_buildQueue[i];
}


void BuildQueue::update() {
    computeNeed();
    for (auto& buildTask : m_buildQueue)
        buildTask->update();

    for (int i = 0; i < m_buildQueue.size(); i++) {
        if (m_buildQueue[i]->getState() == BuildTask::State::finalize) {
            m_buildQueue.erase(m_buildQueue.begin() + i);
            i--;
        }
    }

    
    for (int i = 0; i < std::min((size_t)(5), m_buildQueue.size()); i++) {
        std::string s = std::to_string(1 + i) + std::string(" - " + m_buildQueue[i]->getName());
        BWAPI::Broodwar->drawTextScreen(30, 15 + 10*i, s.c_str());
        BWAPI::Broodwar->drawTextScreen(155, 15 + 10 * i, ("M : " + std::to_string(std::visit([](const auto& field) { return field.mineralPrice(); }, m_buildQueue[i]->getObject()))).c_str());
        BWAPI::Broodwar->drawTextScreen(195, 15 + 10 * i, ("G : " + std::to_string(std::visit([](const auto& field) { return field.mineralPrice(); }, m_buildQueue[i]->getObject()))).c_str());
        BWAPI::Broodwar->drawTextScreen(235, 15 + 10 * i, m_buildQueue[i]->toString().c_str());
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


void BuildQueue::computeNeed() {
    int profondeur = 5;
    float needCristal = 0;
    float needGas = 0;
    float alpha = 0.7f;
    int totalPriority = 0;
    for (int i = 0; i < std::min((size_t)(profondeur), m_buildQueue.size()); i++)
        totalPriority += m_buildQueue[i]->getPriority();

    for (int i = 0; i < std::min((size_t)(profondeur), m_buildQueue.size()); i++) {
        auto task = m_buildQueue[i];
        float factor = std::pow(alpha, (1.0f + m_buildQueue[0]->getPriority()) / (1.0f + m_buildQueue[i]->getPriority()) - 1.0f);
        needCristal += factor * std::visit([](const auto& field) { return field.mineralPrice(); }, task->getObject());
        needGas     += factor * std::visit([](const auto& field) { return field.gasPrice(); }, task->getObject());
    }

    m_manager->setRessourceAim(std::ceil(needCristal), std::ceil(needGas));
}
