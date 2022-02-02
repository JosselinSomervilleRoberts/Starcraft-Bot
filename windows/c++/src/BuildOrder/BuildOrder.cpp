#include "BuildOrder.h"

BuildOrderStep BuildOrder::getCurrentStep() {
	// Returns the currentStep of the build Order
	return this->steps[this->currentStep];
}

bool BuildOrder::nextStep() {
	// Go to the next step of the build order
	// And returns true if the build order is still in progress
	this->currentStep++;
	if (this->currentStep >= this->steps.size()) { finished = true; }
	return this->onGoing();
}

bool BuildOrder::onGoing() {
	return !(this->finished);
}