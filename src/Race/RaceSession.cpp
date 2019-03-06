//
// Created by amrik on 01/03/19.
//

#include "RaceSession.h"

RaceSession::RaceSession(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                         const std::vector<NeedForSpeed> &installedNFS,
                         const std::shared_ptr<ONFSTrack> &currentTrack, std::shared_ptr<Car>
                         &currentCar) : window(glWindow), logger(onfsLogger), installedNFSGames(installedNFS),
                                        track(currentTrack), car(currentCar) {
    loadedAssets = {car->tag, car->name, track->tag, track->name};
    mainCamera = Camera(track->trackBlocks[0].center, Config::get().fov, 0.f, 0.f, window);
    mainCamera.generateSpline(track->trackBlocks);

    // Generate the collision meshes
    physicsEngine.registerTrack(track);
    physicsEngine.registerVehicle(car);

    if (track->tag == NFS_2_SE || track->tag == NFS_2 || track->tag == NFS_3_PS1) {
        userParams.useNbData = false;
    }

    if (track->tag != NFS_3_PS1) {
        mainCamera.setCameraAnimation(track->cameraAnimations);
    }
    // Reset player character to start
    CarAgent::resetToVroad(0, 0, 0.25f, track, car);
    SpawnRacers(Config::get().nRacers);
}

AssetData RaceSession::simulate() {


    while (!glfwWindowShouldClose(window)) {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        auto deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.windowActive && !ImGui::GetIO().MouseDown[1]) {
            car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
            car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
            car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        }

        if(userParams.simulateCars){
            for(auto &racer : racers){
                racer.simulate();
            }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            CarAgent::resetToVroad(mainRenderer.closestBlockID, 0, 0.f, track, car);
        }

        // Step the physics simulation and update physics debug view matrices
        physicsEngine.mydebugdrawer.SetMatrices(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix);
        physicsEngine.stepSimulation(deltaTime);

        bool assetChange = mainRenderer.Render(totalTime, deltaTime, mainCamera, userParams, loadedAssets, car, racers, physicsEngine);

        if (assetChange) {
            return loadedAssets;
        }

        // For the next frame, the "last time" will be "now"
        lastTime = currentTime;
        // Keep track of total elapsed time too
        totalTime += deltaTime;
        ++ticks;
    }

    // Just set a flag temporarily to let main know that we outta here
    loadedAssets.trackTag = UNKNOWN;
    return loadedAssets;
}

void RaceSession::SpawnRacers(int nRacers) {
    float racerSpawnOffset = -0.25f;
    for(uint16_t racer_Idx = 0; racer_Idx < nRacers; ++racer_Idx){
        CarAgent racer(racerNames[racer_Idx % 23], BEST_NETWORK_PATH, car, track);
        physicsEngine.registerVehicle(racer.car);
        racer.resetToVroad(0, racer_Idx + 1, racerSpawnOffset, track, racer.car);
        racers.emplace_back(racer);
        racerSpawnOffset = -racerSpawnOffset;
    }
}

