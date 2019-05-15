#include "TrainingGround.h"

TrainingGround::TrainingGround(uint16_t nGenerations, uint32_t nTicks,
                               std::shared_ptr<ONFSTrack> &training_track, std::shared_ptr<Car> &training_car,
                               std::shared_ptr<Logger> &logger, GLFWwindow *gl_window) : window(gl_window),
                                                                                         raceNetRenderer(gl_window,
                                                                                                         logger) {
    LOG(INFO) << "Beginning GA evolution session. nGenerations Cap: "
              << nGenerations << " nTicks: " << nTicks << " Track: " << training_track->name << " ("
              << ToString(training_track->tag) << ")";

    this->training_track = training_track;
    this->training_car = training_car;
    physicsEngine.registerTrack(this->training_track);

    TrainAgents(nGenerations, nTicks);

    LOG(INFO) << "Done";
}

void TrainingGround::TrainAgents(uint16_t nGenerations, uint32_t nTicks) {
    // 8 input, 4 output, 6 bias, cannot be recurrent
    pool pool(8, 5, 4, false);
    pool.import_fromfile("generation.dat");
    bool haveWinner = false;
    uint32_t gen_Idx = 0;
    int globalMaxFitness = 0;

    // iterator
    int specieCounter = 0;
    auto specieIter = pool.species.begin();

    // init initial
    if (specieIter != pool.species.end()) {
        for (size_t i = 0; i < (*specieIter).genomes.size(); i++) {
            // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
            carAgents.emplace_back(i, this->training_car, this->training_track);
            carAgents[i].raceNet.from_genome((*specieIter).genomes[i]);
            physicsEngine.registerVehicle(carAgents[i].car);
            carAgents[i].reset();
        }
    }
    LOG(INFO) << "Agents initialised";

    // Start simulating GA generations
    while (!glfwWindowShouldClose(window) && (!haveWinner)) {
        gen_Idx++;
        // If user provided a generation cap and we've hit it, bail
        if (Config::get().nGenerations != 0) {
            if (gen_Idx == Config::get().nGenerations) break;
        }

        bool allDead = true;

        for (auto &car_agent : carAgents) {
            if (!car_agent.dead) {
                allDead = false;
            }
        }

        // If every Car Agent has died during simulation, iterate through them to find the Agent with the best fitness
        // And export a representation of it's ANN network configuration for inference later
        if (allDead) {
            if (specieIter != pool.species.end()) {
                int best_id = -1;
                for (size_t i = 0; i < (*specieIter).genomes.size(); i++) {
                    (*specieIter).genomes[i].fitness = carAgents[i].fitness;
                    if ((*specieIter).genomes[i].fitness > globalMaxFitness) {
                        globalMaxFitness = (*specieIter).genomes[i].fitness;
                        best_id = i;
                    }
                }
                if (best_id != -1) {
                    carAgents[best_id].raceNet.export_tofile("best_network");
                }
            }

            // Change to a new species, and remove all current car agents operating with genome
            specieIter++;
            specieCounter++;
            for(auto &carAgent : carAgents){
                physicsEngine.getDynamicsWorld()->removeRigidBody(carAgent.car->getVehicleRigidBody());
            }
            carAgents.clear();

            // If TinyAI has gone through all of the species in the pool, begin a new generation
            if (specieIter == pool.species.end()) {
                pool.new_generation();
                std::string fname = "gen";
                fname += std::to_string(pool.generation());
                pool.export_tofile(fname);
                pool.export_tofile("generation.dat");
                std::cerr << "Starting new generation. Number = " << pool.generation() << std::endl;
                specieIter = pool.species.begin();
                specieCounter = 0;
            }

            // Generate new Car Agents from the latest species and corresponding genome
            if (specieIter != pool.species.end())
                for (size_t i = 0; i < (*specieIter).genomes.size(); i++) {
                    // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
                    CarAgent car_agent((uint16_t) i, this->training_car, this->training_track);
                    car_agent.raceNet.from_genome((*specieIter).genomes[i]);
                    physicsEngine.registerVehicle(car_agent.car);
                    car_agent.reset();
                    carAgents.emplace_back(car_agent);
                }
        }

        for (uint32_t tick_Idx = 0; tick_Idx < nTicks; ++tick_Idx) {
            // Simulate the population
            for (auto &car_agent : carAgents) {
                if (car_agent.dead)
                    continue;

                car_agent.simulate();

                physicsEngine.stepSimulation(stepTime);

                if (!Config::get().headless) {
                    raceNetRenderer.Render(tick_Idx, carAgents, training_track);
                }
                if (glfwWindowShouldClose(window)) break;
            }
        }

        unsigned int localMaxFitness = 0;
        for (auto &carAgent : carAgents) {
            if (carAgent.fitness > localMaxFitness) {
                localMaxFitness = carAgent.fitness;
            }
        }

        size_t winnerIdx;
        for (size_t i = 0; i < carAgents.size(); i++)
            if (carAgents[i].isWinner()) {
                haveWinner = true;
                winnerIdx = i;
            }

        if (haveWinner) {
            LOG(INFO) << "WINNER: Saving best agent network to " << BEST_NETWORK_PATH;
            carAgents[winnerIdx].raceNet.export_tofile(BEST_NETWORK_PATH);
        }
        // Display the fitnesses
        LOG(INFO) << "Generation: " << pool.generation() << " Specie number: " << specieCounter
                  << " Genomes in specie: " << carAgents.size() << " Global max fitness: " << globalMaxFitness
                  << " Current specie max: " << localMaxFitness;
    }
}





