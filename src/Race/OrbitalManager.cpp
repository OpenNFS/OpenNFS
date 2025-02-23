#include "OrbitalManager.h"

namespace OpenNFS {
    OrbitalManager::OrbitalManager()
        : m_sun(std::make_unique<GlobalLight>(glm::vec3(0, 0, 0), glm::vec3(0, SKYDOME_RADIUS, 0))),
          m_moon(std::make_unique<GlobalLight>(glm::vec3(0, 0, 0), glm::vec3(0, -SKYDOME_RADIUS, 0))) {
    }

    void OrbitalManager::Update(BaseCamera const &camera, float const timeScaleFactor) const {
        m_sun->ChangeTarget(camera.position);
        m_sun->Update(timeScaleFactor);

        m_moon->ChangeTarget(camera.position);
        m_moon->Update(timeScaleFactor);
    }

    GlobalLight *OrbitalManager::GetActiveGlobalLight() const {
        // TODO: Switch this over to be time based
        if (m_sun->position.y <= 0) {
            return m_moon.get();
        }

        return m_sun.get();
    }
} // namespace OpenNFS
