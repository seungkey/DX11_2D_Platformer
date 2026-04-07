#include "pch.h"
#include "BoxColliderComponent.h"

#include "Entity.h"
#include "TransformComponent.h"

void BoxColliderComponent::Update(Entity& owner, float dt)
{
    (void)dt;

    if (auto* transform = owner.GetComponent<TransformComponent>())
    {
        UpdateBounds(transform->position);
    }
}
