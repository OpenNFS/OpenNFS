#include "Config.h"

#include <Common/Utils.h>

void Config::ParseFile(std::ifstream &inStream) {
    ASSERT(false, "Config load from file not implemented! (yet)");
}

void Config::InitFromCommandLine(int argc, char **argv) {
    // TODO: Add proper parsing later, without Boost
    // Option name/short name, parameter to option, description
    /*("help,h", "Print OpenNFS command-line parameters")("spark", bool_switch(&sparkMode), "Ignore Virual Road boundaries")(
      "vulkan", bool_switch(&vulkanRender), "Use the Vulkan renderer instead of GL default")("headless", bool_switch(&headless), "Launch ONFS without a window")(
      "carv,cv", value(&carTag), "NFS Version containing desired car (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5")("track,t", value(&track), "Name of desired track")(
      "trackv,tv", value(&trackTag), "NFS Version containing desired track (NFS_2, NFS_3, NFS_3_PS1, NFS_4, NFS_4_PS1, NFS_5")(
      "resX,x", value<uint32_t>(&resX), "Horizontal screen resolution")("resY,y", value<uint32_t>(&resY), "Vertical screen resolution")(
      "fixup-asset-paths", bool_switch(&renameAssets), "Rename all available NFS files and folders to lowercase so can be consistent for ONFS read");*/
    return;
}