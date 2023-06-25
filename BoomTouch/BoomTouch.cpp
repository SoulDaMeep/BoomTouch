#include "pch.h"
#include "BoomTouch.h"
#include <vector>

BAKKESMOD_PLUGIN(BoomTouch, "Explodes the player when they touch the ball in a set amount of time", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr < CVarManagerWrapper > _globalCvarManager;

struct ExplodingPlayer {
    float timeRemaining;
    std::string playerId;
};

std::vector<ExplodingPlayer> explodingPlayers;

std::string GetPlayerUniqueId(PriWrapper& pri) {
    if (pri.GetbBot()) {
        return "Bot|" + pri.GetPlayerName().ToString();
    }
    return pri.GetUniqueIdWrapper().GetIdString();
}

float GetDeltaTime() {
    using namespace std::chrono;

    static steady_clock::time_point previousTime = steady_clock::now();

    steady_clock::time_point currentTime = steady_clock::now();
    float deltaTime = duration_cast<duration<float>>(currentTime - previousTime).count();

    previousTime = currentTime;
    return deltaTime;
}

int FindPlayerIndex(std::vector<ExplodingPlayer>& arr, const std::string& playerId) {
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i].playerId == playerId) {
            return i;
        }
    }
    return -1;
}

float deltaTime;
bool isPluginEnabled = false;
bool isReplayActive = false;

void BoomTouch::onLoad() {
    _globalCvarManager = cvarManager;

    cvarManager->registerCvar("BoomTouch_Enabled", "0", "Turn off and on");
    cvarManager->registerCvar("BoomTouch_ExplosionTime", "2", "Time before explosion");

    gameWrapper->HookEvent("Function TAGame.Ball_TA.OnHitGoal", [this](std::string eventName) {
        explodingPlayers.clear();
    });

    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", [this](std::string eventName) {
        isReplayActive = true;
    });

    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", [this](std::string eventName) {
        isReplayActive = false;
    });

    gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnHitBall", [this](CarWrapper car, void* params, std::string eventName) {
        CVarWrapper enabledCvar = cvarManager->getCvar("BoomTouch_Enabled");
        if (enabledCvar.IsNull()) {
            return;
        }

        isPluginEnabled = enabledCvar.getBoolValue();
        if (isPluginEnabled && !isReplayActive) {
            if (car.IsNull()) {
                return;
            }

            PriWrapper pri = car.GetPRI();
            if (pri.IsNull()) {
                return;
            }

            std::string playerId = GetPlayerUniqueId(pri);

            for (const ExplodingPlayer& player : explodingPlayers) {
                if (player.playerId == playerId) {
                    return;
                }
            }

            CVarWrapper explosionTimeCvar = cvarManager->getCvar("BoomTouch_ExplosionTime");
            if (explosionTimeCvar.IsNull()) {
                return;
            }

            int explosionTime = explosionTimeCvar.getIntValue();

            ExplodingPlayer newPlayer;
            newPlayer.playerId = playerId;
            newPlayer.timeRemaining = static_cast<float>(explosionTime);
            explodingPlayers.push_back(newPlayer);
        }
        });


    gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", [this](std::string eventName) {
        deltaTime = GetDeltaTime();

        ServerWrapper server = gameWrapper->GetCurrentGameState();
        if (server.IsNull()) {
            return;
        }

        ArrayWrapper<CarWrapper> cars = server.GetCars();

        for (ExplodingPlayer& player : explodingPlayers) {
            player.timeRemaining -= deltaTime;

            if (player.timeRemaining <= 0.0f) {
                std::string currentPlayerId = player.playerId;

                for (CarWrapper car : cars) {
                    if (car.IsNull()) {
                        return;
                    }

                    PriWrapper pri = car.GetPRI();

                    if (pri.IsNull()) {
                        return;
                    }
                    std::string carPlayerId = GetPlayerUniqueId(pri);

                    if (currentPlayerId == carPlayerId) {
                        int playerIndex = FindPlayerIndex(explodingPlayers, currentPlayerId);
                        explodingPlayers.erase(explodingPlayers.begin() + playerIndex);
                        car.Demolish2(car);
                    }
                }
            }
        }
     });
}

void BoomTouch::onUnload() {}