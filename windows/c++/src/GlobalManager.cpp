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


	BaseManager base;
	bases.push_back(base);
	base.setBuildOrder(buildorder);
}

void GlobalManager::update() {
	for (int i = 0; i < bases.size(); i++) {
		bases[i].update();
	}
}




void GlobalManager::reserveGas(int gas_) {
	reservedGas += gas_;
}

void GlobalManager::reserveMinerals(int minerals_) {
	reservedMinerals += minerals_;
}

void GlobalManager::reserveRessources(int minerals_, int gas_) {
	reserveGas(gas_);
	reserveMinerals(minerals_);
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
