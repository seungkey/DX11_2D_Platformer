#pragma once
#include <wrl.h>
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;
using std::vector;

class Entity;

struct ConstantBuffer
{
    Matrix Model;
    Matrix View;
    Matrix Proj;
};

class Renderer
{
public:
    void Initialize(HWND hwnd);
    void Render();
    void Clear();
    void ComponentRender(const vector<std::shared_ptr<Entity>> &Entities);
    void RenderEnd();

private:
    

    template<typename T>
    void CreateVertexBuffer(const vector<T>& vertices, ComPtr<ID3D11Buffer>& vertexBuffer) {
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = UINT(sizeof(T) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.StructureByteStride = sizeof(T);

        D3D11_SUBRESOURCE_DATA vertexBufferData = {
            0 }; // MS 예제에서 초기화하는 방식
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        const HRESULT hr = m_pDevice->CreateBuffer(
            &bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            std::cout << "CreateBuffer() failed. " << std::hex << hr
                << std::endl;
        };
    }

    int m_screenWidth;
    int m_screenHeight;

    // DirectX Objects
    ComPtr<IDXGISwapChain>         m_pSwapChain;
    ComPtr<ID3D11Device>           m_pDevice;
    ComPtr<ID3D11DeviceContext>    m_pDeviceContext;
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

    // Shader objects
    ComPtr<ID3D11VertexShader>     m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;
    ComPtr<ID3D11InputLayout> m_pInputLayout;
    ComPtr<ID3D11Buffer> m_pVertexBuffer;
    ComPtr<ID3D11Buffer> m_pConstantBuffer;
    ComPtr<ID3D11Buffer> m_pIndexBuffer;

    ConstantBuffer m_constantBufferData;
};

