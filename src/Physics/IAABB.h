#pragma once

class AABB;

class IAABB {
public:
    virtual ~IAABB()             = default;
    [[nodiscard]] virtual AABB GetAABB() const = 0;
};
