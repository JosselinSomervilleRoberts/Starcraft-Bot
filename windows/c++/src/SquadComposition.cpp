#include "SquadComposition.h"

#define PRIORITY_ADDITIONAL_FOR_REQUIRED_BUILDING 2



SquadComposition::SquadComposition(BuildQueue* queue_) : queue(queue_) {
	using namespace BWAPI::UnitTypes;

	// Army 2
	Requirement req2 = { Protoss_Gateway, 2 };
	Requirement req3 = { BWAPI::UpgradeTypes::Singularity_Charge, 1 };
	requirements = { req2, req3};
	squadTypes = { Protoss_Observer, Protoss_Dragoon, Protoss_Zealot};
	squadProportions = { 0.1f, 0.5f, 0.4f };
}


void SquadComposition::updateRequirements() {
	for (int i = 0; i < requirements.size(); i++) {
		Requirement& req = requirements[i];
		int quantity = req.quantity;


		if (std::holds_alternative<BWAPI::UnitType>(req.toBuild)) { // It's a Unit 
			BWAPI::UnitType unitTypeToBuild = std::get<BWAPI::UnitType>(req.toBuild);
			req.quantity_current = Tools::CountUnitsOfType(unitTypeToBuild, BWAPI::Broodwar->self()->getUnits());
			quantity *= multiplier;
		}

		else if (std::holds_alternative<BWAPI::UpgradeType>(req.toBuild)) { // It's an Upgrade 
			BWAPI::UpgradeType upgradeTypeToBuild = std::get<BWAPI::UpgradeType>(req.toBuild);
			req.quantity_current = BWAPI::Broodwar->self()->getUpgradeLevel(upgradeTypeToBuild);
		}

		else if (std::holds_alternative<BWAPI::TechType>(req.toBuild)) { // It's a tech 
			BWAPI::TechType techTypeToBuild = std::get<BWAPI::TechType>(req.toBuild);
			if (BWAPI::Broodwar->self()->hasResearched(techTypeToBuild)) req.quantity_current = 1;
			else req.quantity_current = 0;
		}

		req.satisfied = (req.quantity_current >= quantity);
	}
}

bool SquadComposition::checkRequirements() {
	updateRequirements();
	for (int i = 0; i < requirements.size(); i++) {
		if (!(requirements[i].satisfied)) return false;
	}
	return true;
}

bool SquadComposition::checkMinimalRequirements() {
	updateRequirements();
	for (int i = 0; i < requirements.size(); i++) {
		if (std::holds_alternative<BWAPI::UnitType>(requirements[i].toBuild)) {
			if ((requirements[i].quantity_current < 1)) return false;
		}
	}
	return true;
}

void SquadComposition::fixMissingRequirements(int priority) {
	if (checkRequirements()) return; // Nothing to fix, everything works

	for (int i = 0; i < requirements.size(); i++) {
		Requirement req = requirements[i];

		if (!req.satisfied) {
			if (std::holds_alternative<BWAPI::UnitType>(req.toBuild)) { // It's a Unit 
				BWAPI::UnitType unitTypeToBuild = std::get<BWAPI::UnitType>(req.toBuild);

				// We add the Build to the queue
				int imax = req.quantity * multiplier - queue->countUnitTypeInTotal(unitTypeToBuild);
				for(int i=0; i<imax; i++)
					queue->addTask(unitTypeToBuild, priority, false);
			}

			else if (std::holds_alternative<BWAPI::UpgradeType>(req.toBuild)) { // It's an Upgrade 
				BWAPI::UpgradeType upgradeTypeToBuild = std::get<BWAPI::UpgradeType>(req.toBuild);
				BWAPI::UnitType upgradeBuilding = upgradeTypeToBuild.whatUpgrades();

				// We check if we have the building
				if (Tools::CountUnitsOfType(upgradeBuilding, BWAPI::Broodwar->self()->getUnits()) == 0)
					queue->addTask(upgradeBuilding, priority + PRIORITY_ADDITIONAL_FOR_REQUIRED_BUILDING, true);

				// We add the Upgrade to the queue
				queue->addTask(upgradeTypeToBuild, priority, true);
			}

			else if (std::holds_alternative<BWAPI::TechType>(req.toBuild)) { // It's a tech 
				BWAPI::TechType techTypeToBuild = std::get<BWAPI::TechType>(req.toBuild);
				BWAPI::UnitType researchBuilding = techTypeToBuild.whatResearches();

				// We check if we have the building
				if (Tools::CountUnitsOfType(researchBuilding, BWAPI::Broodwar->self()->getUnits()) == 0)
					queue->addTask(researchBuilding, priority + PRIORITY_ADDITIONAL_FOR_REQUIRED_BUILDING, true);

				// We add the Upgrade to the queue
				queue->addTask(techTypeToBuild, priority, true);
			}
		}
	}

	for (int i = 0; i < squadTypes.size(); i++) {
		queue->addTask(squadTypes[i], priority, true);
	}
}

bool SquadComposition::trainUnit(std::vector<BWAPI::Unit>& squad_units, int priority) {
	std::vector<float> proportions;
	int n = squad_units.size();
	proportions.resize(squadProportions.size());

	for (int j = 0; j < squadProportions.size(); j++)
		proportions[j] = 0;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < squadProportions.size(); j++) {
			if (squad_units[i]->getType() == squadProportions[j]) {
				j = squadProportions.size(); // To end loop
				proportions[j] += 1;
			}
		}
	}

	// Add what is in the build queue
	for (int j = 0; j < squadProportions.size(); j++) {
		int additional = queue->countUnitTypeInQueue(squadTypes[j]);
		n += additional;
		proportions[j] += additional;
	}

	// Compute the difference
	for (int j = 0; j < squadProportions.size(); j++)
		proportions[j] = proportions[j] / (float)(n) - squadProportions[j];

	// Get the min to choose th unitType
	// initialize original index locations
	std::vector<size_t> idx(proportions.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in diffDist
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values 
	stable_sort(idx.begin(), idx.end(),
		[&proportions](size_t i1, size_t i2) {return proportions[i1] < proportions[i2]; });

	// Add the chosen unit to the queue
	for (int j = 0; j < squadProportions.size(); j++) {
		BWAPI::UnitType unitType = squadTypes[idx[j]];
		auto requirement = unitType.whatBuilds();
		if (Tools::CountUnitsOfType(requirement.first, BWAPI::Broodwar->self()->getUnits()) >= requirement.second) { // Can build
			queue->addTask(unitType, priority);
			return true;
		}
	}

	return false;
}

void SquadComposition::scaleUp() {
	multiplier += 1;
}

void SquadComposition::scaleDown() {
	multiplier = std::max(1, multiplier - 1);
}