#pragma once
#include "Component.h"
enum class CollisionType
{
    BLOCK,
    UNBLOCK,
    ONEWAY,
};

class BoxColliderComponent : public Component
{
public:
    Vector2 size;
    Vector2 offset;
    CollisionType type;

    Vector2 min;
    Vector2 max;

    bool isColliding = false;

    BoxColliderComponent(Vector2 s = Vector2(1.0f, 1.0f), Vector2 off = Vector2(0.f, 0.f))
        : size(s), offset(off), type(CollisionType::BLOCK) {}

    void Update(Entity& owner) override;
    void UpdateBounds(const Vector2& position)
    {
        Vector2 center = position + offset;
        min = center - (size * 0.5f);
        max = center + (size * 0.5f);
    }
};
