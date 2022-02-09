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


	BaseManager base(this);
	bases.push_back(base);
	//base.setBuildOrder(buildorder);
}

void GlobalManager::update() {
	workerManager.update();
	for (int i = 0; i < bases.size(); i++) {
		bases[i].update();
		std::cout << "StarterBot On GlobalManager update\n";


	}
}




bool GlobalManager::reserveGas(int gas_) {
	if (getAvailableGas() >= gas_) {
		reservedGas += gas_;
		return true;
	}
	return false;
}

bool GlobalManager::reserveMinerals(int minerals_) {
	if (getAvailableMinerals() >= minerals_) {
		reservedMinerals += minerals_;
		return true;
	}
	return false;
}

bool GlobalManager::reserveRessources(int minerals_, int gas_) {
	bool ok = reserveGas(gas_);
	if (ok) {
		bool ok2 = reserveMinerals(minerals_);
		if (!ok2) releaseGas(gas_);
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
	return bases[0].acquireWorker();
}





void GlobalManager::onUnitCreate(BWAPI::Unit unit) {
	// TODO :change for several bases
	bases[0].unitCreated(unit);
}

void GlobalManager::onUnitDestroy(BWAPI::Unit unit) {
	// TODO :change for several bases
	bases[0].unitDestroyed(unit);
}

void GlobalManager::onUnitComplete(BWAPI::Unit unit) {
	// TODO :change for several bases
	bases[0].unitCompleted(unit);
}


void GlobalManager::releaseWorker(BWAPI::Unit worker) {
	// TODO : clean that shit
	onUnitComplete(worker);
}