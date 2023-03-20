#define TINYOBJLOADER_IMPLEMENTATION

#include "gtest/gtest.h"
#include "../src/Util/Logger.h"
#include "../src/Util/Utils.h"

#include <filesystem>
#include <map>

class UtilTest : public testing::Test {
public:
    virtual void SetUp() {
        // Initialise Logging
        auto logger = std::make_shared<Logger>();
    }

    virtual void TearDown() {
    }
};

// Test VIV files are extracted successfully, by comparing against known good data
TEST_F(UtilTest, ExtractVIV) {
    using namespace std::filesystem;

    const std::string vivPath    = "../resources/NFS_3/gamedata/carmodel/diab/car.viv";
    const std::string targetPath = "./test/assets/car/NFS_3/diab/";
    std::map<std::string, int> expectedVivContents{{"car.bnk", 217136}, {"car.fce", 152948}, {"car00.tga", 262188},  {"carp.txt", 5023},  {"dash.qfs", 516356}, {"fedata.bri", 687},
                                                   {"fedata.eng", 689}, {"fedata.fre", 630}, {"fedata.fsh", 131112}, {"fedata.ger", 683}, {"fedata.ita", 619},  {"fedata.spa", 692},
                                                   {"fedata.swe", 685}, {"ocar.bnk", 69528}, {"ocard.bnk", 69544},   {"scar.bnk", 152288}};

    // Attempt to extract the VIV file
    Utils::ExtractVIV(vivPath, targetPath);

    // Check the number of files extracted matches that which is expected
    const path vivExtractPath(targetPath);

    for (directory_iterator itr(vivExtractPath); itr != directory_iterator(); ++itr) {
        std::string vivFileName = itr->path().filename().string();
        // Ensure the file is present and filesize is correct
        ASSERT_EQ(expectedVivContents.count(vivFileName), 1) << "Extracted viv file " << vivFileName << " not expected";
        ASSERT_EQ(expectedVivContents[vivFileName], file_size(itr->path().string())) << "Extracted viv file " << vivFileName << " did not have expected file size";
    }
}

// Test car moves in physics world when acceleration applied for severl physics ticks
/*
TEST_F(UtilTest, CarMoveTest){
    // Get physics Engine
    PhysicsEngine physicsEngine;

    // Get car and track to test on
    AssetData loadedAssets = {
            NFS_3, Config::get().car,
            NFS_3, Config::get().track
    };

    //Load Track Data
    std::shared_ptr<Track> track = TrackLoader::LoadTrack(loadedAssets.trackTag, loadedAssets.track);
    //Load Car data from unpacked NFS files
    std::shared_ptr<Car> car = CarLoader::LoadCar(loadedAssets.carTag, loadedAssets.car);

    CarAgent racer("Test", BEST_NETWORK_PATH, car, track);
    physicsEngine.registerVehicle(racer.car);
    physicsEngine.registerTrack(track);
    racer.resetToVroad(0, 0, 0.0f, track, racer.car);

    glm::vec3 initialCarPosition = racer.car->carBodyModel.position;

    // Tick over the physics
    for(uint8_t tickIdx = 0; tickIdx < 5; ++tickIdx){
        racer.car->applyAccelerationForce(true, false);
        physicsEngine.stepSimulation(1 / 60.f);
    }

    // Ensure car has moved from initial position by pythagorean distance
    ASSERT_TRUE(glm::distance(initialCarPosition, racer.car->carBodyModel.position) > 2.f) << "Car has not moved when "
                                                                                              "it should have!";
}

// Check every ONFS shader compiles
TEST_F(UtilTest, ShaderCompileTest){
    // Used to store length of OpenGL shader compile log
    int InfoLogLength = 0;
    TrackShader trackShader;
    glGetShaderiv(trackShader.shaders.mShaders[0].handle, GL_INFO_LOG_LENGTH, &InfoLogLength);
    // Retrieve length of information log given whilst compiling. If less than 1, no error
    ASSERT_TRUE(InfoLogLength < 1);

    CarShader carShader;
    glGetShaderiv(carShader.shaders.mShaders[0].handle, GL_INFO_LOG_LENGTH, &InfoLogLength);
    ASSERT_TRUE(InfoLogLength < 1);

    DepthShader depthShader;
    glGetShaderiv(depthShader.shaders.mShaders[0].handle, GL_INFO_LOG_LENGTH, &InfoLogLength);
    ASSERT_TRUE(InfoLogLength < 1);

    RaceNetShader raceNetShader;
    glGetShaderiv(raceNetShader.shaders.mShaders[0].handle, GL_INFO_LOG_LENGTH, &InfoLogLength);
    ASSERT_TRUE(InfoLogLength < 1);
}

// Check the raycasts return valid distances once ticked in the physics world
TEST_F(UtilTest, RaycastTest){
    // Get physics Engine
    PhysicsEngine physicsEngine;

    // Get car and track to test on
    AssetData loadedAssets = {
            NFS_3, Config::get().car,
            NFS_3, Config::get().track
    };

    //Load Track Data
    std::shared_ptr<Track> track = TrackLoader::LoadTrack(loadedAssets.trackTag, loadedAssets.track);
    //Load Car data from unpacked NFS files
    std::shared_ptr<Car> car = CarLoader::LoadCar(loadedAssets.carTag, loadedAssets.car);

    CarAgent racer("Test", BEST_NETWORK_PATH, car, track);
    physicsEngine.registerVehicle(racer.car);
    physicsEngine.registerTrack(track);
    racer.resetToVroad(0, 0, 0.0f, track, racer.car);

    // Step physics engine once to generate raycasts
    physicsEngine.stepSimulation(1 / 60.f);

    // Tick over the physics
    for(uint8_t rayIdx = 0; rayIdx < 5; ++rayIdx){
        ASSERT_TRUE(racer.car->rangefinders[rayIdx] > 0) << "Ray " << rayIdx << " not registering valid distance";
    }
}

// Ensure a log file has been successfully written by the g3Log logger instance
TEST_F(UtilTest, LogTest){
    using namespace boost::filesystem;

    // Check for log file in current path
    const path logLocation("./");

    for (directory_iterator itr(logLocation); itr != directory_iterator(); ++itr) {
        std::string potentialLogFileName = itr->path().filename().string();
        if(potentialLogFileName.find(".log") != std::string::npos){
            // Ensure the filesize is correct (above 0, logs stored within)
            ASSERT_TRUE(file_size(itr->path().string() > 0)) << "Log file " << potentialLogFileName << " did not have expected file size.";
        }
    }
}

// Ensure the training ground exits after a set number of generations and ticks
TEST_F(UtilTest, TrainingGroundTest){
    // Get car and track to test on
    AssetData trainingAssets = {
            NFS_3, Config::get().car,
            NFS_3, Config::get().track
    };

    //Load Track Data
    std::shared_ptr<Track> track = TrackLoader::LoadTrack(trainingAssets.trackTag, trainingAssets.track);
    //Load Car data from unpacked NFS files
    std::shared_ptr<Car> car = CarLoader::LoadCar(trainingAssets.carTag, trainingAssets.car);

    // Override number of generations in config
    Config::get().nGenerations = 1;
    Config::get().nTicks = 1;
    auto trainingGround = TrainingGround(Config::get().nGenerations,Config::get().nTicks, track, car, logger, window);

    ASSERT_TRUE(true) << "ONFS TrainingGround did not exit!";
}

// Step function for below matrix network test
double stepFunction(double x){
    if(x>0.5){
        return 1.0;
    }
    if(x < 0.5){
        return 0.0;
    }
    return x;
}

// Check the Matrix neural network library is functioning properly
TEST_F(UtilTest, MatrixNetTest){
    // Set neuron configuration for matrix neural network
    std::vector<int> neuronConfiguration = {3, 10, 3};
    std::vector<int> raycastInputs = {10, 10, 10};

    // Build matrix neural network
    Network testNetwork(neuronConfiguration, 1.f);

    // Run inference
    Matrix<double> resultMatrix = testNetwork.computeOutput(raycastInputs).applyFunction(stepFunction);

    // Ensure matrix output either high or low
    for(uint8_t outputIdx = 0; outputIdx < 3; ++outputIdx){
        ASSERT_TRUE(resultMatrix.get(0, outputIdx) > 0.5 || resultMatrix.get(0, outputIdx) < 0.5);
    }
}

TEST_F(UtilTest, AgentResetTest){
    // Get physics Engine
    PhysicsEngine physicsEngine;

    // Get car and track to test on
    AssetData loadedAssets = {
            NFS_3, Config::get().car,
            NFS_3, Config::get().track
    };

    //Load Track Data
    std::shared_ptr<Track> track = TrackLoader::LoadTrack(loadedAssets.trackTag, loadedAssets.track);
    //Load Car data from unpacked NFS files
    std::shared_ptr<Car> car = CarLoader::LoadCar(loadedAssets.carTag, loadedAssets.car);

    CarAgent racer("Test", BEST_NETWORK_PATH, car, track);
    physicsEngine.registerVehicle(racer.car);
    physicsEngine.registerTrack(track);
    racer.resetToVroad(0, 0, 0.0f, track, racer.car);

    // Step physics engine once to reset car
    physicsEngine.stepSimulation(1 / 60.f);

    // Check car is on VROAD 0 as told to reset there
    ASSERT_TRUE(CarAgent::getClosestVroad(racer.car, track) == 0);

    // Tick over the physics
    for(uint8_t vroadIdx = 0; vroadIdx < 5; ++vroadIdx){
        racer.resetToVroad(vroadIdx, 0, 0.0f, track, racer.car);
        physicsEngine.stepSimulation(1 / 60.f);
        ASSERT_TRUE(CarAgent::getClosestVroad(racer.car, track) == vroadIdx) << "Car agent did not reset to vroad " <<
        vroadIdx;
    }
}

TEST_F(UtilTest, RaceSessionTest){
    // Get car and track to test on
    AssetData trainingAssets = {
            NFS_3, Config::get().car,
            NFS_3, Config::get().track
    };

    //Load Track Data
    std::shared_ptr<Track> track = TrackLoader::LoadTrack(trainingAssets.trackTag, trainingAssets.track);
    //Load Car data from unpacked NFS files
    std::shared_ptr<Car> car = CarLoader::LoadCar(trainingAssets.carTag, trainingAssets.car);

    // Spawn a race session
    RaceSession race(window, logger, nullptr, track, car);

    // Ensure the loaded assets are correct
    ASSERT_TRUE(race.car.carID == trainingAssets.carTag && race.track.name == trainingAssets.track) << "ONFS RaceSession"
                                                                                                       "loaded wrong assets!";
}*/
