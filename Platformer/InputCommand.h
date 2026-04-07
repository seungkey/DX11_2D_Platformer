#pragma once

#include <cstdint>

struct InputCommand
{
    std::uint32_t tick = 0;
    float moveX = 0.0f;
    bool jumpPressed = false;
    bool jumpHeld = false;
};
