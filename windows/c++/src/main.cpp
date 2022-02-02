#include "KBot.h"
#include <BWAPI.h>
#include <BWAPI/Client.h>
#include <chrono>
#include <iostream>
#include <thread>

template <typename Bot>
void dispatchEvents(Bot &bot) {
    for (auto &e : BWAPI::Broodwar->getEvents())
        switch (e.getType()) {
        case BWAPI::EventType::MatchStart:
            bot.onStart();
            break;
        case BWAPI::EventType::MatchEnd:
            bot.onEnd(e.isWinner());
            break;
        case BWAPI::EventType::MatchFrame:
            bot.onFrame();
            break;
        case BWAPI::EventType::MenuFrame:
            // Looking at the server implementation, it seems that this event is never fired while
            // being in-game. It looks like events are fired every frame, even when we're not in a
            // game. And while we're outside a game, this will be the event fired each frame. So
            // this case has no relevance while being in-game. It will just never happen.
            assert(false);
            break;
        case BWAPI::EventType::SendText:
            bot.onSendText(e.getText());
            break;
        case BWAPI::EventType::ReceiveText:
            bot.onReceiveText(e.getPlayer(), e.getText());
            break;
        case BWAPI::EventType::PlayerLeft:
            bot.onPlayerLeft(e.getPlayer());
            break;
        case BWAPI::EventType::NukeDetect:
            bot.onNukeDetect(e.getPosition());
            break;
        case BWAPI::EventType::UnitDiscover:
            bot.onUnitDiscover(e.getUnit());
            break;
        case BWAPI::EventType::UnitEvade:
            bot.onUnitEvade(e.getUnit());
            break;
        case BWAPI::EventType::UnitShow:
            bot.onUnitShow(e.getUnit());
            break;
        case BWAPI::EventType::UnitHide:
            bot.onUnitHide(e.getUnit());
            break;
        case BWAPI::EventType::UnitCreate:
            bot.onUnitCreate(e.getUnit());
            break;
        case BWAPI::EventType::UnitDestroy:
            bot.onUnitDestroy(e.getUnit());
            break;
        case BWAPI::EventType::UnitMorph:
            bot.onUnitMorph(e.getUnit());
            break;
        case BWAPI::EventType::UnitRenegade:
            bot.onUnitRenegade(e.getUnit());
            break;
        case BWAPI::EventType::SaveGame:
            bot.onSaveGame(e.getText());
            break;
        case BWAPI::EventType::UnitComplete:
            bot.onUnitComplete(e.getUnit());
            break;
        default:
            // This will never happen.
            assert(false);
        }
}

int main(int /*argc*/, const char ** /*argv*/) {
    int gameCounter = 0;

    std::cout << "Connecting to server..." << std::endl;
    while (!BWAPI::BWAPIClient.connect()) {
        std::this_thread::sleep_for(std::chrono::milliseconds{300});
    }

    // Main loop
    while (BWAPI::BWAPIClient.isConnected()) {
        std::cout << "Waiting for a game to begin..." << std::endl;
        while (BWAPI::BWAPIClient.isConnected() && !BWAPI::Broodwar->isInGame())
            BWAPI::BWAPIClient.update(); // update shared memory

        if (!BWAPI::BWAPIClient.isConnected())
            break;

        // Handle non-playing cases right here so we can remove them from the bot entirly. (TODO!)
        if (BWAPI::Broodwar->self() == nullptr || BWAPI::Broodwar->isReplay())
            return EXIT_FAILURE;

        // Initialize game objects
        std::cout << ++gameCounter << ". game ready!" << std::endl;
        KBot::KBot kbot;

        // Dispatch events
        while (BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame()) {
            dispatchEvents(kbot);

            // Trigger shared memory update. Blocks until next frame.
            BWAPI::BWAPIClient.update();
        }
        std::cout << "Game ended!" << std::endl;
    }
    std::cout << "Connection closed!" << std::endl;

    return EXIT_SUCCESS;
}


/*#include <BWAPI.h>
#include <BWAPI/Client.h>
#include "StarterBot.h"
#include <iostream>
#include <thread>
#include <chrono>

void PlayGame();

int main(int argc, char * argv[])
{
    size_t gameCount = 0;

    // if we are not currently connected to BWAPI, try to reconnect
    while (!BWAPI::BWAPIClient.connect())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
    }

    // if we have connected to BWAPI
    while (BWAPI::BWAPIClient.isConnected())
    {
        // the starcraft exe has connected but we need to wait for the game to start
        std::cout << "Waiting for game start\n";
        while (BWAPI::BWAPIClient.isConnected() && !BWAPI::Broodwar->isInGame())
        {
            BWAPI::BWAPIClient.update();
        }

        // Check to see if Starcraft shut down somehow
        if (BWAPI::BroodwarPtr == nullptr) { break; }

        // If we are successfully in a game, call the module to play the game
        if (BWAPI::Broodwar->isInGame())
        {
            std::cout << "Playing game " << gameCount++ << " on map " << BWAPI::Broodwar->mapFileName() << "\n";

            PlayGame();
        }
    }



	return 0;
}

void PlayGame()
{
    StarterBot bot;

    // The main game loop, which continues while we are connected to BWAPI and in a game
    while (BWAPI::BWAPIClient.isConnected() && BWAPI::Broodwar->isInGame())
    {
        // Handle each of the events that happened on this frame of the game
        for (const BWAPI::Event& e : BWAPI::Broodwar->getEvents())
        {
            switch (e.getType())
            {
                case BWAPI::EventType::MatchStart:   { bot.onStart();                       break; }
                case BWAPI::EventType::MatchFrame:   { bot.onFrame();                       break; }
                case BWAPI::EventType::MatchEnd:     { bot.onEnd(e.isWinner());             break; }
                case BWAPI::EventType::UnitShow:     { bot.onUnitShow(e.getUnit());         break; }
                case BWAPI::EventType::UnitHide:     { bot.onUnitHide(e.getUnit());         break; }
                case BWAPI::EventType::UnitCreate:   { bot.onUnitCreate(e.getUnit());       break; }
                case BWAPI::EventType::UnitMorph:    { bot.onUnitMorph(e.getUnit());        break; }
                case BWAPI::EventType::UnitDestroy:  { bot.onUnitDestroy(e.getUnit());      break; }
                case BWAPI::EventType::UnitRenegade: { bot.onUnitRenegade(e.getUnit());     break; }
                case BWAPI::EventType::UnitComplete: { bot.onUnitComplete(e.getUnit());     break; }
                case BWAPI::EventType::SendText:     { bot.onSendText(e.getText());         break; }
            }
        }

        BWAPI::BWAPIClient.update();
        if (!BWAPI::BWAPIClient.isConnected())
        {
            std::cout << "Disconnected\n";
            break;
        }
    }

    std::cout << "Game Over\n";
}
*/