#include "TrainingGround.h"

TrainingGround::TrainingGround(uint16_t nGenerations,
                               uint32_t nTicks,
                               const std::shared_ptr<Track> &training_track,
                               const std::shared_ptr<Car> &training_car,
                               const std::shared_ptr<Logger> &logger,
                               const std::shared_ptr<GLFWwindow> &window) :
    m_window(window), raceNetRenderer(m_window, logger)
{
	LOG(INFO) << "Beginning GA evolution session. nGenerations Cap: " << nGenerations << " nTicks: " << nTicks << " Track: " << training_track->name << " ("
	          << ToString(training_track->tag) << ")";

	this->training_track = training_track;
	this->training_car   = training_car;
	physicsEngine.RegisterTrack(this->training_track);

	TrainAgents(nGenerations, nTicks);

	LOG(INFO) << "Done";
}

void TrainingGround::TrainAgents(uint16_t nGenerations, uint32_t nTicks)
{
	// 8 input, 4 output, 6 bias, cannot be recurrent
	pool pool(8, 5, 4, false);
	pool.import_fromfile("generation.dat");
	bool haveWinner               = false;
	uint32_t gen_Idx              = 0;
	unsigned int globalMaxFitness = 0;

	// iterator
	int specieCounter = 0;
	auto specieIter   = pool.species.begin();

	// init initial
	if (specieIter != pool.species.end())
	{
		for (size_t i = 0; i < (*specieIter).genomes.size(); i++)
		{
			// Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
			trainingAgents.emplace_back(i, this->training_car, this->training_track);
			trainingAgents[i].raceNet.from_genome((*specieIter).genomes[i]);
			physicsEngine.RegisterVehicle(trainingAgents[i].vehicle);
			trainingAgents[i].Reset();
		}
	}
	LOG(INFO) << "Agents initialised";

	// Start simulating GA generations
	while (!glfwWindowShouldClose(m_window.get()) && (!haveWinner))
	{
		gen_Idx++;
		// If user provided a generation cap and we've hit it, bail
		if (Config::get().nGenerations != 0)
		{
			if (gen_Idx == Config::get().nGenerations)
				break;
		}

		bool allDead = true;

		for (auto &car_agent : trainingAgents)
		{
			if (!car_agent.isDead)
			{
				allDead = false;
			}
		}

		// If every Car Agent has died during simulation, iterate through them to find the Agent with the best fitness
		// And export a representation of it's ANN network configuration for inference later
		if (allDead)
		{
			if (specieIter != pool.species.end())
			{
				size_t best_id = -1;
				for (size_t i = 0; i < (*specieIter).genomes.size(); i++)
				{
					(*specieIter).genomes[i].fitness = trainingAgents[i].fitness;
					if ((*specieIter).genomes[i].fitness > globalMaxFitness)
					{
						globalMaxFitness = (*specieIter).genomes[i].fitness;
						best_id          = i;
					}
				}
				if (best_id != -1)
				{
					trainingAgents[best_id].raceNet.export_tofile("best_network");
				}
			}

			// Change to a new species, and remove all current car agents operating with genome
			specieIter++;
			specieCounter++;
			for (auto &carAgent : trainingAgents)
			{
				// physicsEngine.m_pDynamicsWorld->removeRigidBody(carAgent.car->getVehicleRigidBody());
			}
			trainingAgents.clear();

			// If TinyAI has gone through all of the species in the pool, begin a new generation
			if (specieIter == pool.species.end())
			{
				pool.new_generation();
				std::string fname = "gen";
				fname += std::to_string(pool.generation());
				pool.export_tofile(fname);
				pool.export_tofile("generation.dat");
				std::cerr << "Starting new generation. Number = " << pool.generation() << std::endl;
				specieIter    = pool.species.begin();
				specieCounter = 0;
			}

			// Generate new Car Agents from the latest species and corresponding genome
			if (specieIter != pool.species.end())
				for (size_t i = 0; i < (*specieIter).genomes.size(); i++)
				{
					// Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
					TrainingAgent trainingAgent((uint16_t) i, this->training_car, this->training_track);
					trainingAgent.raceNet.from_genome((*specieIter).genomes[i]);
					physicsEngine.RegisterVehicle(trainingAgent.vehicle);
					trainingAgent.Reset();
					trainingAgents.emplace_back(trainingAgent);
				}
		}

		for (uint32_t tick_Idx = 0; tick_Idx < nTicks; ++tick_Idx)
		{
			// Simulate the population
			for (auto &car_agent : trainingAgents)
			{
				if (car_agent.isDead)
					continue;

				car_agent.Simulate();

				std::vector<uint32_t> dummy = {0, 1, 2, 3};
				physicsEngine.StepSimulation(stepTime, dummy);

				if (!Config::get().headless)
				{
					raceNetRenderer.Render(tick_Idx, trainingAgents, training_track);
				}
				if (glfwWindowShouldClose(m_window.get()))
					break;
			}
		}

		int localMaxFitness = 0;
		for (auto &carAgent : trainingAgents)
		{
			if (carAgent.fitness > localMaxFitness)
			{
				localMaxFitness = carAgent.fitness;
			}
		}

		size_t winnerIdx;
		for (size_t i = 0; i < trainingAgents.size(); i++)
			if (trainingAgents[i].IsWinner())
			{
				haveWinner = true;
				winnerIdx  = i;
			}

		if (haveWinner)
		{
			LOG(INFO) << "WINNER: Saving best agent network to " << BEST_NETWORK_PATH;
			trainingAgents[winnerIdx].raceNet.export_tofile(BEST_NETWORK_PATH);
		}
		// Display the fitnesses
		// LOG(INFO) << "Generation: " << pool.generation() << " Specie number: " << specieCounter
		//          << " Genomes in specie: " << carAgents.size() << " Global max fitness: " << globalMaxFitness
		//          << " Current specie max: " << localMaxFitness;
		LOG(INFO) << gen_Idx << ", " << localMaxFitness << ", ";
	}
}
