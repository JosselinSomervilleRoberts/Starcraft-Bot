#include "WorkerManager.h"
#include "BaseManager.h"


// This is based on https://liquipedia.net/starcraft/Mining

#define CRISTAL_NEED_MIN 50
#define CRISTAL_PER_MINUTE 68.1
#define GAS_PER_MINUTE 103.0
#define TIME_MAX_MINUTE 0.4
#define NB_WORKERS_MAX_PER_BASE 16
#define NB_WORKERS_MAX_PER_GAS 3



WorkerManager::WorkerManager(BaseManager* base_): base(base_) {
	this->computeNeed();
}

void WorkerManager::computeNeed() {
	gasNeed     = std::max(0, gasAim - BWAPI::Broodwar->self()->gas());
	cristalNeed = std::max(CRISTAL_NEED_MIN, cristalAim - BWAPI::Broodwar->self()->minerals());
}


void WorkerManager::addWorker(BWAPI::Unit worker) {
	if (!(std::find(workers.begin(), workers.end(), worker) != workers.end())) {
		workers.push_back(worker);
		workersAvailable.push_back(worker);
		nbWorkersTotal += 1;
		this->computeRepartition();
	}
}


void WorkerManager::computeRepartition() {
	float timeForCristalPerWorker = cristalNeed / CRISTAL_PER_MINUTE;
	float timeForGasPerWorker     = gasNeed / GAS_PER_MINUTE;
	nbWorkersGasWanted     = std::min(NB_WORKERS_MAX_PER_GAS, (int)(std::ceil(nbWorkersTotal * timeForGasPerWorker / (timeForCristalPerWorker + timeForGasPerWorker))));
	nbWorkersCristalWanted = nbWorkersTotal - nbWorkersGasWanted;

	if ((nbWorkersGasWanted > 0) && (!hasRefinery)) {
		// Need refinery
		nbWorkersCristalWanted = nbWorkersTotal;
		base->constructRefinery(nbWorkersGasWanted);
	}

	float timeForCristal = timeForCristalPerWorker / nbWorkersCristalWanted;
	float timeForGas     = timeForGasPerWorker     / nbWorkersGasWanted;

	if ((timeForCristal > TIME_MAX_MINUTE) || (timeForGas > TIME_MAX_MINUTE)) {
		// Need more workers

		if ((timeForGas > TIME_MAX_MINUTE) && (nbWorkersGasWanted == NB_WORKERS_MAX_PER_GAS)) {
			// Need to expand for gas
			base->transmit_expansion();
		}
		else if (nbWorkersTotal >= NB_WORKERS_MAX_PER_BASE) {
			// Need to expand for more workers
			base->transmit_expansion();
		}
		else {
			// We can ask for a new worker in this base
			base->newWorker();
		}
	}
}


void WorkerManager::update() {
	// We check if we need to change the repartition
	int nbWanted = std::max(0, (int)(nbWorkersGasWanted - workersGas.size())) + std::max(0, (int)(nbWorkersCristalWanted - workersCristal.size()));
	if (nbWanted > 0) {
		this->findAvailableWorkers(nbWanted);
		int nbAvailable = workersAvailable.size();

		if (nbAvailable > 0) {
			float diffGas     = 1. - workersGas.size()     / (float)(nbWorkersGasWanted);
			float diffCristal = 1. - workersCristal.size() / (float)(nbWorkersCristalWanted);

			int forGas = (int)(round(nbAvailable * diffGas / (diffGas + diffCristal)));
			int forCristal = nbAvailable - forGas;

			std::vector<float> diffDist;
			for (auto unit : workersAvailable) {
				BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());
				BWAPI::Unit closestGeyser  = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getGeysers());

				float distanceCristal = unit->getDistance(closestMineral);
				float distanceGas     = unit->getDistance(closestGeyser);
				diffDist.push_back(distanceGas - distanceCristal);
			}

			// initialize original index locations
			std::vector<size_t> idx(diffDist.size());
			iota(idx.begin(), idx.end(), 0);

			// sort indexes based on comparing values in diffDist
			// using std::stable_sort instead of std::sort
			// to avoid unnecessary index re-orderings
			// when v contains elements of equal values 
			stable_sort(idx.begin(), idx.end(),
				[&diffDist](size_t i1, size_t i2) {return diffDist[i1] < diffDist[i2]; });

			for (int i = 0; i < nbAvailable; i++) {
				if (i < forGas)
					this->setGasWorker(workersAvailable[idx[i]]);
				else
					this->setCristalWorker(workersAvailable[idx[i]]);
			}

			workersAvailable.clear();
		}
	}


	// Display available resources
	BWAPI::Broodwar->drawTextScreen(435, 15, "A: %d", cristalAim);
	BWAPI::Broodwar->drawTextScreen(503, 15, "A: %d", gasAim);
	BWAPI::Broodwar->drawTextScreen(435+1, 25, "N: %d", cristalNeed);
	BWAPI::Broodwar->drawTextScreen(503+1, 25, "N: %d", gasNeed);
	BWAPI::Broodwar->drawTextScreen(435-3, 35, "W: %d", workersCristal.size());
	BWAPI::Broodwar->drawTextScreen(503-3, 35, "W: %d", workersGas.size());
	BWAPI::Broodwar->drawTextScreen(435 - 1, 45, "D: %d", nbWorkersCristalWanted);
	BWAPI::Broodwar->drawTextScreen(503 - 1, 45, "D: %d", nbWorkersGasWanted);


	BWAPI::Broodwar->drawTextScreen(350, 15, "Tot: %d", workers.size());
	BWAPI::Broodwar->drawTextScreen(350, 25, "Ava: %d", workersAvailable.size());
}


void WorkerManager::findAvailableWorkers(int nbWanted) {
	int nbFound = 0;
	std::cout << "find worker" << std::endl;

	for (auto worker : workers) {
		if (nbFound >= nbWanted)
			return;

		// If not already available
		if (!(std::find(workersAvailable.begin(), workersAvailable.end(), worker) != workersAvailable.end())) {
			if (!(worker->isCarryingGas() || worker->isCarryingMinerals() || worker->isConstructing() || worker->isGatheringGas() || worker->isGatheringMinerals())) {
				workersAvailable.push_back(worker);
				std::remove(workersCristal.begin(), workersCristal.end(), worker);
				std::remove(workersGas.begin(), workersGas.end(), worker);
				nbFound++;
			}
		}
	}
}



BWAPI::Unit WorkerManager::getAvailableWorker() {
	if (workersAvailable.size() > 0) {
		BWAPI::Unit worker = workersAvailable[0];
		std::remove(workers.begin(), workersCristal.end(), worker);
		std::remove(workersGas.begin(), workersGas.end(), worker);
		std::remove(workersCristal.begin(), workersCristal.end(), worker);
		std::remove(workersAvailable.begin(), workersGas.end(), worker);
		nbWorkersTotal--;
		return worker;
	}

	return nullptr;
}


void WorkerManager::setCristalWorker(BWAPI::Unit worker) {
	std::cout << "add cristal worker " << workersCristal.size() << " - " << worker << std::endl;
	workersCristal.push_back(worker);
	BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(worker, BWAPI::Broodwar->getMinerals());

	// If a valid mineral was found, right click it with the unit in order to start harvesting
	if (closestMineral) { worker->rightClick(closestMineral); }
}



void WorkerManager::setGasWorker(BWAPI::Unit worker) {
	workersGas.push_back(worker);
	BWAPI::Unit closestGeyser = Tools::GetClosestUnitTo(worker, BWAPI::Broodwar->getGeysers());

	// If a valid mineral was found, right click it with the unit in order to start harvesting
	if (closestGeyser) { worker->rightClick(closestGeyser); }
}