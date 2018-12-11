//
// Created by amrik on 11/12/18.
//

#include "Config.h"

// TODO: This is okay... but I should try to use Boost program_options with variables_map as backing storage, avoid expensive strcmp on param check in 'hot' code
std::map<std::string, std::string> Config::ParseCommandLineArgs(std::vector<std::string> args) {
    std::map<std::string, std::string> parameters;

    // Parse key val pairs from command line args
    for (auto &paramValPair : args) {
        size_t delimiterPos = paramValPair.find('=');
        ASSERT(delimiterPos != std::string::npos, "Invalid option on command line (missing '='): " << paramValPair);
        std::string parameter = paramValPair.substr(0, delimiterPos);
        ASSERT(parameter.size(), "Invalid option on command line (missing key or value around '='): " << paramValPair);
        std::string value = paramValPair.substr(delimiterPos + 1, paramValPair.length());
        parameters[parameter] = value;
    }

    return parameters;
}


void Config::ParseFile(std::ifstream &inStream) {
    ASSERT(false, "Config load from file not implemented! (yet)");
}


void Config::InitFromCommandLine(int argc, char **argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    storedConfig = ParseCommandLineArgs(args);
}

template<>
std::string Config::getValue(std::string key){
    return storedConfig[key];
}

template<>
int Config::getValue(std::string key){
    return stoi(storedConfig[key]);
}
