#pragma once
#include "BoxColliderComponent.h"
#include <vector>
#include <memory>

class Entity;

class CollisionManager
{
public:
    static bool CheckAABB(const BoxColliderComponent& a, const BoxColliderComponent& b);
    
    // Check collision between two entities with BoxColliderComponent
    static bool CheckCollision(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
};
