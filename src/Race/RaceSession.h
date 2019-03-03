//
// Created by amrik on 01/03/19.
//

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Scene/Camera.h"
#include "../Physics/PhysicsEngine.h"
#include "../Loaders/car_loader.h"
#include "../Loaders/trk_loader.h"
#include "../RaceNet/CarAgent.h"
#include "../Util/Logger.h"
#include "../Config.h"
#include "../Renderer/Renderer.h"

class RaceSession {
private:
    // TODO: Read this from file
    char const *racerNames[21] =
            {
                    "DumbPanda",
                    "Spark198rus",
                    "Keiiko",
                    "N/A",
                    "Patas De Pavo",
                    "Dopamine Flint",
                    "Oh Hansssss",
                    "scaryred24",
                    "MaximilianVeers",
                    "AJ_Lethal",
                    "Sirius-R",
                    "Ewil",
                    "heyitsleo",
                    "MADMAN_nfs",
                    "Wild One",
                    "Gotcha",
                    "Mulligan",
                    "Lead Foot",
                    "Ace",
                    "Dead Beat",
                    "Ram Rod"
            };

    GLFWwindow *window;
    std::shared_ptr<Logger> logger;
    std::vector<NeedForSpeed> installedNFSGames;
    AssetData loadedAssets;
    shared_ptr<ONFSTrack> track;
    shared_ptr<Car> car;
    std::vector<CarAgent> racers;
    PhysicsEngine physicsEngine;
    Camera mainCamera;
    ParamData userParams;
    Renderer mainRenderer = Renderer(window, logger, installedNFSGames, track, car);

    uint64_t ticks = 0; // Engine ticks elapsed
    float totalTime = 0;

    void SpawnRacers(int nRacers);
public:
    RaceSession(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger, const std::vector<NeedForSpeed> &installedNFS, const shared_ptr<ONFSTrack> &currentTrack, shared_ptr<Car> &currentCar);
    AssetData simulate();
};
