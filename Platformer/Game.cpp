#include "pch.h"
#include "Game.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "RenderComponent.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "MovementComponent.h"
#include "PlatformerSimulation.h"

// Vertex structure
struct SimpleVertex
{
    float Pos[2]; // 2D position
};


Game::Game() :
    m_hwnd(nullptr),
    m_simulationTick(0)
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
    InputManager* inputManager = InputManager::GetInstance();
    inputManager->Initialize(m_hwnd);
    inputManager->AddActionBinding("MoveLeft", 'A');
    inputManager->AddActionBinding("MoveLeft", VK_LEFT);
    inputManager->AddActionBinding("MoveRight", 'D');
    inputManager->AddActionBinding("MoveRight", VK_RIGHT);
    inputManager->AddActionBinding("Jump", 'W');
    inputManager->AddActionBinding("Jump", VK_UP);
    inputManager->AddActionBinding("Jump", VK_SPACE);

    // --- Entity Initialization ---
    
    // Player
    m_player = std::make_shared<Entity>();
    // Position: Center of screen, Size: 50x50
    m_player->AddComponent<TransformComponent>(Vector2(800.f, 360.f), Vector2(1.f, 1.f));
    m_player->AddComponent<BoxColliderComponent>(Vector2(20.f, 20.f));
    m_player->AddComponent<MovementComponent>();
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

    for (auto& entity : m_entities)
    {
        entity->UpdateComponents();
    }
}

void Game::Shutdown()
{
}

void Game::Update(float dt)
{
    UNREFERENCED_PARAMETER(dt);

    InputManager::GetInstance()->Update();
    CapturePlayerInput();
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
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;

    while (TimeManager::m_elapsedTime >= kFixedDeltaTime)
    {
        ResetCollisionStates();

        InputCommand tickInput = m_pendingInput;
        tickInput.tick = ++m_simulationTick;

        if (auto* movement = m_player->GetComponent<MovementComponent>())
        {
            movement->SimulateInput(tickInput, kFixedDeltaTime);
            movement->MoveHorizontally(*m_player, kFixedDeltaTime);
            PlatformerSimulation::ResolveHorizontalCollisions(*m_player, m_platforms);

            movement->MoveVertically(*m_player, kFixedDeltaTime);
            PlatformerSimulation::ResolveVerticalCollisions(*m_player, m_platforms);
        }

        m_pendingInput.jumpPressed = false;
        TimeManager::m_elapsedTime -= kFixedDeltaTime;
    }
}

void Game::CapturePlayerInput()
{
    const InputManager* inputManager = InputManager::GetInstance();

    m_pendingInput.moveX = inputManager->GetAxis("MoveLeft", "MoveRight");
    // Keep one-shot jump input alive until the next fixed tick consumes it.
    m_pendingInput.jumpPressed = m_pendingInput.jumpPressed || inputManager->IsActionPressed("Jump");
    m_pendingInput.jumpHeld = inputManager->IsActionDown("Jump");
}

void Game::ResetCollisionStates()
{
    if (auto* playerCollider = m_player->GetComponent<BoxColliderComponent>())
    {
        playerCollider->isColliding = false;
    }

    for (auto& platform : m_platforms)
    {
        if (auto* platformCollider = platform->GetComponent<BoxColliderComponent>())
        {
            platformCollider->isColliding = false;
        }
    }
}

