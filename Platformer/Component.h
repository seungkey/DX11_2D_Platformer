#pragma once
#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;

class Entity;

// Base Component class
class Component
{
public:
    virtual ~Component() {}
    virtual void Update(Entity& owner) { (void)owner; }

};
