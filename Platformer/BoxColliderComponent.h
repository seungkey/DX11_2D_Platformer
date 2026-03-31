#pragma once
#include "Component.h"

class BoxColliderComponent : public Component
{
public:
    Vector2 size;
    Vector2 offset;

    BoxColliderComponent(Vector2 s = Vector2(0.1f, 0.1f), Vector2 off = Vector2(0.f, 0.f))
        : size(s), offset(off) {}
};
