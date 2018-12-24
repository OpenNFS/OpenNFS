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

    InitialiseAgents(populationSize);
    std::vector<std::vector<int>> trainedAgentFitness = TrainAgents(nGenerations, nTicks);

    LOG(INFO) << "Saving best agent network to " << BEST_NETWORK_PATH;
    car_agents[trainedAgentFitness[0][0]]->carNet.net.saveNetworkParams(BEST_NETWORK_PATH.c_str());

    LOG(INFO) << "Done";
}

void TrainingGround::Mutate(RaceNet &toMutate) {
    for (uint8_t mut_Idx = 0; mut_Idx < 5; ++mut_Idx) {
        unsigned long layerToMutate = rand() % toMutate.net.W.size();

        auto &m = toMutate.net.W[layerToMutate];

        int h = m.getHeight();
        int w = m.getWidth();

        int xNeuronToMutate = rand() % h;
        int yNeuronToMutate = rand() % w;

        m.put(xNeuronToMutate, yNeuronToMutate, m.get(xNeuronToMutate, yNeuronToMutate) * Utils::RandomFloat(0.5, 1.5));
    }
}

// Move this to agent class?
float TrainingGround::EvaluateFitness(shared_ptr<Car> &car_agent) {
    uint32_t nVroad = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(training_track->trackData)->col.vroadHead.nrec;

    int closestVroadID = 0;
    float lowestDistanceSqr = FLT_MAX;
    for (uint32_t vroad_Idx = 0; vroad_Idx < nVroad; ++vroad_Idx) {
        INTPT refPt = boost::get<shared_ptr<NFS3_4_DATA::TRACK>>(training_track->trackData)->col.vroad[vroad_Idx].refPt;
        glm::vec3 vroadPoint = glm::normalize(glm::quat(glm::vec3(-SIMD_PI / 2, 0, 0))) *
                               glm::vec3((refPt.x / 65536.0f) / 10.f, ((refPt.y / 65536.0f) / 10.f),
                                         (refPt.z / 65536.0f) / 10.f);

        float distanceSqr = glm::length2(glm::distance(car_agent->car_body_model.position, vroadPoint));
        if (distanceSqr < lowestDistanceSqr) {
            closestVroadID = vroad_Idx;
            lowestDistanceSqr = distanceSqr;
        }
    }

    // Return a number corresponding to the distance driven
    return (float) closestVroadID;
}

void TrainingGround::InitialiseAgents(uint16_t populationSize) {
    // Create new cars from models loaded in training_car to avoid VIV extract again, each with new RaceNetworks
    for (uint16_t pop_Idx = 0; pop_Idx < populationSize; ++pop_Idx) {
        shared_ptr<Car> car_agent = std::make_shared<Car>(pop_Idx, this->training_car->all_models, NFS_3, "diab", RaceNet());
        car_agent->colour = glm::vec3(Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f), Utils::RandomFloat(0.f, 1.f));
        physicsEngine.registerVehicle(car_agent);
        Renderer::ResetToVroad(1, training_track, car_agent);
        car_agents.emplace_back(car_agent);
    }

    LOG(INFO) << "Agents initialised";
}

void
TrainingGround::SelectAgents(std::vector<shared_ptr<Car>> &car_agents, std::vector<std::vector<int>> agent_fitnesses) {
    // Clone the best network into the worst 50%
    for (uint32_t cull_Idx = agent_fitnesses.size() / 2; cull_Idx < agent_fitnesses.size(); ++cull_Idx) {
        for (auto &car_agent : car_agents) {
            if (car_agent->populationID == agent_fitnesses[cull_Idx][0]) {
                car_agent->carNet = car_agents[agent_fitnesses[0][0]]->carNet;
            }
        }
    }
}

void TrainingGround::Crossover(RaceNet &a, RaceNet &b) {
    // TODO: Actually implement this
}

std::vector<std::vector<int>> TrainingGround::TrainAgents(uint16_t nGenerations, uint32_t nTicks) {
    std::vector<std::vector<int>> agentFitnesses;
    // TODO: Remove this and
    std::vector<int> dummyAgentData = {0, 0};
    agentFitnesses.emplace_back(dummyAgentData);

    for (uint16_t gen_Idx = 0; gen_Idx < nGenerations; ++gen_Idx) {
        LOG(INFO) << "Beginning Generation " << gen_Idx;

        // Simulate the population
        for (uint32_t tick_Idx = 0; tick_Idx < nTicks; ++tick_Idx) {
            for (auto &car_agent : car_agents) {
                car_agent->simulate();
            }
            physicsEngine.stepSimulation(stepTime);
            raceNetRenderer.Render(tick_Idx, car_agents, training_track);
            if (glfwWindowShouldClose(window)) return agentFitnesses;
        }

        // Clear fitness data for next generation
        agentFitnesses.clear();

        // Evaluate the fitnesses and sort them
        for (auto &car_agent : car_agents) {
            LOG(INFO) << "Agent " << car_agent->populationID << " made it to trkblock " << EvaluateFitness(car_agent);
            std::vector<int> agentData = {car_agent->populationID, (int) EvaluateFitness(car_agent)};
            agentFitnesses.emplace_back(agentData);
        }

        std::sort(agentFitnesses.begin(), agentFitnesses.end(),
                  [](const std::vector<int> &a, const std::vector<int> &b) {
                      return a[1] > b[1];
                  });


        LOG(DEBUG) << "Agent " << agentFitnesses[0][0] << " was fittest";

        // Mutate the fittest network
        // Mutate(car_agents[agentFitnesses[0][0]]->carNet);

        // Perform selection
        SelectAgents(car_agents, agentFitnesses);

        // Mutate all networks
        for (auto &car_agent : car_agents) {
            Mutate(car_agent->carNet);
        }


        // Reset the cars for the next generation
        for (auto &car_agent : car_agents) {
            Renderer::ResetToVroad(1, training_track, car_agent);
        }
    }

    return agentFitnesses;
}
