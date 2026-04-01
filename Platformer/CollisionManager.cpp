#include "pch.h"
#include "CollisionManager.h"
#include "Entity.h"
#include "TransformComponent.h"
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

    auto transformA = a->GetComponent<TransformComponent>();
    auto colliderA = a->GetComponent<BoxColliderComponent>();
    auto transformB = b->GetComponent<TransformComponent>();
    auto colliderB = b->GetComponent<BoxColliderComponent>();

    if (!transformA || !colliderA || !transformB || !colliderB)
        return false;

    // Update bounds based on current position
    colliderA->UpdateBounds(transformA->position);
    colliderB->UpdateBounds(transformB->position);

    return CheckAABB(*colliderA, *colliderB);
}
