#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace OpenNFS {
    // Forward declarations
    class Car;
    class CarAgent;

    // Surface categories for visual differentiation
    enum class SurfaceCategory : uint8_t {
        ASPHALT = 0,
        DIRT = 1,
        GRASS = 2,
        GRAVEL = 3,
        SNOW = 4,
        SAND = 5,
        UNKNOWN = 6
    };

    // A single segment of the trail ribbon
    struct TrailSegment {
        glm::vec3 leftPosition{0.0f};  // Left edge of ribbon
        glm::vec3 rightPosition{0.0f}; // Right edge of ribbon
        glm::vec3 normal{0.0f, 1.0f, 0.0f};
        float age{0.0f};       // Time since creation (seconds)
        float intensity{0.0f}; // Skid intensity (0-1)
        SurfaceCategory surface{SurfaceCategory::UNKNOWN};
        bool valid{false}; // Is this segment active?
    };

    // Trail for a single wheel
    struct WheelTrail {
        static constexpr size_t MAX_SEGMENTS = 256;

        std::array<TrailSegment, MAX_SEGMENTS> segments{};
        size_t headIndex{0}; // Points to oldest segment (for ring buffer)
        size_t count{0};     // Number of valid segments
        bool wasInContact{false};
        glm::vec3 lastPosition{0.0f};

        void AddSegment(TrailSegment const &segment);
        void Clear();
    };

    // All trails for a single vehicle
    struct VehicleTrails {
        std::array<WheelTrail, 4> wheelTrails;
        uint32_t vehicleID{0};
    };

    class SkidMarkSystem {
      public:
        SkidMarkSystem();

        // Update the system - call after physics step
        void Update(float deltaTime, std::vector<std::shared_ptr<CarAgent>> const &racers);

        // Get all trails for rendering
        [[nodiscard]] std::vector<VehicleTrails> const &GetAllTrails() const {
            return m_vehicleTrails;
        }

        // Configuration
        [[nodiscard]] float GetLifetime() const {
            return m_lifetime;
        }
        [[nodiscard]] float GetFadeStart() const {
            return m_fadeStart;
        }
        void SetLifetime(float const lifetime) {
            m_lifetime = lifetime;
        }
        void SetFadeStart(float const fadeStart) {
            m_fadeStart = fadeStart;
        }

        // Clear all trails
        void Clear();

      private:
        // Map NFS4 surface type ID to our surface category
        static SurfaceCategory MapSurfaceType(int nfsSurfaceType);

        // Check if a wheel should emit a skid mark
        [[nodiscard]] bool ShouldEmitSkidMark(std::shared_ptr<Car> const &car, int wheelIndex) const;

        // Get the intensity of the skid mark (0-1)
        static float CalculateSkidIntensity(std::shared_ptr<Car> const &car, int wheelIndex);

        // Process a single vehicle
        void ProcessVehicle(float deltaTime, std::shared_ptr<CarAgent> const &racer, size_t vehicleIndex);

        // Age and cull old segments
        void AgeSegments(float deltaTime);

        std::vector<VehicleTrails> m_vehicleTrails;

        // Configuration parameters
        float m_lifetime{15.0f};           // Total trail lifetime in seconds
        float m_fadeStart{0.3f};           // When fade begins (fraction of lifetime)
        float m_minSegmentDistance{0.05f}; // Minimum distance between segments (meters)
        float m_gripThreshold{0.85f};      // Grip below this triggers skid marks
        float m_tireHalfWidthMult{2.f};    // Multiplier of tire width for ribbon edges
        float m_heightOffset{0.005f};      // Height above ground to prevent z-fighting
    };
} // namespace OpenNFS
