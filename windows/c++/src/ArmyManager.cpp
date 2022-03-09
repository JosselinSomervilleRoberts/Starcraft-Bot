#include "ArmyManager.h"
#include "BaseManager.h"
#include "GlobalManager.h"

#define UNITS_IN_QUEUE_MAX 5



ArmyManager::ArmyManager(BaseManager* base_) : base(base_), compo(base_->getQueue()) {
}




void ArmyManager::addSoldier(BWAPI::Unit soldier) {
	if (!(std::find(soldiers.begin(), soldiers.end(), soldier) != soldiers.end())) {
		soldiers.push_back(soldier);
		nbSoldiersTotal += 1;
		nbUnitsInQueue--;
		this->computeRepartition();
	}
}


void ArmyManager::checkRepartition() {
	/*for (auto soldier : soldiers) {
		if (soldier->isAttacking() || soldier->isPatrolling()) {

			auto indexSoldier = std::find(soldiersAvailable.begin(), soldiersAvailable.end(), soldier);
			if (indexSoldier != soldiersAvailable.end()) 
				soldiersAvailable.erase(indexSoldier);

		}
	}*/
}

void ArmyManager::computeRepartition() {
	/* TODO: compute repartition between patrol, attack, defense, first all in defense and few units in patrol then attackk*/
	soldiers.clear();
	auto workerType = BWAPI::Broodwar->self()->getRace().getWorker();
	for (auto unit : BWAPI::Broodwar->self()->getUnits()) {
		if (unit->canAttack() || unit->getType() == BWAPI::UnitTypes::Protoss_Observer) {
			if (unit->getType() != workerType) {
				soldiers.push_back(unit);
			}
		}
	}

	defenseSoldiers.clear();
	patrolSoldiers.clear();
	if (mode == Mode::defense) {
		attackSoldiers.clear();
		for (auto soldier : soldiers) {
			defenseSoldiers.push_back(soldier);
		}
			
	}
	else if (mode == Mode::attack) {
		attackSoldiers.clear();
		for (auto soldier : soldiers) {
			if (soldier->getType() == BWAPI::UnitTypes::Protoss_Observer && std::rand() <= RAND_MAX/2 )
				defenseSoldiers.push_back(soldier);
			else
				attackSoldiers.push_back(soldier);
		}
	}
	else if (mode == Mode::normal) {
		for (auto soldier : attackSoldiers) { // Delete from list units that died
			if(std::find(soldiers.begin(), soldiers.end(), soldier) == soldiers.end())
				attackSoldiers.erase(std::remove(attackSoldiers.begin(), attackSoldiers.end(), soldier), attackSoldiers.end());
		}
		for (int i = 0; i < (soldiers.size()); i++) {
			//We go through the soldiers list
			if (std::find(attackSoldiers.begin(), attackSoldiers.end(), soldiers[i]) == attackSoldiers.end()) {
				// If the soldier is already attacking, we let it in attack, else, we compute the repartition
				if ((float)attackSoldiers.size() / (float)soldiers.size() < 0.45 && (soldiers[i]->getType() != BWAPI::UnitTypes::Protoss_Observer || std::rand() <= RAND_MAX / 2)) {
					attackSoldiers.push_back(soldiers[i]);
				}
				else if ((float)patrolSoldiers.size() / (float)soldiers.size() < 0.15 && soldiers[i]->getType() != BWAPI::UnitTypes::Protoss_Observer)
					patrolSoldiers.push_back(soldiers[i]);
				else
					defenseSoldiers.push_back(soldiers[i]);
			}
		}
	}
}


void ArmyManager::update() {
	compo.fixMissingRequirements(15);
	if (base->getManager()->getAvailableMinerals() > 150) {
		if (nbUnitsInQueue < UNITS_IN_QUEUE_MAX * compo.getMultiplier())
		{
			bool trainSolider = compo.trainUnit(soldiers);
			if (trainSolider) nbUnitsInQueue++;
		}
	}
	if (base->getManager()->getAvailableMinerals() > 400 *pow(1.1, compo.getMultiplier() - 1)) {
		compo.scaleUp();
	}
	checkRepartition();
	computeRepartition();
	// We check if we need to change the repartition
	BWAPI::Position enemyPos;
	if (mode == Mode::attack && attackSoldiers.size()>=20) {
		if(BWAPI::Broodwar->self()->getStartLocation() != BWAPI::TilePosition(31, 7))
			enemyPos = (BWAPI::Position)BWAPI::TilePosition(31, 7); // Conversion of Tile Position to position  BWAPI::Broodwar->enemy()->getStartLocation()
		else
			enemyPos = (BWAPI::Position)BWAPI::TilePosition(64, 118);
		//2nd base 70,13 top,  25,105?
		attack(attackSoldiers, enemyPos);
		mode = Mode::normal;
	}
	
	else if (mode == Mode::normal)
	{
		if (BWAPI::Broodwar->self()->getStartLocation() != BWAPI::TilePosition(31, 7)){
			enemyPos = (BWAPI::Position)BWAPI::TilePosition(31, 7); // Conversion of Tile Position to position  BWAPI::Broodwar->enemy()->getStartLocation()
		}
		else{
			enemyPos = (BWAPI::Position)BWAPI::TilePosition(64, 118);
		}
		//ATTACK
		if (attackSoldiers.size() > 15) {
			attack(attackSoldiers, enemyPos);
			for (auto soldier : attackSoldiers) {
				for (auto enemy : BWAPI::Broodwar->enemy()->getUnits()) {
					if (enemy->getPosition().getDistance(soldier->getPosition()) < 100) {
						attack(attackSoldiers, enemy);
					}
				}
			}
		}

		//PATROL
		
	}
	if (mode == Mode::defense && defenseSoldiers.size() >= 15 && state != Mode::attack)
		mode = Mode::normal;


	if (state == Mode::attack) {
		//attack(defenseSoldiers, (BWAPI::Position)BWAPI::Broodwar->self()->getStartLocation());
	}
	

	if (patrolSoldiers.size() > 0) {
		for (auto soldier : patrolSoldiers)
			soldier->patrol((BWAPI::Position)BWAPI::TilePosition(30, 55));
	}
	//else if (mode == Mode::normal) //TODO: envoie l'attaque, envoie la patrol, 





	// Display soldiers organisation
	if(mode==Mode::attack)
		BWAPI::Broodwar->drawTextScreen(350, 15, "Att.", soldiers.size());
	else if (mode == Mode::defense)
		BWAPI::Broodwar->drawTextScreen(350, 15, "Def.", soldiers.size());
	else
		BWAPI::Broodwar->drawTextScreen(350, 15, "Norm", soldiers.size());
	BWAPI::Broodwar->drawTextScreen(350, 25, "Tot: %d", soldiers.size());
	BWAPI::Broodwar->drawTextScreen(350, 35, "Def: %d", defenseSoldiers.size());
	BWAPI::Broodwar->drawTextScreen(350, 45, "Att: %d", attackSoldiers.size());
	BWAPI::Broodwar->drawTextScreen(350, 55, "Ptr: %d", patrolSoldiers.size());
}


void ArmyManager::findAvailableSoldiers(int nbWanted) {
	/*int nbFound = soldiersAvailable.size();


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
	}*/
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
	if (defenseSoldiers.size() > 0) {
		BWAPI::Unit soldier = defenseSoldiers[0];
		auto indexSoldier = std::find(soldiers.begin(), soldiers.end(), soldier);
		if (indexSoldier != soldiers.end()) soldiers.erase(indexSoldier);

		indexSoldier = std::find(defenseSoldiers.begin(), defenseSoldiers.end(), soldier);
		if (indexSoldier != defenseSoldiers.end()) defenseSoldiers.erase(indexSoldier);
		nbSoldiersTotal--;
		return soldier;
	}

	return nullptr;
}

void ArmyManager::onAttack(BWAPI::Unit threat) {
	ennemies.push_back(threat);
	auto soldier = getAvailableSoldier();
	state = Mode::attack; // we are under attack
	mode = Mode::defense;
	if(defenseSoldiers.size() > 0)
		attack(defenseSoldiers, threat);
}

void ArmyManager::noAttack() {
	if (state == Mode::attack) { // If we were under attack, then we won, let's go after them!
		mode = Mode::attack;
		state = Mode::normal;
	}
}
void ArmyManager::attack(BWAPI::Unit soldier, BWAPI::Unit threat) {
	soldier->attack(threat->getPosition());
}
void ArmyManager::attack(std::vector<BWAPI::Unit> soldiers, BWAPI::Unit threat) {
	for(auto soldier : soldiers)
		soldier->attack(threat->getPosition());
}
void ArmyManager::attack(std::vector<BWAPI::Unit> soldiers, BWAPI::Position position) {
	for (auto soldier : soldiers)
		soldier->attack(position);

}
void ArmyManager::onUnitDestroyed(BWAPI::Unit unit) {
	//soldiers.erase(std::remove(soldiers.begin(), soldiers.end(), unit), soldiers.end());;
	/*std::remove(attackSoldiers.begin(), attackSoldiers.end(), unit);
	std::remove(defenseSoldiers.begin(), defenseSoldiers.end(), unit);
	std::remove(patrolSoldiers.begin(), patrolSoldiers.end(), unit);*/

	/* TODO: When unit dies, we remove it, then compute the next best unit to create instead, to replace it, and then add it to the BuildQueue*/
}
void patrol(BWAPI::Position position, std::vector<BWAPI::Unit> soldiers) {
	if (soldiers.size() > 0) {
		for (auto soldier : soldiers)
			soldier->patrol(position);
	}
}