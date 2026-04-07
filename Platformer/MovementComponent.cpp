#include "pch.h"
#include "MovementComponent.h"

#include "Entity.h"
#include "TransformComponent.h"

void MovementComponent::SimulateInput(const InputCommand& command, float fixedDeltaTime)
{
    const bool wasGrounded = isGrounded;
    isGrounded = false;

    velocity.x = command.moveX * moveSpeed;

    if (command.jumpPressed && wasGrounded)
    {
        velocity.y = jumpVelocity;
    }

    velocity.y += gravity * fixedDeltaTime;
    if (velocity.y < -maxFallSpeed)
    {
        velocity.y = -maxFallSpeed;
    }
}

void MovementComponent::MoveHorizontally(Entity& owner, float fixedDeltaTime)
{
    if (auto* transform = owner.GetComponent<TransformComponent>())
    {
        transform->position.x += velocity.x * fixedDeltaTime;
        owner.UpdateComponents();
    }
}

void MovementComponent::MoveVertically(Entity& owner, float fixedDeltaTime)
{
    if (auto* transform = owner.GetComponent<TransformComponent>())
    {
        transform->position.y += velocity.y * fixedDeltaTime;
        owner.UpdateComponents();
    }
}

void MovementComponent::StopHorizontal()
{
    velocity.x = 0.0f;
}

void MovementComponent::StopVertical(bool grounded)
{
    velocity.y = 0.0f;
    isGrounded = grounded;
}
