#include "SquadComposition.h"

#define PRIORITY_ADDITIONAL_FOR_REQUIRED_BUILDING 2



SquadComposition::SquadComposition(BuildQueue* queue_) : queue(queue_) {
	using namespace BWAPI::UnitTypes;

	Requirement req1 = { Protoss_Gateway, 2 };
	Requirement req2 = { Protoss_Cybernetics_Core, 1 };
	Requirement req3 = { BWAPI::UpgradeTypes::Singularity_Charge, 1 };
	requirements = { req1, req2, req3};
	squadTypes = { Protoss_Dragoon, Protoss_Observer };
	squadProportions = { 0.6f, 0.4f };
}


void SquadComposition::updateRequirements() {
	for (int i = 0; i < requirements.size(); i++) {
		Requirement& req = requirements[i];


		if (std::holds_alternative<BWAPI::UnitType>(req.toBuild)) { // It's a Unit 
			BWAPI::UnitType unitTypeToBuild = std::get<BWAPI::UnitType>(req.toBuild);
			req.quantity_current = Tools::CountUnitsOfType(unitTypeToBuild, BWAPI::Broodwar->self()->getUnits());
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

		req.satisfied = (req.quantity_current >= req.quantity);
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
		if ((requirements[i].quantity_current < 1)) return false;
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
				int imax = req.quantity - queue->countUnitTypeInTotal(unitTypeToBuild);
				for(int i=0; i<imax; i++)
					queue->addTask(unitTypeToBuild, priority, true);
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
				std::cout << "ADD TECH" << std::endl;
			}
		}
	}
}