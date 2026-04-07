#include "pch.h"
#include "PlatformerSimulation.h"

#include "BoxColliderComponent.h"
#include "CollisionManager.h"
#include "Entity.h"
#include "MovementComponent.h"
#include "TransformComponent.h"

namespace
{
    constexpr float kCollisionEpsilon = 0.001f;

    float GetOverlap(float minA, float maxA, float minB, float maxB)
    {
        const float overlapMax = (maxA < maxB) ? maxA : maxB;
        const float overlapMin = (minA > minB) ? minA : minB;
        return overlapMax - overlapMin;
    }
}

void PlatformerSimulation::ResolveHorizontalCollisions(
    Entity& player,
    const std::vector<std::shared_ptr<Entity>>& platforms)
{
    auto* transform = player.GetComponent<TransformComponent>();
    auto* collider = player.GetComponent<BoxColliderComponent>();
    auto* movement = player.GetComponent<MovementComponent>();
    if (!transform || !collider || !movement)
    {
        return;
    }

    for (const auto& platform : platforms)
    {
        if (!platform)
        {
            continue;
        }

        platform->UpdateComponents();

        auto* platformCollider = platform->GetComponent<BoxColliderComponent>();
        if (!platformCollider)
        {
            continue;
        }

        if (!CollisionManager::CheckAABB(*collider, *platformCollider))
        {
            continue;
        }

        const float verticalOverlap = GetOverlap(
            collider->min.y,
            collider->max.y,
            platformCollider->min.y,
            platformCollider->max.y);
        if (verticalOverlap <= kCollisionEpsilon)
        {
            continue;
        }

        if (movement->velocity.x > 0.0f)
        {
            transform->position.x = platformCollider->min.x - collider->offset.x - (collider->size.x * 0.5f);
        }
        else if (movement->velocity.x < 0.0f)
        {
            transform->position.x = platformCollider->max.x - collider->offset.x + (collider->size.x * 0.5f);
        }
        else
        {
            continue;
        }

        movement->StopHorizontal();
        player.UpdateComponents();
        MarkCollision(player, *platform);
        break;
    }
}

void PlatformerSimulation::ResolveVerticalCollisions(
    Entity& player,
    const std::vector<std::shared_ptr<Entity>>& platforms)
{
    auto* transform = player.GetComponent<TransformComponent>();
    auto* collider = player.GetComponent<BoxColliderComponent>();
    auto* movement = player.GetComponent<MovementComponent>();
    if (!transform || !collider || !movement)
    {
        return;
    }

    for (const auto& platform : platforms)
    {
        if (!platform)
        {
            continue;
        }

        platform->UpdateComponents();

        auto* platformCollider = platform->GetComponent<BoxColliderComponent>();
        if (!platformCollider)
        {
            continue;
        }

        if (!CollisionManager::CheckAABB(*collider, *platformCollider))
        {
            continue;
        }

        const float horizontalOverlap = GetOverlap(
            collider->min.x,
            collider->max.x,
            platformCollider->min.x,
            platformCollider->max.x);
        if (horizontalOverlap <= kCollisionEpsilon)
        {
            continue;
        }

        if (movement->velocity.y > 0.0f)
        {
            transform->position.y = platformCollider->min.y - collider->offset.y - (collider->size.y * 0.5f);
            movement->StopVertical(false);
        }
        else if (movement->velocity.y < 0.0f)
        {
            transform->position.y = platformCollider->max.y - collider->offset.y + (collider->size.y * 0.5f);
            movement->StopVertical(true);
        }
        else
        {
            continue;
        }

        player.UpdateComponents();
        MarkCollision(player, *platform);
        break;
    }
}

void PlatformerSimulation::MarkCollision(Entity& player, Entity& platform)
{
    auto* playerCollider = player.GetComponent<BoxColliderComponent>();
    auto* platformCollider = platform.GetComponent<BoxColliderComponent>();
    if (playerCollider)
    {
        playerCollider->isColliding = true;
    }

    if (platformCollider)
    {
        platformCollider->isColliding = true;
    }
}
