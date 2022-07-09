#include "OrbitalManager.h"

OrbitalManager::OrbitalManager() :
    m_sun(std::make_shared<GlobalLight>(glm::vec3(0, 0, 0), glm::vec3(0, SKYDOME_RADIUS, 0))),
    m_moon(std::make_shared<GlobalLight>(glm::vec3(0, 0, 0), glm::vec3(0, -SKYDOME_RADIUS, 0))) {
}

void OrbitalManager::Update(const BaseCamera &camera, float timeScaleFactor) {
    m_sun->ChangeTarget(camera.position);
    m_sun->Update(timeScaleFactor);

    m_moon->ChangeTarget(camera.position);
    m_moon->Update(timeScaleFactor);
}

std::shared_ptr<GlobalLight> OrbitalManager::GetActiveGlobalLight() {
    // TODO: Switch this over to be time based
    if (m_sun->position.y <= 0) {
        return m_moon;
    }

    return m_sun;
}
