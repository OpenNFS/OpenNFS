#include "Config.h"

#define yeet throw

void option_dependency(const variables_map &vm, const char *for_what, const char *required_option)
{
    if (vm.count(for_what) && !vm[for_what].defaulted())
        if (vm.count(required_option) == 0 || vm[required_option].defaulted())
            yeet std::logic_error(std::string("Option '") + for_what + "' requires option '" + required_option + "'.");
}

void Config::ParseFile(std::ifstream &inStream)
{
    ASSERT(false, "Config load from file not implemented! (yet)");
}

void Config::InitFromCommandLine(int argc, char **argv)
{
    try
    {
        options_description desc("Allowed options");
        desc.add_options()
          // Option name/short name, parameter to option, description
          ("help,h", "Print OpenNFS command-line parameters")("spark", bool_switch(&sparkMode), "Ignore Virual Road boundaries")(
            "vulkan", bool_switch(&vulkanRender), "Use the Vulkan renderer instead of GL default")("headless", bool_switch(&headless), "Launch ONFS without a window")(
            "train", bool_switch(&trainingMode), "Launch ONFS in AI training mode")("fullv", bool_switch(&useFullVroad), "Allow AI to drive whole track")(
            "nracers", value(&nRacers), "Number of AI Racers on track")("ngens", value(&nGenerations), "Number of generations to allow AI to develop for (training mode)")(
            "nticks", value(&nTicks), "Number of ticks to allow AI agents to simulate in, per generation (training mode)")("car,c", value(&car), "Name of desired car")(
            "carv,cv", value(&carTag), "NFS Version containing desired car (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5")("track,t", value(&track), "Name of desired track")(
            "trackv,tv", value(&trackTag), "NFS Version containing desired track (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5")(
            "resX,x", value<uint32_t>(&resX), "Horizontal screen resolution")("resY,y", value<uint32_t>(&resY), "Vertical screen resolution");

        store(parse_command_line(argc, argv, desc), storedConfig);
        notify(storedConfig);

        if (storedConfig.count("help"))
        {
            std::cout << desc << "\n"; // Avoid LOG calls right now as a) Not initialised yet, b) Async nature makes printout of help vars look weird
            std::terminate();
        }

        option_dependency(storedConfig, "train", "ngens");
        option_dependency(storedConfig, "train", "nticks");
        option_dependency(storedConfig, "car", "carv");
        option_dependency(storedConfig, "track", "trackv");
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << "\n";
        std::terminate();
    }
}