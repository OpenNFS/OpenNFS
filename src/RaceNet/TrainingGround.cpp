//
// Created by Amrik on 28/10/2018.
//

#include "TrainingGround.h"

TrainingGround::TrainingGround(uint16_t populationSize, uint16_t nGenerations, uint32_t nTicks,
                               shared_ptr<ONFSTrack> &training_track, shared_ptr<Car> &training_car,
                               std::shared_ptr<Logger> &logger, GLFWwindow *gl_window) : window(gl_window),
                                                                                         raceNetRenderer(gl_window,
                                                                                                         logger) {
    LOG(INFO) << "Beginning GA evolution session. Population Size: " << populationSize << " nGenerations: "
              << nGenerations << " nTicks: " << nTicks << " Track: " << training_track->name << " ("
              << ToString(training_track->tag) << ")";

    this->training_track = training_track;
    this->training_car = training_car;
    physicsEngine.registerTrack(this->training_track);

    TrainAgents(nGenerations, nTicks);

    LOG(INFO) << "Saving best agent network to " << BEST_NETWORK_PATH;
    //car_agents[trainedAgentFitness[0][0]]->carNet.net.saveNetworkParams(BEST_NETWORK_PATH.c_str());

    LOG(INFO) << "Done";
}

void TrainingGround::TrainAgents(uint16_t nGenerations, uint32_t nTicks) {
    // 5 input, 3 output, 1 bias, can be recurrent
    pool ga_pool(3, 3, 1, true);
    ga_pool.import_fromfile("generation.dat");
    bool have_a_winner = false;
    uint32_t gen_Idx = 0;
    unsigned int global_maxfitness = 0;

    // iterator
    unsigned int specie_counter = 0;
    auto specie_it = ga_pool.species.begin();

    // init initial
    if (specie_it != ga_pool.species.end()) {
        for (size_t i = 0; i < (*specie_it).genomes.size(); i++) {
            // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
            CarAgent car_agent(i, this->training_car, this->training_track);
            car_agent.raceNet.from_genome((*specie_it).genomes[i]);
            physicsEngine.registerVehicle(car_agent.car);
            Renderer::ResetToVroad(1, training_track, car_agent.car);
            car_agents.emplace_back(car_agent);
        }
    }
    LOG(INFO) << "Agents initialised";

    // Start simulating GA generations
    //for (uint16_t gen_Idx = 0; gen_Idx < nGenerations; ++gen_Idx) {

    while(!glfwWindowShouldClose(window) && (!have_a_winner)) {
        LOG(INFO) << "Beginning Generation " << gen_Idx++;

        bool all_dead = true;

        for (auto &car_agent : car_agents) {
            if (!car_agent.dead) {
                all_dead = false;
            }
        }

        if (all_dead) {
            if (specie_it != ga_pool.species.end()) {
                int best_id = -1;
                for (size_t i = 0; i < (*specie_it).genomes.size(); i++) {
                    (*specie_it).genomes[i].fitness = car_agents[i].fitness;
                    if ((*specie_it).genomes[i].fitness > global_maxfitness) {
                        global_maxfitness = (*specie_it).genomes[i].fitness;
                        best_id = i;
                    }
                }
                if (best_id != -1) {
                    car_agents[best_id].raceNet.export_tofile("best_network");
                }
            }

            specie_it++;
            specie_counter++;
            car_agents.clear();

            if (specie_it == ga_pool.species.end()) {
                ga_pool.new_generation();
                std::string fname = "result/gen";
                fname += std::to_string(ga_pool.generation());
                ga_pool.export_tofile(fname);
                ga_pool.export_tofile("generation.dat");
                std::cerr << "Starting new generation. Number = " << ga_pool.generation() << std::endl;
                specie_it = ga_pool.species.begin();
                specie_counter = 0;
            }

            if (specie_it != ga_pool.species.end())
                for (size_t i = 0; i < (*specie_it).genomes.size(); i++) {
                    // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
                    CarAgent car_agent(i, this->training_car, this->training_track);
                    car_agent.raceNet.from_genome((*specie_it).genomes[i]);
                    physicsEngine.registerVehicle(car_agent.car);
                    Renderer::ResetToVroad(1, training_track, car_agent.car);
                    car_agents.emplace_back(car_agent);
                }
        }

        for (uint32_t tick_Idx = 0; tick_Idx < nTicks; ++tick_Idx) {
            // Simulate the population
            for (auto &car_agent : car_agents) {
                if (car_agent.dead)
                    continue;

                car_agent.simulate();

                physicsEngine.stepSimulation(stepTime);

                if (!Config::get().headless) {
                    raceNetRenderer.Render(tick_Idx, car_agents, training_track);
                }
                if (glfwWindowShouldClose(window)) abort();
            }
        }

        // Display the fitnesses
        for (auto &car_agent : car_agents) {
            LOG(INFO) << "Agent " << car_agent.populationID << " made it to vroad " << car_agent.fitness;
        }

        unsigned int local_maxfitness = 0;
        for (auto &car_agent : car_agents) {
            if (car_agent.fitness > local_maxfitness) {
                local_maxfitness = car_agent.fitness;
            }
        }

        size_t winner_id;
        for (size_t i = 0; i < car_agents.size(); i++)
            if (car_agents[i].isWinner()) {
                have_a_winner = true;
                winner_id = i;
            }

        if (have_a_winner) {
            car_agents[winner_id].raceNet.export_tofile("winner_network");
        }

        //output_info call
    }
}
