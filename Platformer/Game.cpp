#include "pch.h"
#include "Game.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "RenderComponent.h"
#include "SpriteAnimationComponent.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "MovementComponent.h"
#include "PlatformerSimulation.h"

#include <vector>

// Vertex structure
struct SimpleVertex
{
    float Pos[2]; // 2D position
};

namespace
{
    std::wstring NormalizePath(const std::wstring& path)
    {
        wchar_t fullPath[MAX_PATH] = {};
        const DWORD length = GetFullPathNameW(path.c_str(), MAX_PATH, fullPath, nullptr);
        if (length == 0 || length >= MAX_PATH)
        {
            return path;
        }

        return std::wstring(fullPath);
    }

    std::wstring GetModuleDirectory()
    {
        wchar_t modulePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, modulePath, MAX_PATH);

        std::wstring path(modulePath);
        const size_t lastSlash = path.find_last_of(L"\\/");
        if (lastSlash == std::wstring::npos)
        {
            return L".";
        }

        return path.substr(0, lastSlash);
    }

    bool FileExists(const std::wstring& path)
    {
        const DWORD attributes = GetFileAttributesW(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES
            && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    std::wstring FindPlayerTexturePath()
    {
        wchar_t currentDirectory[MAX_PATH] = {};
        const DWORD currentDirectoryLength = GetCurrentDirectoryW(MAX_PATH, currentDirectory);
        const std::wstring moduleDir = GetModuleDirectory();
        const std::vector<std::wstring> candidates =
        {
            currentDirectoryLength > 0 ? std::wstring(currentDirectory) + L"\\Resource\\Player.png" : L"",
            currentDirectoryLength > 0 ? std::wstring(currentDirectory) + L"\\Resource\\player.png" : L"",
            currentDirectoryLength > 0 ? std::wstring(currentDirectory) + L"\\..\\Resource\\Player.png" : L"",
            currentDirectoryLength > 0 ? std::wstring(currentDirectory) + L"\\..\\Resource\\player.png" : L"",
            L"Resource\\Player.png",
            L"Resource\\player.png",
            L"..\\Resource\\Player.png",
            L"..\\Resource\\player.png",
            moduleDir + L"\\..\\..\\..\\Resource\\Player.png",
            moduleDir + L"\\..\\..\\..\\Resource\\player.png",
            moduleDir + L"\\..\\..\\..\\..\\Resource\\Player.png",
            moduleDir + L"\\..\\..\\..\\..\\Resource\\player.png",
        };

        for (const auto& candidate : candidates)
        {
            const std::wstring fullPath = NormalizePath(candidate);
            if (FileExists(fullPath))
            {
                return fullPath;
            }
        }

        return L"";
    }

    SpriteAnimationFrame MakeFrameUV(
        float textureWidth,
        float textureHeight,
        float left,
        float top,
        float width,
        float height)
    {
        return SpriteAnimationFrame(
            Vector2(left / textureWidth, top / textureHeight),
            Vector2(width / textureWidth, height / textureHeight));
    }

    std::vector<SpriteAnimationFrame> CreatePlayerIdleFrames()
    {
        constexpr float kTextureWidth = 3148.0f;
        constexpr float kTextureHeight = 1065.0f;

        return
        {
            MakeFrameUV(kTextureWidth, kTextureHeight, 0.0f, 0.0f, 93.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 101.0f, 0.0f, 98.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 199.0f, 0.0f, 99.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 298.0f, 0.0f, 101.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 398.0f, 0.0f, 100.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 500.0f, 0.0f, 99.0f, 94.0f),
            MakeFrameUV(kTextureWidth, kTextureHeight, 600.0f, 0.0f, 96.0f, 94.0f),
        };
    }
}


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

    std::string playerTextureId = "player_test";
    if (auto* resourceManager = m_renderer->GetResourceManager())
    {
        const std::wstring playerTexturePath = FindPlayerTexturePath();
        if (!playerTexturePath.empty() && resourceManager->LoadTexture2D("player", playerTexturePath))
        {
            playerTextureId = "player";
        }
    }
    
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
    m_player->AddComponent<TransformComponent>(Vector2(800.f, 360.f), Vector2(5.f, 5.f));
    m_player->AddComponent<BoxColliderComponent>(Vector2(100.f, 100.f));
    m_player->AddComponent<MovementComponent>();
    m_player->AddComponent<RenderComponent>(true, "default", "quad", playerTextureId);
    auto& playerAnimation = m_player->AddComponent<SpriteAnimationComponent>(playerTextureId, 4, 1, 0, 4, 8.0f, true);
    if (playerTextureId == "player")
    {
        playerAnimation.SetFrames(CreatePlayerIdleFrames());
    }
    m_entities.push_back(m_player);

    // Platforms
    auto ground = std::make_shared<Entity>();
    // Position: Bottom, Size: 1280x40
    ground->AddComponent<TransformComponent>(Vector2(640.f, 20.f), Vector2(128.f, 1.f));
    ground->AddComponent<BoxColliderComponent>(Vector2(2560.f, 20.f));
    ground->AddComponent<RenderComponent>(true, "default", "quad", "white", Vector4(0.20f, 0.24f, 0.28f, 1.0f));
    m_platforms.push_back(ground);
    m_entities.push_back(ground);

    auto p1 = std::make_shared<Entity>();
    p1->AddComponent<TransformComponent>(Vector2(300.f, 200.f), Vector2(20.f, 4.f));
    p1->AddComponent<BoxColliderComponent>(Vector2(400.f, 80.f));
    p1->AddComponent<RenderComponent>(true, "default", "quad", "white", Vector4(0.28f, 0.50f, 0.35f, 1.0f));
    m_platforms.push_back(p1);
    m_entities.push_back(p1);

    auto p2 = std::make_shared<Entity>();
    p2->AddComponent<TransformComponent>(Vector2(900.f, 400.f), Vector2(20.f, 4.f));
    p2->AddComponent<BoxColliderComponent>(Vector2(400.f, 80.f));
    p2->AddComponent<RenderComponent>(true, "default", "quad", "white", Vector4(0.33f, 0.42f, 0.60f, 1.0f));
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
    InputManager::GetInstance()->Update();
    CapturePlayerInput();

    for (auto& entity : m_entities)
    {
        entity->UpdateComponents(dt);
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

