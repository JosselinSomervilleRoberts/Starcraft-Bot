#include "GlobalManager.h"

GlobalManager::GlobalManager() {
	const BWAPI::UnitType commandCenterType = BWAPI::Broodwar->self()->getRace().getCenter();
	const BWAPI::Unit commandCenter = Tools::GetUnitOfType(commandCenterType);
}

void GlobalManager::initiate() {
	using namespace BWAPI::UnitTypes;

	const auto workerType = BWAPI::Broodwar->self()->getRace().getWorker();
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
	const auto refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();


	const auto buildorder = {
		workerType,    workerType,          workerType,
		workerType,    workerType,          supplyProviderType, // @ 9/10 supply
		workerType,    workerType };
	

	BaseManager* base = new BaseManager(this);
	const auto upgrade_1 = BWAPI::UpgradeTypes::Protoss_Ground_Weapons;
	const auto upgrade_2 = BWAPI::UpgradeTypes::Protoss_Air_Weapons;

	bases.push_back(base); // ICI Ca fait une copie c'ets pas ouf
	using namespace BWAPI::UnitTypes;
	const auto forge = Protoss_Forge;
	const auto pylon = Protoss_Pylon;
	const auto gateway = Protoss_Gateway;
	const auto cybercore = Protoss_Cybernetics_Core;
	const auto cannon = Protoss_Photon_Cannon;
	const auto zealots = Protoss_Zealot;
	
	const auto roboticsType = Protoss_Dragoon;
	std::vector<std::variant<BWAPI::UnitType, BWAPI::UpgradeType, BWAPI::TechType>> build_vect = { gateway, forge, upgrade_1, zealots, cannon, zealots, zealots, zealots, zealots, cybercore, upgrade_2, cannon, roboticsType, roboticsType, roboticsType, zealots, roboticsType, zealots };
	std::vector<int> priority_vect = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0};
	bases[0]->initializeQueue(build_vect, priority_vect);
	
	
	//base.constructRefinery(300);


	//base.setBuildOrder(buildorder);
}

void GlobalManager::update() {
	for (int i = 0; i < bases.size(); i++) {
		bases[i]->update();
	}

	BWAPI::Broodwar->drawTextScreen(435 - 1, 55, "R: %d", reservedMinerals);
	BWAPI::Broodwar->drawTextScreen(503 - 1, 55, "R: %d", reservedGas);
}




bool GlobalManager::reserveGas(int gas_, bool& enoughGas) {
	if ((gas_ > 0) && !(enoughGas)) return false;
	if (getAvailableGas() >= gas_) {
		reservedGas += gas_;
		return true;
	}
	else {
		enoughGas = false;
	}
	return false;
}

bool GlobalManager::reserveMinerals(int minerals_, bool& enoughMinerals) {
	if ((minerals_ > 0) && !(enoughMinerals)) return false;
	if (getAvailableMinerals() >= minerals_) {
		reservedMinerals += minerals_;
		return true;
	}
	else {
		enoughMinerals = false;
	}
	return false;
}

bool GlobalManager::reserveRessources(int minerals_, int gas_, bool& enoughMinerals, bool& enoughGas) {
	bool copyEnoughGas = enoughGas;
	bool ok = reserveGas(gas_, enoughGas);
	if (ok) {
		bool ok2 = reserveMinerals(minerals_, enoughMinerals);
		if (!ok2) {
			releaseGas(gas_);
			enoughGas = copyEnoughGas;
		}
		return ok2;
	}
	return false;
}

void GlobalManager::releaseGas(int gas_) {
	reservedGas -= gas_;
}

void GlobalManager::releaseMinerals(int minerals_) {
	reservedMinerals -= minerals_;
}

void GlobalManager::releaseRessources(int minerals_, int gas_) {
	releaseGas(gas_);
	releaseMinerals(minerals_);
}

int GlobalManager::getAvailableGas() {
	return BWAPI::Broodwar->self()->gas() - reservedGas;
}

int GlobalManager::getAvailableMinerals() {
	return BWAPI::Broodwar->self()->minerals() - reservedMinerals;
}

BWAPI::Unit GlobalManager::acquireWorker() {
	// TODO :change for several bases
	return bases[0]->acquireWorker();
}





void GlobalManager::onUnitCreate(BWAPI::Unit unit) {
	// TODO :change for several bases
	std::cout << "Create " << unit->getType() << std::endl;
	bases[0]->unitCreated(unit);
}

void GlobalManager::onUnitDestroy(BWAPI::Unit unit) {
	// TODO :change for several bases
	bases[0]->unitDestroyed(unit);
}

void GlobalManager::onUnitComplete(BWAPI::Unit unit) {
	// TODO :change for several bases
	std::cout << "Complete " << unit->getType() << std::endl;
	bases[0]->unitCompleted(unit);
}


void GlobalManager::releaseWorker(BWAPI::Unit worker) {
	// TODO : clean that shit
	onUnitComplete(worker);
}


void  GlobalManager::setRessourceAim(int cristalAim, int gasAim) {
	// TODO :change for several bases
	bases[0]->setRessourceAim(cristalAim, gasAim);
}

void GlobalManager::setRefineryState(BuildingState state) {
	// TODO :change for several bases
	bases[0]->setRefineryState(state);
}