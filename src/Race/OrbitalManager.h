#pragma once

#include "../Camera/BaseCamera.h"
#include "../Scene/GlobalLight.h"

namespace OpenNFS {
    class OrbitalManager {
      public:
        explicit OrbitalManager();
        void Update(BaseCamera const &camera, float timeScaleFactor) const;
        [[nodiscard]] GlobalLight *GetActiveGlobalLight() const;

        static constexpr float SKYDOME_RADIUS{200.f};
      private:
        std::unique_ptr<GlobalLight> m_sun;
        std::unique_ptr<GlobalLight> m_moon;
    };
} // namespace OpenNFS
