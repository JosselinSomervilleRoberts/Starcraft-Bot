#pragma once

#include "MapTools.h"
#include "BuildOrder/BuildOrder.h"
#include "Manager.h"
#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
    void drawDebugInformation();

    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);

	void followBuildOrder();

	// Getter for members.
	// Manager& manager() { return m_manager; };
	// const Manager& manager() const { return m_manager; };

private:
	BuildOrder bo;
	Manager m_manager;
};