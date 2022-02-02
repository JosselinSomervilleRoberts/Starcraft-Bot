#include "BuildOrderStep.h"
#include "../Tools.h"


BuildOrderStep::BuildOrderStep() {}

bool BuildOrderStep::isTrigerred() {

	if (this->trigger.type == NB_UNITS) {
		if (BWAPI::Broodwar->self()->supplyUsed() >= 2*this->trigger.value) return true;
	}

	return false;
}


bool BuildOrderStep::doAction() {
	if (this->isTrigerred()) {

		if (this->action.type == BUILD) {

			if (BWAPI::Broodwar->self()->gas() >= this->action.unit.gasPrice() * this->action.count
				&& BWAPI::Broodwar->self()->minerals() >= this->action.unit.mineralPrice() * this->action.count) {
				BWAPI::Broodwar->printf(("Building " + std::to_string(this->action.count) + " " + this->action.unit.getName()).c_str());

				bool startedBuilding;
				for(int i = 0; i < this->action.count; i++) {
					startedBuilding = Tools::BuildBuilding(this->action.unit);
					if (startedBuilding) BWAPI::Broodwar->printf("Started Building %s", this->action.unit.getName().c_str());
				}

				if (startedBuilding) return true;
			}
		}
	}
	else {
		const BWAPI::Unit myDepot = Tools::GetDepot();
		if (myDepot && !myDepot->isTraining()) {

			if (BWAPI::Broodwar->self()->gas() >= this->trigger.unitToWait.gasPrice()
				&& BWAPI::Broodwar->self()->minerals() >= this->trigger.unitToWait.mineralPrice()
				&& Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed() > 0) {
				myDepot->train(this->trigger.unitToWait);
				BWAPI::Broodwar->printf(("Training worker (" + std::to_string((int)(0.5*BWAPI::Broodwar->self()->supplyUsed())) + "/" + std::to_string(this->trigger.value) + ")").c_str());
			}
		}
	}

	return false;
}