#include "pch.h"
#include "CollisionManager.h"
#include "Entity.h"
#include "BoxColliderComponent.h"

bool CollisionManager::CheckAABB(const BoxColliderComponent& a, const BoxColliderComponent& b)
{
    // Basic AABB overlap check
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y);
}

bool CollisionManager::CheckCollision(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
    if (!a || !b) return false;

    a->UpdateComponents();
    b->UpdateComponents();

    auto colliderA = a->GetComponent<BoxColliderComponent>();
    auto colliderB = b->GetComponent<BoxColliderComponent>();

    if (!colliderA || !colliderB)
        return false;

    return CheckAABB(*colliderA, *colliderB);
}
