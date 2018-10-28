//
// Created by Amrik on 28/10/2018.
//

#include "MutationController.h"

MutationController::MutationController(uint16_t populationSize, uint16_t nGenerations, shared_ptr<ONFSTrack> training_track) {
    LOG(INFO) << "Beginning GA evolution session. Population Size: " << populationSize << " nGenerations: " << nGenerations << " Track: " << training_track->name;
    const float stepTime = 1 / 60.f;

    this->training_track = training_track;
    physicsEngine.registerTrack(this->training_track);

    std::vector<CarModel> fce_models = NFS3::LoadFCE("./assets/car/NFS_3/diab/car.fce");

    // Create new cars, each with new RaceNetworks
    for (uint16_t pop_Idx = 0; pop_Idx < populationSize; ++pop_Idx) {
        shared_ptr<Car> car_agent = std::make_shared<Car>(pop_Idx, fce_models, NFS_3, "diab", RaceNet());
        // TODO: Ensure that these cannot hit eachother
        physicsEngine.registerVehicle(car_agent);
        car_agents.emplace_back(car_agent);
    }

    LOG(INFO) << "Agents initialised. Simulating for " << nGenerations << " ticks";

    // Simulate the population
    for (uint16_t gen_Idx = 0; gen_Idx < nGenerations; ++gen_Idx) {
        for (auto &car_agent : car_agents) {
            car_agent->simulate();
        }
        physicsEngine.stepSimulation(stepTime);
    }

    // Evaluate the fitnesses
    for (auto &car_agent : car_agents) {
        LOG(INFO) << car_agent->populationID << " made it to " << EvaluateFitness(car_agent);
    }

    LOG(INFO) << "Done";
}

// Move this to agent class?
float MutationController::EvaluateFitness(shared_ptr<Car> car_agent) {
    int closestBlockID = 0;

    float lowestDistanceSqr = FLT_MAX;
    // Get nearest track block to car
    for (auto &track_block :  training_track->track_blocks) {
        float distanceSqr = glm::length2(glm::distance(car_agent->car_body_model.position, track_block.center));
        if (distanceSqr < lowestDistanceSqr) {
            closestBlockID = track_block.block_id;
            lowestDistanceSqr = distanceSqr;
        }
    }

    // Return a number corresponding to the distance driven
    return (float) closestBlockID;
}
