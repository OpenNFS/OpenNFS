#include "Config.h"

void option_dependency(const variables_map &vm, const char *for_what, const char *required_option) {
    if (vm.count(for_what) && !vm[for_what].defaulted())
        if (vm.count(required_option) == 0 || vm[required_option].defaulted())
            throw std::logic_error(std::string("Option '") + for_what + "' requires option '" + required_option + "'.");
}

void Config::ParseFile(std::ifstream &inStream) {
    ASSERT(false, "Config load from file not implemented! (yet)");
}


void Config::InitFromCommandLine(int argc, char **argv) {
    try {
        options_description desc("Allowed options");
        desc.add_options()
                // Option name/short name, parameter to option, description
                ("help,h", "Print OpenNFS command-line parameters")
                ("vulkan", bool_switch(&vulkanRender), "Use the Vulkan renderer instead of GL default")
                ("headless", bool_switch(&headless), "Launch ONFS without a window")
                ("train", bool_switch(&trainingMode), "Launch ONFS in AI training mode")
                ("fullv", bool_switch(&useFullVroad), "Allow AI to drive whole track")
                ("nracers", value(&nRacers), "Number of AI Racers on track")
                ("popsize", value(&populationSize), "Number of AI agents to place in a GA generation (training mode)")
                ("ngens", value(&nGenerations), "Number of generations to allow AI to develop for (training mode)")
                ("nticks", value(&nTicks), "Number of ticks to allow AI agents to simulate in, per generation (training mode)")
                ("car,c", value(&car), "Name of desired car")
                ("track,t", value(&track), "Name of desired track")
                ("resX,x", value<uint32_t>(&resX), "Horizontal screen resolution")
                ("resY,y", value<uint32_t>(&resY), "Vertical screen resolution");

        store(parse_command_line(argc, argv, desc), storedConfig);
        notify(storedConfig);

        if (storedConfig.count("help")) {
            std::cout << desc
                      << "\n";  // Avoid LOG calls right now as a) Not initialised yet, b) Async nature makes printout of help vars look weird
            std::terminate();
        }

        option_dependency(storedConfig, "train", "popsize");
        option_dependency(storedConfig, "train", "ngens");
        option_dependency(storedConfig, "train", "nticks");
    }
    catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        std::terminate();
    }
}