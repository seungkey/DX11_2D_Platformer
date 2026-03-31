#pragma once
#include <vector>
#include "Entity.h"
#include "Renderer.h"


using namespace DirectX::SimpleMath;

// Constant buffer structure

class Game
{
public:
    Game();
    ~Game();

    void Initialize(HWND hwnd);
    void Shutdown();
    void Tick();

    std::vector<std::shared_ptr<Entity>> m_entities; // To track all entities for easy loop

private:
   

    HWND m_hwnd;

    std::unique_ptr<Renderer> m_renderer;

    // Game Objects
    std::shared_ptr<Entity> m_player;
    Vector2 m_playerVelocity; // Velocity is often a separate component, but for now we can keep it or make a PhysicsComponent later.

    std::vector<std::shared_ptr<Entity>> m_platforms;
};
