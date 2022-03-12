#include "BuildQueue.h"
#include "GlobalManager.h"


#include <cassert>
#include <stdexcept>
#include <type_traits>

#define ADDITIONAL_PRIORITY_REQUIREMENTS 2



using namespace BWAPI;

BuildQueue::BuildQueue(GlobalManager* manager, int index_base_) : index_base(index_base_), m_manager(manager) {
    std::cout << "Constructor buildQueue" << std::endl;
    if (BWAPI::Broodwar->self()->getStartLocation() != BWAPI::TilePosition(31, 7)) {
        pylonPos = { BWAPI::TilePosition(49,109),BWAPI::TilePosition(30, 103) };
        cannonPos = { BWAPI::TilePosition(47,106), BWAPI::TilePosition(45,107),BWAPI::TilePosition(33, 102), BWAPI::TilePosition(31, 101) };
    }
    else {
        pylonPos = { BWAPI::TilePosition(47,20), BWAPI::TilePosition(64, 24) };
        cannonPos = { BWAPI::TilePosition(49,19),  BWAPI::TilePosition(48,17), BWAPI::TilePosition(60, 25), BWAPI::TilePosition(62, 25) };
    }
        
    
}


void BuildQueue::addTask(BWAPI::UnitType toBuild, int priority , bool unique) {
    addTask(toBuild, priority, BWAPI::Broodwar->self()->getStartLocation(), unique);
}

void BuildQueue::addTask(BWAPI::UpgradeType toBuild, int priority, bool unique) {
    addTask(toBuild, priority, BWAPI::Broodwar->self()->getStartLocation(), unique);
}

void BuildQueue::addTask(BWAPI::TechType toBuild, int priority, bool unique) {
    addTask(toBuild, priority, BWAPI::Broodwar->self()->getStartLocation(), unique);
}

void BuildQueue::addTask(std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType> toBuild, int priority, TilePosition position, bool unique, bool exactPos) {
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
    //TODO: when we add an unit, we check its requiredTech and requiredUnits and then check if they are already done and finally add them to queue (as unique)
    if (indexTask >= 0) {
        buildtask = m_buildQueue[indexTask];
        if (buildtask->getPriority() >= priority) return;

        buildtask->setPriority(priority);
        buildtask->unique = true;
        // TODO: update pos

        m_buildQueue.erase(m_buildQueue.begin() + indexTask);
    }
    else if (std::holds_alternative<BWAPI::UnitType>(toBuild)) {
        /*for (auto unit : std::get<BWAPI::UnitType>(toBuild).requiredUnits()) {
            std::cout << "UNIT REQUIRED : " << unit.first << std::endl;
        }*/
        if (std::get<BWAPI::UnitType>(toBuild).getType(std::get<BWAPI::UnitType>(toBuild).getName()) == BWAPI::UnitTypes::Protoss_Pylon && pylonPos.size() > 0) {
            position = pylonPos[0];
            exactPos = true;
            auto index = std::find(pylonPos.begin(), pylonPos.end(), position);
            if (index != pylonPos.end()) pylonPos.erase(index);
        }
        else if (std::get<BWAPI::UnitType>(toBuild).getType(std::get<BWAPI::UnitType>(toBuild).getName()) == BWAPI::UnitTypes::Protoss_Photon_Cannon && cannonPos.size() > 0) {
           
            position = cannonPos[0];
            exactPos = true;
            auto index = std::find(cannonPos.begin(), cannonPos.end(), cannonPos[0]);
            if (index != cannonPos.end()) cannonPos.erase(index);
        }

        buildtask = new BuildTask(m_manager, std::get<BWAPI::UnitType>(toBuild), priority, position, exactPos);
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


    // Check previous requirements
    
    auto object = buildtask->getObject();

    if (std::holds_alternative<BWAPI::UnitType>(object)) {
        BWAPI::UnitType unitType = std::get<BWAPI::UnitType>(object);
        if (true) { //unitType.isBuilding()) {
            BWAPI::TechType techType = unitType.requiredTech();
            if ((techType != BWAPI::TechTypes::None) && !(BWAPI::Broodwar->self()->hasResearched(techType))) {
                addTask(techType, priority + ADDITIONAL_PRIORITY_REQUIREMENTS, true);
            }
            auto mapRequirements = unitType.requiredUnits();
            std::map<BWAPI::UnitType, int>::iterator it;

            for (it = mapRequirements.begin(); it != mapRequirements.end(); it++)
            {
                int count = countUnitTypeInTotal(it->first);
                if(count < it->second) addTask(it->first, priority + ADDITIONAL_PRIORITY_REQUIREMENTS, true);
            }
        }
        if (!(unitType.isBuilding() )) {
            auto requirement = unitType.whatBuilds();
            if (Tools::CountUnitsOfType(requirement.first, BWAPI::Broodwar->self()->getUnits()) < requirement.second) 
                addTask(requirement.first, priority + ADDITIONAL_PRIORITY_REQUIREMENTS, true);
        }
    }
    
    else if (std::holds_alternative<BWAPI::UpgradeType>(object)) {
        BWAPI::UpgradeType upgradeType = std::get<BWAPI::UpgradeType>(object);
        auto building = upgradeType.whatUpgrades();
        if (Tools::CountUnitsOfType(building, BWAPI::Broodwar->self()->getUnits()) < 1)
            addTask(building, priority + ADDITIONAL_PRIORITY_REQUIREMENTS, true);
    }
    
    else if (std::holds_alternative<BWAPI::TechType>(object)) {
        BWAPI::TechType techType = std::get<BWAPI::TechType>(object);
        auto building = techType.whatResearches();
        if (true)//Tools::CountUnitsOfType(building, BWAPI::Broodwar->self()->getUnits()) < 1)
            addTask(building, priority + ADDITIONAL_PRIORITY_REQUIREMENTS, true);
    }


    // Only recompute need if we did not add a supplyDepot
    if (std::holds_alternative<BWAPI::UpgradeType>(toBuild)) {
        if (std::get<BWAPI::UpgradeType>(toBuild) == BWAPI::Broodwar->self()->getRace().getSupplyProvider()) {
            computeNeed(true);
            return;
        }
    }
    else if (std::holds_alternative<BWAPI::TechType>(toBuild)) {
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


/*
 * Erase First Occurrence of given  substring from main string.
 */
void eraseSubStr(std::string& mainStr, const std::string& toErase)
{
    // Search for the substring in string
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
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
        std::string name = m_buildQueue[i]->getName();
        eraseSubStr(name, "Protoss_");
        int addY = 0;
        if (index_base > 0) addY = 100;
        BWAPI::Broodwar->drawTextScreen(28, addY + 15 + 10 * i, std::to_string(m_buildQueue[i]->getPriority()).c_str());
        BWAPI::Broodwar->drawTextScreen(40,  addY+15 + 10*i, ("- " + name).c_str());
        BWAPI::Broodwar->drawTextScreen(155, addY+15 + 10 * i, ("M : " + std::to_string(std::visit([](const auto& field) { return field.mineralPrice(); }, m_buildQueue[i]->getObject()))).c_str());
        BWAPI::Broodwar->drawTextScreen(195, addY+15 + 10 * i, ("G : " + std::to_string(std::visit([](const auto& field) { return field.gasPrice(); }, m_buildQueue[i]->getObject()))).c_str());
        BWAPI::Broodwar->drawTextScreen(235, addY+15 + 10 * i, m_buildQueue[i]->toString().c_str());
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



void BuildQueue::unitCreated(BWAPI::Unit unit) {
    for (int i = 0; i < m_buildQueue.size(); i++) {
        m_buildQueue[i]->onUnitCreated(unit);
    }
}

void BuildQueue::unitCompleted(BWAPI::Unit unit) {
    for (int i = 0; i < m_buildQueue.size(); i++) {
        m_buildQueue[i]->onUnitCompleted(unit);
    }
}

int BuildQueue::countUnitTypeInQueue(BWAPI::UnitType type) {
    int count = 0;
    for (int i = 0; i < m_buildQueue.size(); i++) {
        auto object = m_buildQueue[i]->getObject();
        if (std::holds_alternative<BWAPI::UnitType>(object)) {
            if (std::get<BWAPI::UnitType>(object) == type) count++;
        }
    }
    return count;
}

int BuildQueue::countUnitTypeInTotal(BWAPI::UnitType type) {
    int count = countUnitTypeInQueue(type);
    count += Tools::CountUnitsOfType(type, BWAPI::Broodwar->self()->getUnits());
    return count;
}


void BuildQueue::computeNeed(bool once) {
    int profondeur = 5;
    float needCristal = 0;
    float needGas = 0;
    float alpha = 0.7f;
    int totalPriority = 0;
    BWAPI::UnitType supplyType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
    int totalSupply = BWAPI::Broodwar->self()->supplyTotal() + countUnitTypeInQueue(supplyType) * supplyType.supplyProvided();
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
            
            int marge = std::min(12, std::max(3, (int)(0.12f * totalSupply))); // Nb of supply available proportionnal to number of supply
            if ((supply >= totalSupply - marge) && !(addSupply) && (std::get<BWAPI::UnitType>(task->getObject()).supplyProvided() == 0)) {
                if(!once) addTask(supplyType, std::min(100, task->getPriority() + ADDITIONAL_PRIORITY_REQUIREMENTS), BWAPI::Broodwar->self()->getStartLocation(), false);
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

void BuildQueue::unitDestroyed(BWAPI::Unit unit) {
    for (auto task : m_buildQueue) {
        task->restart(unit);
    }
}