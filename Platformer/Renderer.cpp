#include "pch.h"
#include "Renderer.h"
#include "Entity.h"

void Renderer::Initialize(HWND hwnd)
{
    m_screenWidth = 1280;
    m_screenHeight = 720;
    // Create a device and swap chain
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
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
        &scd, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), nullptr, m_pDeviceContext.GetAddressOf());
    if (FAILED(hr)) return;

    // Create a render target view
    ComPtr<ID3D11Texture2D> pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
    if (FAILED(hr)) return;

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pRenderTargetView);
    if (FAILED(hr)) return;

    m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);

    // Set up the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_screenWidth;
    vp.Height = (FLOAT)m_screenHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pDeviceContext->RSSetViewports(1, &vp);

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile and create shaders
    ComPtr<ID3DBlob> errorBlob;
    ComPtr<ID3DBlob> pVSBlob;
    hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_Main", "vs_4_0", compileFlags, 0, &pVSBlob, &errorBlob);
    if (FAILED(hr))
    {
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }

        // 에러 메시지가 있으면 출력
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
    }
    hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf());
    if (FAILED(hr)) return;

    D3D11_INPUT_ELEMENT_DESC layout[] = { { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, };
    hr = m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_pInputLayout.GetAddressOf());
    if (FAILED(hr)) return;

    ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_Main", "ps_4_0", compileFlags, 0, &pPSBlob, &errorBlob);
    if (FAILED(hr)) return;
    hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf());
    if (FAILED(hr)) return;

    //2d는 quad를 그려서 텍스쳐 매핑만 할 예정이라 공용으로 사용할 버텍스 버퍼 생성
    vector<Vector2> quad = { Vector2(-10.f, 10.f),Vector2(10.f,10.f),Vector2(-10.f, -10.f),Vector2(10.f, -10.f) };//{ Vector2(-0.5f, 0.5f),Vector2(0.5f,0.5f),Vector2(-0.5f, -0.5f),Vector2(0.5f, -0.5f) };
    CreateVertexBuffer(quad, m_pVertexBuffer);

    // Create constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    hr = m_pDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer);
    if (FAILED(hr)) return;

    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthClipEnable = TRUE;

    ComPtr<ID3D11RasterizerState> rsState;
    m_pDevice->CreateRasterizerState(&rsDesc, &rsState);
    m_pDeviceContext->RSSetState(rsState.Get());

    std::vector<UINT> indices =
    {
        0, 1, 2,
        2, 1, 3
    };
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;
    ibDesc.MiscFlags = 0;
    ibDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();

    hr = m_pDevice->CreateBuffer(&ibDesc, &ibData, m_pIndexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        return;
    }
    m_constantBufferData.Proj = Matrix::CreateOrthographicOffCenter(0.f, m_screenWidth, 0.f, m_screenHeight, 0.f, 1.0f);
    m_constantBufferData.Proj = m_constantBufferData.Proj.Transpose();
    //m_constantBufferData.Proj = Matrix::Matrix();
    m_constantBufferData.View = Matrix::Matrix();
    
}

void Renderer::Clear()
{
    float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor);
}

void Renderer::ComponentRender(const vector<std::shared_ptr<Entity>> &Entities)
{
    for (auto E : Entities)
    {
        TransformComponent* Comp = E->GetComponent<TransformComponent>();
        m_constantBufferData.Model = Matrix::CreateScale(Comp->scale.x, Comp->scale.y, 1.0f) * Matrix::CreateTranslation(Comp->position.x, Comp->position.y, 0.0f);
        m_constantBufferData.Model = m_constantBufferData.Model.Transpose();
        D3D11_MAPPED_SUBRESOURCE ms;
        m_pDeviceContext->Map(m_pConstantBuffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &m_constantBufferData, sizeof(m_constantBufferData));
        m_pDeviceContext->Unmap(m_pConstantBuffer.Get(), NULL);

        m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
        
        m_pDeviceContext->DrawIndexed(6, 0, 0);;
    }
}

void Renderer::RenderEnd()
{
    m_pSwapChain->Present(1, 0);
}

void Renderer::Render()
{
    Clear();

    // --- Set up pipeline for drawing ---
    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
    UINT stride = sizeof(Vector2);
    UINT offset = 0;
    m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
    // --- Draw all entities with RenderComponent and TransformComponent ---


    // --- Present ---
    
}
