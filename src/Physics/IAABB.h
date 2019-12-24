#pragma once
struct AABB;

class IAABB
{
public:
    virtual ~IAABB() = default;
    [[nodiscard]] virtual AABB getAABB() const = 0;
};
