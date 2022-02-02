#pragma once
#include "BuildOrderStep.h"
#include <vector>

class BuildOrder
{
	public:
		BuildOrderStep getCurrentStep();
		bool nextStep();
		bool onGoing();

	protected:
		std::vector<BuildOrderStep> steps;
		unsigned int currentStep = 0;
		bool finished = false;
};

