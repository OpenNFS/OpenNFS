//
// Created by amrik on 01/03/19.
//

#include "RaceSession.h"

RaceSession::RaceSession(GLFWwindow *glWindow, std::shared_ptr<Logger> &onfsLogger,
                         const std::vector<NeedForSpeed> &installedNFS,
                         const shared_ptr<ONFSTrack> &currentTrack, shared_ptr<Car>
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

    CarAgent::resetToVroad(0, track, car);
}

AssetData RaceSession::simulate() {
    // Skip CAN animation of PS1 track loaded
    bool cameraAnimationPlayed = track->tag == NFS_3_PS1;

    while (!glfwWindowShouldClose(window)) {
        // glfwGetTime is called only once, the first time this function is called
        static double lastTime = glfwGetTime();
        // Compute time difference between current and last frame
        double currentTime = glfwGetTime();
        // Update time between engine ticks
        auto deltaTime = float(currentTime - lastTime); // Keep track of time between engine ticks

        // Step the physics simulation and update physics debug view matrices
        physicsEngine.mydebugdrawer.SetMatrices(mainCamera.ViewMatrix, mainCamera.ProjectionMatrix);
        physicsEngine.stepSimulation(deltaTime);

        // Play the original camera animation
        if (!cameraAnimationPlayed) {
            cameraAnimationPlayed = mainCamera.playAnimation(car->car_body_model.position);
        } else if (userParams.attachCamToHermite) {
            mainCamera.useSpline(totalTime);
        } else if (userParams.attachCamToCar) {
            // Compute MVP from keyboard and mouse, centered around a target car
            mainCamera.followCar(car, userParams.windowActive);
        } else {
            // Compute the MVP matrix from keyboard and mouse input
            mainCamera.computeMatricesFromInputs(userParams.windowActive, deltaTime);
        }

        //TODO: Refactor to controller class? AND USE SDL
        if (userParams.simulate_car) {
            // TODO: The AI should go through a list of CarAgents
            //car->simulate();
        } else {
            if (userParams.windowActive && !ImGui::GetIO().MouseDown[1]) {
                car->applyAccelerationForce(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
                                            glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
                car->applyBrakingForce(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
                car->applySteeringRight(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                car->applySteeringLeft(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            CarAgent::resetToVroad(mainRenderer.closestBlockID, track, car);
        }

        bool assetChange = mainRenderer.Render(deltaTime, totalTime, mainCamera, userParams, loadedAssets, physicsEngine);

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

