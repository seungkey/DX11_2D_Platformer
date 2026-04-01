#include "pch.h"
#include "Game.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "RenderComponent.h"
#include "TimeManager.h"
#include "CollisionManager.h"

// Vertex structure
struct SimpleVertex
{
    float Pos[2]; // 2D position
};


Game::Game() :
    m_hwnd(nullptr),
    m_playerVelocity(Vector2(0.f, 0.f))
{
}

Game::~Game()
{
    Shutdown();
}

void Game::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;

    // Get window dimensions
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

   
    m_renderer = std::make_unique<Renderer>();
    m_renderer->Initialize(m_hwnd);
    
    TimeManager::GetInstance()->Initialize();
    // --- Entity Initialization ---
    
    // Player
    m_player = std::make_shared<Entity>();
    // Position: Center of screen, Size: 50x50
    m_player->AddComponent<TransformComponent>(Vector2(800.f, 360.f), Vector2(1.f, 1.f));
    m_player->AddComponent<BoxColliderComponent>(Vector2(20.f, 20.f));
    m_player->AddComponent<RenderComponent>(true);
    m_entities.push_back(m_player);

    // Platforms
    auto ground = std::make_shared<Entity>();
    // Position: Bottom, Size: 1280x40
    ground->AddComponent<TransformComponent>(Vector2(640.f, 20.f), Vector2(128.f, 1.f));
    ground->AddComponent<BoxColliderComponent>(Vector2(2560.f, 20.f));
    ground->AddComponent<RenderComponent>(true);
    m_platforms.push_back(ground);
    m_entities.push_back(ground);

    auto p1 = std::make_shared<Entity>();
    p1->AddComponent<TransformComponent>(Vector2(300.f, 200.f), Vector2(20.f, 4.f));
    p1->AddComponent<BoxColliderComponent>(Vector2(400.f, 80.f));
    p1->AddComponent<RenderComponent>(true);
    m_platforms.push_back(p1);
    m_entities.push_back(p1);

    auto p2 = std::make_shared<Entity>();
    p2->AddComponent<TransformComponent>(Vector2(900.f, 400.f), Vector2(20.f, 4.f));
    p2->AddComponent<BoxColliderComponent>(Vector2(400.f, 80.f));
    p2->AddComponent<RenderComponent>(true);
    m_platforms.push_back(p2);
    m_entities.push_back(p2);
}

void Game::Shutdown()
{
}

void Game::Update(float dt)
{
    // Reset collision states
    if (auto pCollider = m_player->GetComponent<BoxColliderComponent>())
        pCollider->isColliding = false;

    for (auto& platform : m_platforms)
    {
        if (auto pCollider = platform->GetComponent<BoxColliderComponent>())
            pCollider->isColliding = false;
    }

    // Simple player movement (for testing)
    auto transform = m_player->GetComponent<TransformComponent>();
    if (transform)
    {
        // Add basic test movement here if InputManager is ready
    }

    // Collision Check
    for (auto& platform : m_platforms)
    {
        if (CollisionManager::CheckCollision(m_player, platform))
        {
            // Collision detected!
            if (auto pCollider = m_player->GetComponent<BoxColliderComponent>())
                pCollider->isColliding = true;
            if (auto pCollider = platform->GetComponent<BoxColliderComponent>())
                pCollider->isColliding = true;

            OutputDebugString(L"Collision with platform detected!\n");
        }
    }
}

void Game::Loop()
{
    TimeManager::GetInstance()->Update();
    Update(TimeManager::GetInstance()->GetDeltaTime());
    FixedUpdate();
    m_renderer->Render();
    m_renderer->ComponentRender(m_entities);
    m_renderer->RenderDebug(m_entities); // Draw debug boxes
    m_renderer->RenderEnd();
}

void Game::FixedUpdate()
{
    while (TimeManager::m_elapsedTime > 0.015)
    {
        TimeManager::m_elapsedTime -= 0.015;
    }
}

