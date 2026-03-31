#include "pch.h"
#include "Game.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "RenderComponent.h"

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
    
    // --- Entity Initialization ---
    
    // Player
    m_player = std::make_shared<Entity>();
    m_player->AddComponent<TransformComponent>(Vector2(10.f, 700.f), Vector2(10.f, 10.f));
    m_player->AddComponent<BoxColliderComponent>(Vector2(0.1f, 0.1f));
    m_player->AddComponent<RenderComponent>(true);
    m_entities.push_back(m_player);

    // Platforms
    auto ground = std::make_shared<Entity>();
    ground->AddComponent<TransformComponent>(Vector2(640.f, 350.f), Vector2(1.f, 1.f));
    ground->AddComponent<BoxColliderComponent>(Vector2(100.f, 1.0f));
    ground->AddComponent<RenderComponent>(true);
    m_platforms.push_back(ground);
    m_entities.push_back(ground);

    auto p1 = std::make_shared<Entity>();
    p1->AddComponent<TransformComponent>(Vector2(0.5f, -0.4f), Vector2(0.5f, 0.1f));
    p1->AddComponent<BoxColliderComponent>(Vector2(0.5f, 0.1f));
    p1->AddComponent<RenderComponent>(true);
    m_platforms.push_back(p1);
    m_entities.push_back(p1);

    auto p2 = std::make_shared<Entity>();
    p2->AddComponent<TransformComponent>(Vector2(-0.5f, 0.0f), Vector2(0.5f, 0.1f));
    p2->AddComponent<BoxColliderComponent>(Vector2(0.5f, 0.1f));
    p2->AddComponent<RenderComponent>(true);
    m_platforms.push_back(p2);
    m_entities.push_back(p2);
}

void Game::Shutdown()
{
}

void Game::Tick()
{
    auto transform = m_player->GetComponent<TransformComponent>();
    if (!transform) return;

    // --- Physics & Input ---
    const float gravity = -0.0015f;
    const float moveSpeed = 0.005f;
    const float jumpForce = 0.035f;
    bool isOnGround = false;

    // Store previous position
    Vector2 prevPlayerPosition = transform->position;

    // Apply gravity
    m_playerVelocity.y += gravity;

    // Horizontal movement
    if (GetKeyState(VK_LEFT) & 0x8000)
    {
        m_playerVelocity.x = -moveSpeed;
    }
    else if (GetKeyState(VK_RIGHT) & 0x8000)
    {
        m_playerVelocity.x = moveSpeed;
    }
    else
    {
        m_playerVelocity.x *= 0.9f; // Friction
    }

    // Update position
    transform->position += m_playerVelocity;

    // --- Collision Detection ---
    auto playerCollider = m_player->GetComponent<BoxColliderComponent>();

    for (const auto& platformEntity : m_platforms)
    {
        auto platformTransform = platformEntity->GetComponent<TransformComponent>();
        auto platformCollider = platformEntity->GetComponent<BoxColliderComponent>();
        
        if (!platformTransform || !platformCollider) continue;

        // Simple AABB collision detection using BoxCollider components
        if (transform->position.x < platformTransform->position.x + platformCollider->size.x &&
            transform->position.x + playerCollider->size.x > platformTransform->position.x &&
            transform->position.y < platformTransform->position.y + platformCollider->size.y &&
            transform->position.y + playerCollider->size.y > platformTransform->position.y)
        {
            // Check if player was above the platform in the previous frame
            if (prevPlayerPosition.y + playerCollider->size.y >= platformTransform->position.y + platformCollider->size.y && m_playerVelocity.y < 0)
            {
                // Landed on top
                transform->position.y = platformTransform->position.y + platformCollider->size.y - playerCollider->size.y;
                m_playerVelocity.y = 0;
                isOnGround = true;
                break; 
            }
        }
    }
    
    // Jumping
    if (isOnGround && (GetKeyState(VK_UP) & 0x8000))
    {
        m_playerVelocity.y = jumpForce;
    }

    // Screen bounds check (left/right)
    if (transform->position.x < -0.95f) transform->position.x = -0.95f;
    if (transform->position.x > 0.95f) transform->position.x = 0.95f;

    m_renderer->Render();
    m_renderer->ComponentRender(m_entities);
    m_renderer->RenderEnd();
}

