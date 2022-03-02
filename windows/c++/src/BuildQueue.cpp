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



void BuildQueue::addTask(std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> toBuild, int priority, TilePosition position, bool unique) {
    //std::cout << "Add Task : toBuild " << toBuild << " - " << m_buildQueue.size() << std::endl;
    BuildTask* buildtask;

    // First check if there is not a unique task that already exists with the same object
    int indexTask = -1;
    for (int i = 0; i < m_buildQueue.size(); i++) {
        auto taskObject = m_buildQueue[i]->getObject();
        if (taskObject == toBuild) { // Same unit or upgrade
            if (m_buildQueue[i]->unique || unique) {
                indexTask = i;
                i = m_buildQueue.size();
            }
        }
    }

    if (indexTask >= 0) {
        buildtask = m_buildQueue[indexTask];
        if (buildtask->getPriority() >= priority) return;

        buildtask->setPriority(priority);
        buildtask->unique = true;
        // TODO: update pos

        m_buildQueue.erase(m_buildQueue.begin() + indexTask);
    }
    else if (std::holds_alternative<BWAPI::UnitType>(toBuild)) {
        buildtask = new BuildTask(m_manager, std::get<BWAPI::UnitType>(toBuild), priority, position);
        buildtask->unique = unique;
    }
    else if (std::holds_alternative<BWAPI::UpgradeType>(toBuild)) {
        buildtask = new BuildTask(m_manager, std::get<BWAPI::UpgradeType>(toBuild), priority, position);
        buildtask->unique = unique;
    }
    else if (std::holds_alternative<BWAPI::TechType>(toBuild)) {
        buildtask = new BuildTask(m_manager, std::get<BWAPI::TechType>(toBuild), priority, position);
        buildtask->unique = unique;
    }
    else{
        std::cout << "Error BuildQueue task object not valid" << std::endl;
    }
    

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

    // Only recompute need if we did not add a supplyDepot
    if (std::holds_alternative<BWAPI::UpgradeType>(toBuild)) {
        if (std::get<BWAPI::UpgradeType>(toBuild) == BWAPI::Broodwar->self()->getRace().getSupplyProvider()) {
            computeNeed(true);
            return;
        }
    }
    else if(std::holds_alternative<BWAPI::TechType>(toBuild)) {
        if (std::get<BWAPI::TechType>(toBuild) == BWAPI::Broodwar->self()->getRace().getSupplyProvider()) {
            computeNeed(true);
            return;
        }
    }
    computeNeed();
}

BuildTask* BuildQueue::getTask(int i) {
    return m_buildQueue[i];
}


void BuildQueue::update() {
    computeNeed();

    bool enoughMinerals = true;
    bool enoughGas = true;
    for (int i = 0; i < m_buildQueue.size(); i++) {
        m_buildQueue[i]->update(enoughMinerals, enoughGas);
    }

    for (int i = 0; i < m_buildQueue.size(); i++) {
        if (m_buildQueue[i]->getState() == BuildTask::State::finalize) {
            /*if (std::holds_alternative<BWAPI::UnitType>(m_buildQueue[i]->getObject()) && std::get<BWAPI::UnitType>(m_buildQueue[i]->getObject()).isBuilding()) {
                // TODO: Ajouter à la liste des buildings qui servira aux upgrades
            }*/
            m_buildQueue.erase(m_buildQueue.begin() + i);
            i--;
        }
    }

    
    for (int i = 0; i < std::min((size_t)(5), m_buildQueue.size()); i++) {
        BWAPI::Broodwar->drawTextScreen(28, 15 + 10 * i, std::to_string(m_buildQueue[i]->getPriority()).c_str());
        BWAPI::Broodwar->drawTextScreen(40, 15 + 10*i, ("- " + m_buildQueue[i]->getName()).c_str());
        BWAPI::Broodwar->drawTextScreen(155, 15 + 10 * i, ("M : " + std::to_string(std::visit([](const auto& field) { return field.mineralPrice(); }, m_buildQueue[i]->getObject()))).c_str());
        BWAPI::Broodwar->drawTextScreen(195, 15 + 10 * i, ("G : " + std::to_string(std::visit([](const auto& field) { return field.gasPrice(); }, m_buildQueue[i]->getObject()))).c_str());
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


void BuildQueue::computeNeed(bool once) {
    int profondeur = 5;
    float needCristal = 0;
    float needGas = 0;
    float alpha = 0.7f;
    int totalPriority = 0;
    int totalSupply = BWAPI::Broodwar->self()->supplyTotal();
    bool addSupply = false;
    float supply = BWAPI::Broodwar->self()->supplyUsed();

    for (int i = 0; i < std::min((size_t)(profondeur), m_buildQueue.size()); i++)
        totalPriority += m_buildQueue[i]->getPriority();

    for (int i = 0; i < std::min((size_t)(profondeur), m_buildQueue.size()); i++) {
        auto task = m_buildQueue[i];
        float factor = std::pow(alpha, (1.0f + m_buildQueue[0]->getPriority()) / (1.0f + m_buildQueue[i]->getPriority()) - 1.0f);
        needCristal += factor * std::visit([](const auto& field) { return field.mineralPrice(); }, task->getObject());
        needGas     += factor * std::visit([](const auto& field) { return field.gasPrice(); }, task->getObject());
        
        if (std::holds_alternative<BWAPI::UnitType>(task->getObject())) {
            supply += factor * std::get<BWAPI::UnitType>(task->getObject()).supplyRequired();
            
            if ((supply >= totalSupply-1) && !(addSupply) && (std::get<BWAPI::UnitType>(task->getObject()).supplyProvided() == 0)) {
                BWAPI::UnitType supplyType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
                if(!once) addTask(supplyType, std::min(100, task->getPriority() + 1), BWAPI::Broodwar->self()->getStartLocation(), true);
                addSupply = true;
            }
        }
    }

    m_manager->setRessourceAim(std::ceil(needCristal), std::ceil(needGas));
}


int BuildQueue::getPriorityOfGas() {
    for (int i = 0; i < m_buildQueue.size(); i++) {
        auto buildTask = m_buildQueue[i];
        std::visit([](const auto& field) { std::cout << field << std::endl; }, buildTask->getObject());
        if (std::visit([](const auto& field) { return field.gasPrice(); }, buildTask->getObject()) > 0) {
            std::cout << buildTask->getName() << std::endl;
            return buildTask->getPriority();
        }
    }
    return 0;
}