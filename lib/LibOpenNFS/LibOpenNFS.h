#pragma once

#include <sstream>

namespace LibOpenNFS {
    const std::string ASSET_PATH = "./assets/";
    const std::string CAR_PATH   = ASSET_PATH + "car/";
    const std::string TRACK_PATH = ASSET_PATH + "tracks/";

    std::ostream logger();

    /*void RegisterLogger(std::ostream &_logger) {
        logger = std::move(_logger);
    }*/
} // namespace LibOpenNFS