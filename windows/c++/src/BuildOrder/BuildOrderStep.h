#pragma once
#include <BWAPI.h>

enum TriggerType {
	NB_UNITS
};

enum ActionType {
	BUILD
};

struct Action {
	ActionType type;
	BWAPI::UnitType unit;
	unsigned int count = 1;
};

struct Trigger {
	TriggerType type;
	unsigned int value;
	BWAPI::UnitType unitToWait;
};


class BuildOrderStep
{
public:
	BuildOrderStep();
	bool isTrigerred();
	bool doAction();

	Trigger trigger;
	Action action;
};

