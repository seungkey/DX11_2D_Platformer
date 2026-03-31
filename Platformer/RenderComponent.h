#pragma once
#include "Component.h"

class RenderComponent : public Component
{
public:
    bool visible;

    RenderComponent(bool v = true) : visible(v) {}
};
