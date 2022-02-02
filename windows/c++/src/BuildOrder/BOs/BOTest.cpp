#include "BOTest.h"

BOTest::BOTest() : BuildOrder() {
	BuildOrderStep step1;

	step1.trigger.type = NB_UNITS;
	step1.trigger.unitToWait = BWAPI::Broodwar->self()->getRace().getWorker();
	step1.trigger.value = 9;

	step1.action.type = BUILD;
	step1.action.unit = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
	step1.action.count = 1;

	this->steps.push_back(step1);
	//this->steps.push_back(step1);



	BuildOrderStep step2;
	step2.trigger.type = NB_UNITS;
	step2.trigger.unitToWait = BWAPI::Broodwar->self()->getRace().getWorker();
	step2.trigger.value = 11;

	step2.action.type = BUILD;
	step2.action.unit = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	this->steps.push_back(step2);


	BuildOrderStep step3;
	step3.trigger.type = NB_UNITS;
	step3.trigger.unitToWait = BWAPI::Broodwar->self()->getRace().getWorker();
	step3.trigger.value = 13;

	step3.action.type = BUILD;
	step3.action.unit = BWAPI::Broodwar->self()->getRace().getRefinery();

	this->steps.push_back(step3);


	BuildOrderStep step4;
	step4.trigger.type = NB_UNITS;
	step4.trigger.unitToWait = BWAPI::Broodwar->self()->getRace().getWorker();
	step4.trigger.value = 13;

	step4.action.type = BUILD;
	step4.action.unit = BWAPI::UnitTypes::Terran_Barracks;

	this->steps.push_back(step4);
}