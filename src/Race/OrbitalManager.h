#pragma once

#include <Common/Lights/GlobalLight.h>

#include "../Camera/BaseCamera.h"

const float SKYDOME_RADIUS = 200.f;

class OrbitalManager {
public:
    explicit OrbitalManager();
    void Update(const BaseCamera &camera, float timeScaleFactor);
    std::shared_ptr<GlobalLight> GetActiveGlobalLight();

private:
    std::shared_ptr<GlobalLight> m_sun;
    std::shared_ptr<GlobalLight> m_moon;
};
