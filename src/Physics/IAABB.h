#pragma once

class AABB;

class IAABB {
public:
    virtual ~IAABB()             = default;
    virtual AABB GetAABB() const = 0;
};
