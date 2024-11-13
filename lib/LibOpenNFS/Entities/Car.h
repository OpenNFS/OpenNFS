#pragma once

#include <glm/glm.hpp>
#include <string>

#include "CarGeometry.h"
#include "Common/NFSVersion.h"

namespace LibOpenNFS {
    class Car {
    public:
        class Dummy {
        public:
            std::string name;
            glm::vec3 position;
            Dummy(const char* dummyName, const glm::vec3 position) {
                this->name     = std::string(dummyName);
                this->position = position;
            }
        };

        class Colour {
        public:
            std::string colourName;
            glm::vec3 colour;
            Colour(const std::string& colourName, const glm::vec3 colour) {
                this->colourName = colourName;
                this->colour     = colour;
            }
        };

        class MetaData {
        public:
            explicit MetaData() = default;
            std::string name    = "Unset";
            std::vector<Dummy> dummies;
            std::vector<Colour> colours;
            std::vector<CarGeometry> meshes;
        };

        enum class ModelIndex : uint8_t { LEFT_FRONT_WHEEL = 0, RIGHT_FRONT_WHEEL, LEFT_REAR_WHEEL, RIGHT_REAR_WHEEL, CAR_BODY };

        explicit Car(const MetaData& carData, NFSVersion nfsVersion, const std::string& carID) : metadata(carData), tag(nfsVersion), id(carID){};
        Car(const MetaData& carData, NFSVersion nfsVersion, const std::string& carID, bool _multi_textured) : Car(carData, nfsVersion, carID) {
            isMultitextured = _multi_textured;
        };

        std::string id;
        NFSVersion tag;
        MetaData metadata;
        bool isMultitextured{false};
    };
} // namespace LibOpenNFS
