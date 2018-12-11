//
// Created by amrik on 11/12/18.
//

#include "Config.h"


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
                ("train", bool_switch(&trainingMode), "Launch ONFS in AI training mode")
                ("car,c", value(&car), "Name of desired car")
                ("track,t", value(&track), "Name of desired track")
                ("resX,x", value<uint32_t>(&resX), "Horizontal screen resolution")
                ("resY,y", value<uint32_t>(&resY), "Vertical screen resolution");

        store(parse_command_line(argc, argv, desc), storedConfig);
        notify(storedConfig);

        if (storedConfig.count("help")) {
            std::cout << desc << "\n";  // Avoid LOG calls right now as a) Not initialised yet, b) Async nature makes printout of help vars look weird
            std::terminate();
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << "\n";
        std::terminate();
    }
}