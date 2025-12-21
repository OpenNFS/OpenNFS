#include "Config.h"

#include <argparse/argparse.hpp>

namespace OpenNFS {
    void Config::ParseFile(std::ifstream &inStream) {
        CHECK_F(false, "Config load from file not implemented! (yet)");
    }

    void Config::InitFromCommandLine(int const argc, char **argv) {
        argparse::ArgumentParser program("OpenNFS", ONFS_VERSION, argparse::default_arguments::help);

        // Option name/short name, description, bound variable for parameter
        program.add_argument("--vulkan")
            .help("Use the Vulkan renderer instead of GL default")
            .flag()
            .store_into(vulkanRender);
        program.add_argument("--headless").help("Launch ONFS without a window").flag().store_into(headless);
        program.add_argument("--nracers", "-r")
            .help("Number of AI Racers to spawn")
            .default_value(DEFAULT_NUM_RACERS)
            .store_into(nRacers);
        program.add_argument("--car", "-c").help("Name of desired car").default_value(DEFAULT_CAR).store_into(car);
        program.add_argument("--carv", "-cv")
            .help("NFS Version containing desired car (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5)")
            .default_value(DEFAULT_CAR_NFS_VER)
            .store_into(carTag);
        program.add_argument("--track", "-t")
            .help("Name of desired track")
            .default_value(DEFAULT_TRACK)
            .store_into(track);
        program.add_argument("--trackv", "-tv")
            .help("NFS Version containing desired track (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5)")
            .default_value(DEFAULT_TRACK_NFS_VER)
            .store_into(trackTag);
        program.add_argument("--resX", "-x")
            .help("Horizontal screen resolution")
            .default_value(DEFAULT_X_RESOLUTION)
            .store_into(resX);
        program.add_argument("--resY", "-y")
            .help("Vertical screen resolution")
            .default_value(DEFAULT_Y_RESOLUTION)
            .store_into(resY);
        program.add_argument("--fixup-asset-paths")
            .help("Rename all available NFS files and folders to lowercase so can be consistent for ONFS read on Unix "
                  "systems")
            .flag()
            .store_into(renameAssets);

        try {
            program.parse_args(argc, argv);
        } catch (std::exception const &err) {
            LOG(WARNING) << err.what() << std::endl;
            LOG(WARNING) << program;
            std::exit(1);
        }
    }
} // namespace OpenNFS
