//
// Created by Amrik on 28/10/2018.
//

#include "MutationController.h"

MutationController::MutationController(uint16_t populationSize, uint16_t nGenerations, shared_ptr<ONFSTrack> training_track) {
    LOG(INFO) << "Beginning GA evolution session. Population Size: " << populationSize << " nGenerations: " << nGenerations << " Track: " << training_track->name;
    const float stepTime = 1/60.f;

    std::vector<CarModel> fce_models = NFS3::LoadFCE("./assets/car/NFS_3/diab/car.fce");

    // Create new game worlds, each with independent physics. Create new cars, each with new RaceNetworks
    // TODO: Probably better to place them all in the same world and physics engine, but disable collisions and raycasting to other cars
    for(uint16_t pop_Idx = 0; pop_Idx < populationSize; ++pop_Idx){
        trainingGrounds.emplace_back(std::make_shared<TrainingGround>(pop_Idx, training_track, std::make_shared<Car>(fce_models, NFS_3, "diab", RaceNet())));
    }

    LOG(INFO) << "Training Grounds initialised. Simulating for " << nGenerations << " ticks";

    // Simulate the population
    for(uint16_t gen_Idx = 0; gen_Idx < nGenerations; ++gen_Idx){
        for(auto &trainingGround : trainingGrounds){
            trainingGround->Step(stepTime);
        }
    }

    // Evaluate the fitnesses
    for(auto &trainingGround : trainingGrounds){
        LOG(INFO) << trainingGround->populationID << " made it to " << trainingGround->EvaluateFitness();;
    }

    LOG(INFO) << "Done";


}
