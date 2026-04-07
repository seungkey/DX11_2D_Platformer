#pragma once

#include "Component.h"
#include "InputCommand.h"

class Entity;

class MovementComponent : public Component
{
public:
    Vector2 velocity;
    float moveSpeed;
    float jumpVelocity;
    float gravity;
    float maxFallSpeed;
    bool isGrounded;

    MovementComponent(
        float inMoveSpeed = 260.0f,
        float inJumpVelocity = 620.0f,
        float inGravity = -1800.0f,
        float inMaxFallSpeed = 900.0f)
        : velocity(Vector2(0.0f, 0.0f)),
          moveSpeed(inMoveSpeed),
          jumpVelocity(inJumpVelocity),
          gravity(inGravity),
          maxFallSpeed(inMaxFallSpeed),
          isGrounded(false)
    {
    }

    void SimulateInput(const InputCommand& command, float fixedDeltaTime);
    void MoveHorizontally(Entity& owner, float fixedDeltaTime);
    void MoveVertically(Entity& owner, float fixedDeltaTime);
    void StopHorizontal();
    void StopVertical(bool grounded);
};
