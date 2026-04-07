#include "pch.h"
#include "ResourceManager.h"

#include <iostream>

#include <directxtk/WICTextureLoader.h>

bool ResourceManager::Initialize(ID3D11Device* device)
{
    if (!device)
    {
        return false;
    }

    m_device = device;
    return true;
}

void ResourceManager::Shutdown()
{
    m_shaderPrograms.clear();
    m_meshes.clear();
    m_textures.clear();
    m_device.Reset();
}

bool ResourceManager::LoadShaderProgram(
    const std::string& resourceId,
    const std::wstring& vertexShaderPath,
    const std::wstring& pixelShaderPath,
    const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc,
    UINT inputLayoutCount,
    const char* vertexEntryPoint,
    const char* pixelEntryPoint,
    const char* vertexShaderModel,
    const char* pixelShaderModel)
{
    if (!m_device || !inputLayoutDesc || inputLayoutCount == 0)
    {
        return false;
    }

    ShaderProgramResource shaderProgram;

    if (!CompileShader(vertexShaderPath, vertexEntryPoint, vertexShaderModel, shaderProgram.vertexShaderBlob))
    {
        return false;
    }

    HRESULT hr = m_device->CreateVertexShader(
        shaderProgram.vertexShaderBlob->GetBufferPointer(),
        shaderProgram.vertexShaderBlob->GetBufferSize(),
        nullptr,
        shaderProgram.vertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateInputLayout(
        inputLayoutDesc,
        inputLayoutCount,
        shaderProgram.vertexShaderBlob->GetBufferPointer(),
        shaderProgram.vertexShaderBlob->GetBufferSize(),
        shaderProgram.inputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    ComPtr<ID3DBlob> pixelShaderBlob;
    if (!CompileShader(pixelShaderPath, pixelEntryPoint, pixelShaderModel, pixelShaderBlob))
    {
        return false;
    }

    hr = m_device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        shaderProgram.pixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    m_shaderPrograms[resourceId] = shaderProgram;
    return true;
}

bool ResourceManager::LoadTexture2D(
    const std::string& resourceId,
    const std::wstring& filePath)
{
    if (!m_device)
    {
        return false;
    }

    TextureResource texture;
    ComPtr<ID3D11Resource> textureResource;
    const HRESULT hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),
        filePath.c_str(),
        textureResource.GetAddressOf(),
        texture.shaderResourceView.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    m_textures[resourceId] = texture;
    return true;
}

bool ResourceManager::CreateTexture2D(
    const std::string& resourceId,
    UINT width,
    UINT height,
    const std::vector<std::uint32_t>& pixels)
{
    if (width == 0 || height == 0 || pixels.size() < static_cast<size_t>(width) * static_cast<size_t>(height))
    {
        return false;
    }

    return CreateTextureInternal(resourceId, width, height, pixels.data());
}

const ShaderProgramResource* ResourceManager::GetShaderProgram(const std::string& resourceId) const
{
    const auto it = m_shaderPrograms.find(resourceId);
    if (it == m_shaderPrograms.end())
    {
        return nullptr;
    }

    return &it->second;
}

const MeshResource* ResourceManager::GetMesh(const std::string& resourceId) const
{
    const auto it = m_meshes.find(resourceId);
    if (it == m_meshes.end())
    {
        return nullptr;
    }

    return &it->second;
}

const TextureResource* ResourceManager::GetTexture(const std::string& resourceId) const
{
    const auto it = m_textures.find(resourceId);
    if (it == m_textures.end())
    {
        return nullptr;
    }

    return &it->second;
}

bool ResourceManager::CompileShader(
    const std::wstring& filePath,
    const char* entryPoint,
    const char* shaderModel,
    ComPtr<ID3DBlob>& shaderBlob) const
{
    if (!m_device)
    {
        return false;
    }

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    const HRESULT hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        compileFlags,
        0,
        shaderBlob.GetAddressOf(),
        errorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cout << "Shader compile error: "
                      << static_cast<const char*>(errorBlob->GetBufferPointer())
                      << std::endl;
        }
        else
        {
            std::cout << "Failed to compile shader." << std::endl;
        }

        return false;
    }

    return true;
}

bool ResourceManager::CreateMeshInternal(
    const std::string& resourceId,
    const void* vertices,
    UINT vertexCount,
    UINT vertexStride,
    const UINT* indices,
    UINT indexCount,
    D3D11_PRIMITIVE_TOPOLOGY topology)
{
    if (!m_device || !vertices || !indices || vertexCount == 0 || indexCount == 0 || vertexStride == 0)
    {
        return false;
    }

    MeshResource mesh;
    mesh.vertexStride = vertexStride;
    mesh.indexCount = indexCount;
    mesh.topology = topology;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = vertexStride * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.StructureByteStride = vertexStride;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    HRESULT hr = m_device->CreateBuffer(
        &vertexBufferDesc,
        &vertexData,
        mesh.vertexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(UINT) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    hr = m_device->CreateBuffer(
        &indexBufferDesc,
        &indexData,
        mesh.indexBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    m_meshes[resourceId] = mesh;
    return true;
}

bool ResourceManager::CreateTextureInternal(
    const std::string& resourceId,
    UINT width,
    UINT height,
    const void* pixels)
{
    if (!m_device || !pixels || width == 0 || height == 0)
    {
        return false;
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = pixels;
    textureData.SysMemPitch = width * sizeof(std::uint32_t);

    ComPtr<ID3D11Texture2D> texture2D;
    HRESULT hr = m_device->CreateTexture2D(
        &textureDesc,
        &textureData,
        texture2D.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    TextureResource texture;
    hr = m_device->CreateShaderResourceView(
        texture2D.Get(),
        nullptr,
        texture.shaderResourceView.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    m_textures[resourceId] = texture;
    return true;
}
