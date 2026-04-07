#pragma once
#include <cstdint>
#include <vector>
#include "Entity.h"
#include "InputCommand.h"
#include "Renderer.h"

// Constant buffer structure

class Game
{
public:
    Game();
    ~Game();

    void Initialize(HWND hwnd);
    void Shutdown();
    void Update(float dt);
    void Loop();
    void FixedUpdate();

    std::vector<std::shared_ptr<Entity>> m_entities; // To track all entities for easy loop

private:
    void CapturePlayerInput();
    void ResetCollisionStates();

    HWND m_hwnd;

    std::unique_ptr<Renderer> m_renderer;

    // Game Objects
    std::shared_ptr<Entity> m_player;
    InputCommand m_pendingInput;
    std::uint32_t m_simulationTick;

    std::vector<std::shared_ptr<Entity>> m_platforms;
};
