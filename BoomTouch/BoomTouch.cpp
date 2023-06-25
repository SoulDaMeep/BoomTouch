#include "pch.h"

#include "BoomTouch.h"

#include <vector>

/*
    General formatting suggestions:
        - Put empty lines between your functions and some of your lines of code to break them up visually for better readability
        - I generally put curly braces on new lines. Makes it much easier to see where a block of code begins and ends
*/

BAKKESMOD_PLUGIN(BoomTouch, "Explodes the player when they touch the ball in a set amount of time", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr < CVarManagerWrapper > _globalCvarManager;

struct playerExplode {
    float time;
    std::string id;
};

std::vector < playerExplode > explodeyArr;

// determine wether the player id is a bot or an actual player
std::string GetUniqueId(PriWrapper& pri) {
    if (pri.GetbBot()) {
        return "Bot|" + pri.GetPlayerName().ToString();
    }
    return pri.GetUniqueIdWrapper().GetIdString();
}

// get the delta time between the previous frame and the current frame
float GetDelta() {
    using namespace std::chrono;

    static steady_clock::time_point PreviousTime = steady_clock::now();

    //Store the current time and calculate the delta from that
    steady_clock::time_point CurrentTime = steady_clock::now();
    float InputDelta = duration_cast <duration < float >> (CurrentTime - PreviousTime).count();

    //Set PreviousTime for the next delta call
    PreviousTime = CurrentTime;
    return InputDelta;
}

// search for players in the main array(std::vector)
int findPlayer(std::vector < playerExplode > arr, playerExplode ele) {
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i].id == ele.id) {
            return i;
        }
    }
    // default return value
    return -1;
}

float delta;
bool on = false;
bool inReplay = false;
void BoomTouch::onLoad() {
    _globalCvarManager = cvarManager;
    cvarManager->registerCvar("BoomTouch_On", "0", "Turn off and on"); // Give elaborate names -> BoomTouch, another plugin could use the same cvar
    cvarManager->registerCvar("BoomTouch_TimeBeforeExplosion", "2", "boom");
    gameWrapper->HookEventWithCaller < CarWrapper >("Function TAGame.Car_TA.OnHitBall", [this](CarWrapper car, void* params, std::string eventName) {
        CVarWrapper onOff = cvarManager->getCvar("BoomTouch_On");
    //null check cvar -> BM no longer has the cvar, in case i change the name of the cvar
    if (onOff.IsNull()) {
        return;
    }
    bool on = onOff.getBoolValue();
    if (on && !inReplay) {
        // if there is no car, do nothing
        if (car.IsNull()) {
            return;
        }

        // if there is no pri, do nothing
        PriWrapper pri = car.GetPRI();
        if (pri.IsNull()) {
            return;
        }

        // get the id, if its a bot return it as a bot id
        std::string id = GetUniqueId(pri);

        //does the id already exist in the array?
        for (playerExplode player : explodeyArr) {
            if (player.id == id) {
                return;
            }
        }

        // get the amount of time before explosion variable
        CVarWrapper timeBeforeExplosion = cvarManager->getCvar("BoomTouch_TimeBeforeExplosion");
        if (timeBeforeExplosion.IsNull()) {
            return;
        }

        // get the int value
        int tbe = timeBeforeExplosion.getIntValue();

        // create a new player with the time variable
        playerExplode player;
        player.id = id;
        player.time = tbe; // change this number for faster or slower resawn time
        explodeyArr.push_back(player);
    }
        });

    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnHitGoal", [this](std::string eventName) {
        explodeyArr = {};
        });
    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", [this](std::string eventName) {
        inReplay = true;
        });
    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", [this](std::string eventName) {
        inReplay = false;
        });
    gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", [this](std::string eventName) {
        delta = GetDelta();

    //get the server
    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (server.IsNull()) {
        return;
    }

    //get all the cars
    ArrayWrapper < CarWrapper > cars = server.GetCars();

    for (playerExplode& player : explodeyArr) {
        player.time -= delta;

        if (player.time <= 0.0f) { // I would suggest <= 0 instead of just < 0

            //looking for this id
            std::string currentId = player.id;

            for (CarWrapper car : cars) {
                //does the car exist still?
                if (car.IsNull()) {
                    return;
                }

                PriWrapper pri = car.GetPRI();

                if (pri.IsNull()) {
                    return;
                }
                std::string carId = GetUniqueId(pri);

                if (currentId == carId) {
                    int playerIndex = findPlayer(explodeyArr, player);
                    explodeyArr.erase(explodeyArr.begin() + playerIndex);
                    car.Demolish2(car);
                }
            }
        }
    }
    // End of indented block
        });
}

void BoomTouch::onUnload() {}