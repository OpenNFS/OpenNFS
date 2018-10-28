//
// Created by Amrik on 28/10/2018.
//

#include "TrainingGround.h"


TrainingGround::TrainingGround(uint16_t populationID, shared_ptr <ONFSTrack> track, shared_ptr <Car> car) {
    this->populationID = populationID;
    this->car = car;
    this->track = track;
    this->carNetUnderTest = carNetUnderTest;

    physicsEngine.registerTrack(this->track);
    physicsEngine.registerVehicle(this->car);
}

void TrainingGround::Step(float stepTime){
    // Apply the networks parameters
    car->simulate();

    // Step the physics simulation
    physicsEngine.stepSimulation(stepTime);
}

float TrainingGround::EvaluateFitness(){
    int closestBlockID = 0;

    float lowestDistanceSqr = FLT_MAX;
    // Get nearest track block to car
    for (auto &track_block :  track->track_blocks) {
        float distanceSqr = glm::length2(glm::distance(car->car_body_model.position, track_block.center));
        if (distanceSqr < lowestDistanceSqr) {
            closestBlockID = track_block.block_id;
            lowestDistanceSqr = distanceSqr;
        }
    }

    // Return a number corresponding to the distance driven
    return (float) closestBlockID;
}
