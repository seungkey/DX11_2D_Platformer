#pragma once

#include <memory>
#include <vector>
#include <wrl.h>

#include <d3d11.h>
#include <directxtk/SimpleMath.h>

#include "ResourceManager.h"

using Microsoft::WRL::ComPtr;
using std::vector;
using namespace DirectX::SimpleMath;

class Entity;

struct ConstantBuffer
{
    Matrix Model;
    Matrix View;
    Matrix Proj;
    Vector4 Color;
    Vector4 UVRect;
};

class Renderer
{
public:
    void Initialize(HWND hwnd);
    void Render();
    void Clear();
    void ComponentRender(const vector<std::shared_ptr<Entity>>& entities);
    void RenderEnd();
    void RenderDebug(const vector<std::shared_ptr<Entity>>& entities);
    ResourceManager* GetResourceManager() const { return m_resourceManager.get(); }

private:
    bool CreateDefaultResources();
    bool UpdateConstantBuffer();

    int m_screenWidth = 0;
    int m_screenHeight = 0;

    ComPtr<IDXGISwapChain> m_pSwapChain;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pDeviceContext;
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
    ComPtr<ID3D11Buffer> m_pConstantBuffer;
    ComPtr<ID3D11SamplerState> m_pSamplerState;
    ComPtr<ID3D11BlendState> m_pAlphaBlendState;

    std::unique_ptr<ResourceManager> m_resourceManager;
    ConstantBuffer m_constantBufferData = {};
};
