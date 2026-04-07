#include "pch.h"
#include "BoxColliderComponent.h"

#include "Entity.h"
#include "TransformComponent.h"

void BoxColliderComponent::Update(Entity& owner)
{
    if (auto* transform = owner.GetComponent<TransformComponent>())
    {
        UpdateBounds(transform->position);
    }
}
