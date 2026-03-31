#pragma once
#include "Component.h"

class TransformComponent : public Component
{
public:
    Vector2 position;
    Vector2 scale;

    TransformComponent(Vector2 pos = Vector2(0.f, 0.f), Vector2 s = Vector2(0.1f, 0.1f))
        : position(pos), scale(s) {}
};
