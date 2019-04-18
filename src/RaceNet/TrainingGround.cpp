//
// Created by Amrik on 28/10/2018.
//

#include "TrainingGround.h"

#include "neat.h"
#include "population.h"
#include "experiments.h"

TrainingGround::TrainingGround(uint16_t populationSize, uint16_t nGenerations, uint32_t nTicks,
                               std::shared_ptr<ONFSTrack> &training_track, std::shared_ptr<Car> &training_car,
                               std::shared_ptr<Logger> &logger, GLFWwindow *gl_window) : window(gl_window),
                                                                                         raceNetRenderer(gl_window,
                                                                                                         logger) {
    LOG(INFO) << "Beginning GA evolution session. Population Size: " << populationSize << " nGenerations Cap: "
              << nGenerations << " nTicks: " << nTicks << " Track: " << training_track->name << " ("
              << ToString(training_track->tag) << ")";

    this->training_track = training_track;
    this->training_car = training_car;
    physicsEngine.registerTrack(this->training_track);

    TrainAgents(nGenerations, nTicks);

    LOG(INFO) << "Done";
}


bool xor_evaluatea(Organism *org) {
    double out[4]; //The four outputs
    double this_out; //The current output
    int count;
    double errorsum;

    bool success = false;  //Check for successful activation
    int numnodes;  /* Used to figure out how many nodes
		    should be visited during activation */

    int net_depth; //The max depth of the network to be activated
    int relax; //Activates until relaxation

    //The four possible input combinations to xor
    //The first number is for biasing
    double in[4][3] = {{1.0, 0.0, 0.0},
                       {1.0, 0.0, 1.0},
                       {1.0, 1.0, 0.0},
                       {1.0, 1.0, 1.0}};

    Network *net = org->net;
    numnodes = ((org->gnome)->nodes).size();

    net_depth = net->max_depth();

    //Load and activate the network on each input
    for (count = 0; count <= 3; count++) {
        net->load_sensors(in[count]);

        //Relax net and get output
        success = net->activate();

        //use depth to ensure relaxation
        for (relax = 0; relax <= net_depth; relax++) {
            success = net->activate();
            this_out = (*(net->outputs.begin()))->activation;
        }

        out[count] = (*(net->outputs.begin()))->activation;

        net->flush();
    }

    if (success) {
        errorsum = (fabs(out[0]) + fabs(1.0 - out[1]) + fabs(1.0 - out[2]) + fabs(out[3]));
        org->fitness = pow((4.0 - errorsum), 2);
        org->error = errorsum;
    } else {
        //The network is flawed (shouldnt happen)
        errorsum = 999.0;
        org->fitness = 0.001;
    }

#ifndef NO_SCREEN_OUT
    std::cout << "Org " << (org->gnome)->genome_id << "                                     error: " << errorsum
              << "  [" << out[0] << " " << out[1] << " " << out[2] << " " << out[3] << "]" << endl;
    std::cout << "Org " << (org->gnome)->genome_id << "                                     fitness: " << org->fitness
              << endl;
#endif

    if ((out[0] < 0.5) && (out[1] >= 0.5) && (out[2] >= 0.5) && (out[3] < 0.5)) {
        org->winner = true;
        return true;
    } else {
        org->winner = false;
        return false;
    }
}

int xor_epocha(Population *pop, int generation, char *filename, int &winnernum, int &winnergenes, int &winnernodes) {
    vector<Organism *>::iterator curorg;
    vector<Species *>::iterator curspecies;

    bool win = false;

    //Evaluate each organism on a test
    for (curorg = (pop->organisms).begin(); curorg != (pop->organisms).end(); ++curorg) {
        if (xor_evaluatea(*curorg)) {
            win = true;
            winnernum = (*curorg)->gnome->genome_id;
            winnergenes = (*curorg)->gnome->extrons();
            winnernodes = ((*curorg)->gnome->nodes).size();
            if (winnernodes == 5) {
                //You could dump out optimal genomes here if desired
                //(*curorg)->gnome->print_to_filename("xor_optimal");
                //cout<<"DUMPED OPTIMAL"<<endl;
            }
        }
    }

    //Average and max their fitnesses for dumping to file and snapshot
    for (curspecies = (pop->species).begin(); curspecies != (pop->species).end(); ++curspecies) {
        //This experiment control routine issues commands to collect ave
        //and max fitness, as opposed to having the snapshot do it,
        //because this allows flexibility in terms of what time
        //to observe fitnesses at
        (*curspecies)->compute_average_fitness();
        (*curspecies)->compute_max_fitness();
    }

    //Take a snapshot of the population, so that it can be
    //visualized later on
    //if ((generation%1)==0)
    //  pop->snapshot();

    //Only print to file every print_every generations
    if (win ||((generation % (NEAT::print_every)) == 0))
        pop->print_to_file_by_species(filename);

    if (win) {
        for (curorg = (pop->organisms).begin(); curorg != (pop->organisms).end(); ++curorg) {
            if ((*curorg)->winner) {
                std::cout << "WINNER IS #" << ((*curorg)->gnome)->genome_id << endl;
                //Prints the winner to file
                //IMPORTANT: This causes generational file output!
                print_Genome_tofile((*curorg)->gnome, "xor_winner");
            }
        }

    }
    pop->epoch(generation);
    return win;
}

Population *xor_testa(int gens) {
    NEAT::load_neat_params("../lib/NEAT/test.ne",true);

    Population *pop = 0;
    Genome *start_genome;
    char curword[20];
    int id;
    int evals[NEAT::num_runs];  //Hold records for each run
    int genes[NEAT::num_runs];
    int nodes[NEAT::num_runs];
    int winnernum;
    int winnergenes;
    int winnernodes;

    memset(evals, 0, NEAT::num_runs * sizeof(int));
    memset(genes, 0, NEAT::num_runs * sizeof(int));
    memset(nodes, 0, NEAT::num_runs * sizeof(int));

    ifstream iFile("../lib/NEAT/xorstartgenes", ios::in);
    ASSERT(iFile.is_open(), "Unable to open gene file!");

    std::cout << "START XOR TEST" << endl;
    std::cout << "Reading in the start genome" << endl;

    //Read in the start Genome
    iFile >> curword;
    iFile >> id;
    std::cout << "Reading in Genome id " << id << endl;
    start_genome = new Genome(id, iFile);
    iFile.close();

    for (int expIdx = 0; expIdx < NEAT::num_runs; ++expIdx) {
        //Spawn the Population
        std::cout << "Spawning Population off Genome2" << endl;
        pop = new Population(start_genome, NEAT::pop_size);
        std::cout << "Verifying Spawned Pop" << endl;
        pop->verify();

        for (int genIdx = 0; genIdx < gens; ++genIdx) {
            std::cout << "Epoch " << genIdx << endl;

            char temp[50];
            sprintf(temp, "gen_%d", genIdx);

            //Check for success
            if (xor_epocha(pop, genIdx, temp, winnernum, winnergenes, winnernodes)) {
                //	if (xor_epoch(pop,genIdx,fnamebuf->str(),winnernum,winnergenes,winnernodes)) {
                //Collect Stats on end of experiment
                evals[expIdx] = NEAT::pop_size * (genIdx - 1) + winnernum;
                genes[expIdx] = winnergenes;
                nodes[expIdx] = winnernodes;
                genIdx = gens;

            }
        }
        if (expIdx < NEAT::num_runs - 1) delete pop;
    }

    //Average and print stats
    int samples = 0;
    int totalevals = 0;
    for (int expIdx = 0; expIdx < NEAT::num_runs; expIdx++) {
        std::cout << evals[expIdx] << endl;
        if (evals[expIdx] > 0) {
            totalevals += evals[expIdx];
            samples++;
        }
    }
    std::cout << "Failures: " << (NEAT::num_runs - samples) << " out of " << NEAT::num_runs << " runs" << endl;

    return pop;

}

void TrainingGround::TrainAgents(uint16_t nGenerations, uint32_t nTicks) {
    //NEAT::Population *p = xor_testa(20);
    //return;


    // 8 input, 4 output, 6 bias, cannot be recurrent
    pool pool(8, 5, 3, false);
    pool.import_fromfile("generation.dat");
    bool haveWinner = false;
    uint32_t gen_Idx = 0;
    int globalMaxFitness = 0;

    // iterator
    int specie_counter = 0;
    auto specieIter = pool.species.begin();

    // init initial
    if (specieIter != pool.species.end()) {
        for (size_t i = 0; i < (*specieIter).genomes.size(); i++) {
            // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
            CarAgent car_agent(i, this->training_car, this->training_track);
            car_agent.raceNet.from_genome((*specieIter).genomes[i]);
            physicsEngine.registerVehicle(car_agent.car);
            car_agent.reset();
            carAgents.emplace_back(car_agent);
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

            specieIter++;
            specie_counter++;
            carAgents.clear();

            if (specieIter == pool.species.end()) {
                pool.new_generation();
                std::string fname = "gen";
                fname += std::to_string(pool.generation());
                pool.export_tofile(fname);
                pool.export_tofile("generation.dat");
                std::cerr << "Starting new generation. Number = " << pool.generation() << std::endl;
                specieIter = pool.species.begin();
                specie_counter = 0;
            }

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

        unsigned int local_maxfitness = 0;
        for (auto &carAgent : carAgents) {
            if (carAgent.fitness > local_maxfitness) {
                local_maxfitness = carAgent.fitness;
            }
        }

        size_t winner_id;
        for (size_t i = 0; i < carAgents.size(); i++)
            if (carAgents[i].isWinner()) {
                //haveWinner = true;
                winner_id = i;
            }

        if (haveWinner) {
            LOG(INFO) << "WINNER: Saving best agent network to " << BEST_NETWORK_PATH;
            carAgents[winner_id].raceNet.export_tofile(BEST_NETWORK_PATH);
        }
        // Display the fitnesses
        LOG(INFO) << "Generation: " << pool.generation() << " Specie number: " << specie_counter
                  << " Genomes in specie: " << carAgents.size() << " Global max fitness: " << globalMaxFitness
                  << " Current specie max: " << local_maxfitness;
    }
}





