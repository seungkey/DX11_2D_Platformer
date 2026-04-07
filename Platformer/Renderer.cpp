#include "pch.h"
#include "Renderer.h"

#include <cstdint>
#include <iostream>
#include <vector>

#include "BoxColliderComponent.h"
#include "Entity.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "TransformComponent.h"

namespace
{
    constexpr float kDefaultHalfExtent = 10.0f;
    constexpr UINT kTestSpriteFrameSize = 16;
    constexpr UINT kTestSpriteFrameCount = 4;

    struct SpriteVertex
    {
        Vector2 position;
        Vector2 uv;
    };

    std::uint32_t MakeColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
    {
        return static_cast<std::uint32_t>(r)
            | (static_cast<std::uint32_t>(g) << 8)
            | (static_cast<std::uint32_t>(b) << 16)
            | (static_cast<std::uint32_t>(a) << 24);
    }

    void SetPixel(
        std::vector<std::uint32_t>& pixels,
        UINT textureWidth,
        UINT textureHeight,
        int x,
        int y,
        std::uint32_t color)
    {
        if (x < 0 || y < 0 || x >= static_cast<int>(textureWidth) || y >= static_cast<int>(textureHeight))
        {
            return;
        }

        pixels[static_cast<size_t>(y) * textureWidth + static_cast<size_t>(x)] = color;
    }

    void FillRect(
        std::vector<std::uint32_t>& pixels,
        UINT textureWidth,
        UINT textureHeight,
        int left,
        int top,
        int width,
        int height,
        std::uint32_t color)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                SetPixel(pixels, textureWidth, textureHeight, left + x, top + y, color);
            }
        }
    }

    std::vector<std::uint32_t> CreateTestSpriteSheetPixels(UINT frameSize, UINT frameCount)
    {
        const UINT textureWidth = frameSize * frameCount;
        const UINT textureHeight = frameSize;
        std::vector<std::uint32_t> pixels(textureWidth * textureHeight, MakeColor(0, 0, 0, 0));

        const std::uint32_t outline = MakeColor(21, 24, 30, 255);
        const std::uint32_t skin = MakeColor(255, 224, 189, 255);
        const std::uint32_t shirt = MakeColor(80, 170, 255, 255);
        const std::uint32_t pants = MakeColor(45, 70, 140, 255);
        const std::uint32_t boots = MakeColor(120, 75, 45, 255);

        for (UINT frame = 0; frame < frameCount; ++frame)
        {
            const int frameLeft = static_cast<int>(frame * frameSize);
            const int bobOffset = (frame % 2 == 0) ? 0 : 1;
            const int legShift = static_cast<int>(frame % frameCount);

            FillRect(pixels, textureWidth, textureHeight, frameLeft + 4, 1 + bobOffset, 8, 4, skin);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 4, 5 + bobOffset, 8, 5, shirt);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 5, 10 + bobOffset, 6, 3, pants);

            FillRect(pixels, textureWidth, textureHeight, frameLeft + 3 + (legShift == 1 ? 1 : 0), 8 + bobOffset, 1, 4, skin);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 12 - (legShift == 3 ? 1 : 0), 8 + bobOffset, 1, 4, skin);

            FillRect(pixels, textureWidth, textureHeight, frameLeft + 5 + (legShift == 2 ? -1 : 0), 13 + bobOffset, 2, 3, boots);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 9 + (legShift == 0 ? 1 : 0), 13 + bobOffset, 2, 3, boots);

            FillRect(pixels, textureWidth, textureHeight, frameLeft + 6, 3 + bobOffset, 1, 1, outline);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 9, 3 + bobOffset, 1, 1, outline);

            FillRect(pixels, textureWidth, textureHeight, frameLeft + 4, 1 + bobOffset, 8, 1, outline);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 4, 4 + bobOffset, 8, 1, outline);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 4, 5 + bobOffset, 1, 5, outline);
            FillRect(pixels, textureWidth, textureHeight, frameLeft + 11, 5 + bobOffset, 1, 5, outline);
        }

        return pixels;
    }
}

void Renderer::Initialize(HWND hwnd)
{
    RECT rc = {};
    GetClientRect(hwnd, &rc);
    m_screenWidth = rc.right - rc.left;
    m_screenHeight = rc.bottom - rc.top;

    if (m_screenWidth <= 0)
    {
        m_screenWidth = 1280;
    }

    if (m_screenHeight <= 0)
    {
        m_screenHeight = 720;
    }

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = m_screenWidth;
    scd.BufferDesc.Height = m_screenHeight;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &scd,
        m_pSwapChain.GetAddressOf(),
        m_pDevice.GetAddressOf(),
        nullptr,
        m_pDeviceContext.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_pSwapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr))
    {
        return;
    }

    hr = m_pDevice->CreateRenderTargetView(
        backBuffer.Get(),
        nullptr,
        m_pRenderTargetView.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(m_screenWidth);
    vp.Height = static_cast<FLOAT>(m_screenHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    m_pDeviceContext->RSSetViewports(1, &vp);

    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    hr = m_pDevice->CreateBuffer(&bd, nullptr, m_pConstantBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthClipEnable = TRUE;

    ComPtr<ID3D11RasterizerState> rsState;
    hr = m_pDevice->CreateRasterizerState(&rsDesc, rsState.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    m_pDeviceContext->RSSetState(rsState.Get());

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = m_pDevice->CreateSamplerState(&samplerDesc, m_pSamplerState.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_pDevice->CreateBlendState(&blendDesc, m_pAlphaBlendState.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }

    m_constantBufferData.Proj =
        Matrix::CreateOrthographicOffCenter(
            0.0f,
            static_cast<float>(m_screenWidth),
            0.0f,
            static_cast<float>(m_screenHeight),
            0.0f,
            1.0f)
            .Transpose();
    m_constantBufferData.View = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f).Transpose();
    m_constantBufferData.Color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    m_constantBufferData.UVRect = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

    m_resourceManager = std::make_unique<ResourceManager>();
    if (!m_resourceManager->Initialize(m_pDevice.Get()))
    {
        return;
    }

    CreateDefaultResources();
}

bool Renderer::CreateDefaultResources()
{
    if (!m_resourceManager)
    {
        return false;
    }

    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (!m_resourceManager->LoadShaderProgram(
            "default",
            L"VertexShader.hlsl",
            L"PixelShader.hlsl",
            layout,
            ARRAYSIZE(layout)))
    {
        std::cout << "Failed to load default shader program." << std::endl;
        return false;
    }

    const std::vector<SpriteVertex> quad =
    {
        { Vector2(-kDefaultHalfExtent, kDefaultHalfExtent), Vector2(0.0f, 0.0f) },
        { Vector2(kDefaultHalfExtent, kDefaultHalfExtent), Vector2(1.0f, 0.0f) },
        { Vector2(-kDefaultHalfExtent, -kDefaultHalfExtent), Vector2(0.0f, 1.0f) },
        { Vector2(kDefaultHalfExtent, -kDefaultHalfExtent), Vector2(1.0f, 1.0f) },
    };
    const std::vector<UINT> quadIndices = { 0, 1, 2, 2, 1, 3 };

    if (!m_resourceManager->CreateMesh("quad", quad, quadIndices))
    {
        std::cout << "Failed to create default quad mesh." << std::endl;
        return false;
    }

    const std::vector<SpriteVertex> debugQuad =
    {
        { Vector2(-0.5f, 0.5f), Vector2(0.0f, 0.0f) },
        { Vector2(0.5f, 0.5f), Vector2(1.0f, 0.0f) },
        { Vector2(-0.5f, -0.5f), Vector2(0.0f, 1.0f) },
        { Vector2(0.5f, -0.5f), Vector2(1.0f, 1.0f) },
    };
    const std::vector<UINT> debugIndices = { 0, 1, 3, 2, 0 };

    if (!m_resourceManager->CreateMesh(
            "quad_debug",
            debugQuad,
            debugIndices,
            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP))
    {
        std::cout << "Failed to create debug quad mesh." << std::endl;
        return false;
    }

    if (!m_resourceManager->CreateTexture2D("white", 1, 1, { MakeColor(255, 255, 255, 255) }))
    {
        std::cout << "Failed to create default white texture." << std::endl;
        return false;
    }

    if (!m_resourceManager->CreateTexture2D(
            "player_test",
            kTestSpriteFrameSize * kTestSpriteFrameCount,
            kTestSpriteFrameSize,
            CreateTestSpriteSheetPixels(kTestSpriteFrameSize, kTestSpriteFrameCount)))
    {
        std::cout << "Failed to create test sprite sheet." << std::endl;
        return false;
    }

    return true;
}

void Renderer::Clear()
{
    if (!m_pDeviceContext || !m_pRenderTargetView)
    {
        return;
    }

    constexpr float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor);
}

void Renderer::ComponentRender(const vector<std::shared_ptr<Entity>>& entities)
{
    if (!m_pDeviceContext || !m_resourceManager)
    {
        return;
    }

    for (const auto& entity : entities)
    {
        auto* transform = entity->GetComponent<TransformComponent>();
        auto* renderComponent = entity->GetComponent<RenderComponent>();
        if (!transform || !renderComponent || !renderComponent->visible)
        {
            continue;
        }

        const ShaderProgramResource* shaderProgram =
            m_resourceManager->GetShaderProgram(renderComponent->shaderId);
        const MeshResource* mesh =
            m_resourceManager->GetMesh(renderComponent->meshId);
        const TextureResource* texture =
            m_resourceManager->GetTexture(renderComponent->textureId);
        if (!texture)
        {
            texture = m_resourceManager->GetTexture("white");
        }

        if (!shaderProgram || !mesh || !texture)
        {
            continue;
        }

        m_pDeviceContext->VSSetShader(shaderProgram->vertexShader.Get(), nullptr, 0);
        m_pDeviceContext->PSSetShader(shaderProgram->pixelShader.Get(), nullptr, 0);
        m_pDeviceContext->IASetInputLayout(shaderProgram->inputLayout.Get());

        UINT stride = mesh->vertexStride;
        UINT offset = 0;
        ID3D11Buffer* vertexBuffer = mesh->vertexBuffer.Get();
        m_pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        m_pDeviceContext->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        m_pDeviceContext->IASetPrimitiveTopology(mesh->topology);
        ID3D11ShaderResourceView* shaderResourceView = texture->shaderResourceView.Get();
        m_pDeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
        m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

        m_constantBufferData.Model =
            (Matrix::CreateScale(transform->scale.x, transform->scale.y, 1.0f) *
             Matrix::CreateTranslation(transform->position.x, transform->position.y, 0.0f))
                .Transpose();
        m_constantBufferData.Color = renderComponent->color;
        m_constantBufferData.UVRect = Vector4(
            renderComponent->uvOffset.x,
            renderComponent->uvOffset.y,
            renderComponent->uvScale.x,
            renderComponent->uvScale.y);

        if (!UpdateConstantBuffer())
        {
            continue;
        }

        m_pDeviceContext->DrawIndexed(mesh->indexCount, 0, 0);
    }
}

void Renderer::RenderDebug(const vector<std::shared_ptr<Entity>>& entities)
{
    if (!m_pDeviceContext || !m_resourceManager)
    {
        return;
    }

    const ShaderProgramResource* shaderProgram = m_resourceManager->GetShaderProgram("default");
    const MeshResource* debugMesh = m_resourceManager->GetMesh("quad_debug");
    const TextureResource* texture = m_resourceManager->GetTexture("white");
    if (!shaderProgram || !debugMesh || !texture)
    {
        return;
    }

    m_pDeviceContext->VSSetShader(shaderProgram->vertexShader.Get(), nullptr, 0);
    m_pDeviceContext->PSSetShader(shaderProgram->pixelShader.Get(), nullptr, 0);
    m_pDeviceContext->IASetInputLayout(shaderProgram->inputLayout.Get());

    UINT stride = debugMesh->vertexStride;
    UINT offset = 0;
    ID3D11Buffer* vertexBuffer = debugMesh->vertexBuffer.Get();
    m_pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    m_pDeviceContext->IASetIndexBuffer(debugMesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_pDeviceContext->IASetPrimitiveTopology(debugMesh->topology);
    ID3D11ShaderResourceView* shaderResourceView = texture->shaderResourceView.Get();
    m_pDeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
    m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

    for (const auto& entity : entities)
    {
        auto* transform = entity->GetComponent<TransformComponent>();
        auto* collider = entity->GetComponent<BoxColliderComponent>();
        if (!transform || !collider)
        {
            continue;
        }

        const Vector2 position = transform->position + collider->offset;
        m_constantBufferData.Model =
            (Matrix::CreateScale(collider->size.x, collider->size.y, 1.0f) *
             Matrix::CreateTranslation(position.x, position.y, 0.0f))
                .Transpose();
        m_constantBufferData.Color = collider->isColliding
                                         ? Vector4(1.0f, 0.0f, 0.0f, 1.0f)
                                         : Vector4(0.0f, 1.0f, 0.0f, 1.0f);
        m_constantBufferData.UVRect = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

        if (!UpdateConstantBuffer())
        {
            continue;
        }

        m_pDeviceContext->DrawIndexed(debugMesh->indexCount, 0, 0);
    }
}

bool Renderer::UpdateConstantBuffer()
{
    if (!m_pDeviceContext || !m_pConstantBuffer)
    {
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
    const HRESULT hr = m_pDeviceContext->Map(
        m_pConstantBuffer.Get(),
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mappedSubresource);
    if (FAILED(hr))
    {
        return false;
    }

    memcpy(mappedSubresource.pData, &m_constantBufferData, sizeof(m_constantBufferData));
    m_pDeviceContext->Unmap(m_pConstantBuffer.Get(), 0);

    ID3D11Buffer* constantBuffer = m_pConstantBuffer.Get();
    m_pDeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    m_pDeviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

    return true;
}

void Renderer::RenderEnd()
{
    if (m_pSwapChain)
    {
        m_pSwapChain->Present(1, 0);
    }
}

void Renderer::Render()
{
    Clear();

    if (m_pDeviceContext && m_pRenderTargetView)
    {
        m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
        const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), blendFactor, 0xFFFFFFFF);
    }
}
