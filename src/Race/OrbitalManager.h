#pragma once

#include "../Camera/BaseCamera.h"
#include "../Scene/GlobalLight.h"

namespace OpenNFS {
    class OrbitalManager {
    public:
        explicit OrbitalManager();

        void Update(const BaseCamera &camera, float timeScaleFactor) const;

        std::shared_ptr<GlobalLight> GetActiveGlobalLight();

    private:
        const float SKYDOME_RADIUS = 200.f;

        std::shared_ptr<GlobalLight> m_sun;
        std::shared_ptr<GlobalLight> m_moon;
    };
}
