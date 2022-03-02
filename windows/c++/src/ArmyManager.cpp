#include "ArmyManager.h"
#include "BaseManager.h"





ArmyManager::ArmyManager(BaseManager* base_) : base(base_) {
}




void ArmyManager::addSoldier(BWAPI::Unit soldier) {
	if (!(std::find(soldiers.begin(), soldiers.end(), soldier) != soldiers.end())) {
		soldiers.push_back(soldier);
		soldiersAvailable.push_back(soldier);
		nbSoldiersTotal += 1;
		this->computeRepartition();
	}
}


void ArmyManager::checkRepartition() {
	for (auto soldier : soldiers) {
		if (soldier->isAttacking() || soldier->isPatrolling()) {

			auto indexSoldier = std::find(soldiersAvailable.begin(), soldiersAvailable.end(), soldier);
			if (indexSoldier != soldiersAvailable.end()) soldiersAvailable.erase(indexSoldier);

		}
	}
}

void ArmyManager::computeRepartition() {
	/* TODO: compute repartition between patrol, attack, defense, first all in defense and few units in patrol then attackk*/
}


void ArmyManager::update() {
	checkRepartition();

	// We check if we need to change the repartition
	

	// Display available resources
	
	BWAPI::Broodwar->drawTextScreen(350, 45, "Tot: %d", soldiers.size());
	BWAPI::Broodwar->drawTextScreen(350, 55, "Ava: %d", soldiersAvailable.size());
	BWAPI::Broodwar->drawTextScreen(350, 65, "Tot: %d", nbSoldiersTotal);
}


void ArmyManager::findAvailableSoldiers(int nbWanted) {
	int nbFound = soldiersAvailable.size();


	int i = 0;
	while ((i < soldiers.size()) && (nbFound < nbWanted)) {
		BWAPI::Unit soldier = soldiers[i];
		if (nbFound >= nbWanted)
			return;

		// If not already available
		if (std::find(soldiersAvailable.begin(), soldiersAvailable.end(), soldier) == soldiersAvailable.end()) {
			if (!(soldier->isAttacking() || soldier->isPatrolling())) { // TODO: check if gathering
				soldiersAvailable.push_back(soldier);
				nbFound++;
			}
		}
		i++;
	}
	/*
	int i = 0;
	while((i < workers.size()) && (nbFound < nbWanted)) {
		BWAPI::Unit worker = workers[i];
		if (std::find(workersAvailable.begin(), workersAvailable.end(), worker) == workersAvailable.end()) {
			workersAvailable.push_back(worker);
			std::remove(workersCristal.begin(), workersCristal.end(), worker);
			std::remove(workersGas.begin(), workersGas.end(), worker);
			nbFound++;
		}
		i++;
	}*/

}



BWAPI::Unit ArmyManager::getAvailableSoldier() {
	std::cout << "hola 0" << std::endl;
	if (soldiersAvailable.size() > 0) {
		BWAPI::Unit soldier = soldiersAvailable[0];
		std::cout << "hola 1" << std::endl;
		auto indexSoldier = std::find(soldiers.begin(), soldiers.end(), soldier);
		if (indexSoldier != soldiers.end()) soldiers.erase(indexSoldier);

		indexSoldier = std::find(soldiersAvailable.begin(), soldiersAvailable.end(), soldier);
		if (indexSoldier != soldiersAvailable.end()) soldiersAvailable.erase(indexSoldier);
		nbSoldiersTotal--;
		return soldier;
	}
	std::cout << "hola 6" << std::endl;

	return nullptr;
}

void ArmyManager::onAttack(BWAPI::Unit threat) {
	ennemies.push_back(threat);
	auto soldier = getAvailableSoldier();
	if(soldier)
		attack(soldier, threat);

}
void ArmyManager::attack(BWAPI::Unit soldier, BWAPI::Unit threat) {
	soldier->attack(threat->getPosition());
}
void ArmyManager::onUnitDestroyed(BWAPI::Unit unit) {
	std::remove(soldiers.begin(), soldiers.end(), unit);
	std::remove(soldiersAvailable.begin(), soldiersAvailable.end(), unit);
	nbSoldiersTotal--;
}