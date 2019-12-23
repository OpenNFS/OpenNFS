#pragma once
struct AABB;

class IAABB
{
public:
    virtual ~IAABB() = default;
    virtual AABB getAABB() const = 0;
};
