#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <wrl.h>

#include <d3d11.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

struct ShaderProgramResource
{
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3DBlob> vertexShaderBlob;
};

struct MeshResource
{
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT vertexStride = 0;
    UINT indexCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct TextureResource
{
    ComPtr<ID3D11ShaderResourceView> shaderResourceView;
};

class ResourceManager
{
public:
    bool Initialize(ID3D11Device* device);
    void Shutdown();

    bool LoadShaderProgram(
        const std::string& resourceId,
        const std::wstring& vertexShaderPath,
        const std::wstring& pixelShaderPath,
        const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc,
        UINT inputLayoutCount,
        const char* vertexEntryPoint = "VS_Main",
        const char* pixelEntryPoint = "PS_Main",
        const char* vertexShaderModel = "vs_4_0",
        const char* pixelShaderModel = "ps_4_0");

    bool LoadTexture2D(
        const std::string& resourceId,
        const std::wstring& filePath);

    bool CreateTexture2D(
        const std::string& resourceId,
        UINT width,
        UINT height,
        const std::vector<std::uint32_t>& pixels);

    template <typename TVertex>
    bool CreateMesh(
        const std::string& resourceId,
        const std::vector<TVertex>& vertices,
        const std::vector<UINT>& indices,
        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    {
        if (vertices.empty() || indices.empty())
        {
            return false;
        }

        return CreateMeshInternal(
            resourceId,
            vertices.data(),
            static_cast<UINT>(vertices.size()),
            sizeof(TVertex),
            indices.data(),
            static_cast<UINT>(indices.size()),
            topology);
    }

    const ShaderProgramResource* GetShaderProgram(const std::string& resourceId) const;
    const MeshResource* GetMesh(const std::string& resourceId) const;
    const TextureResource* GetTexture(const std::string& resourceId) const;

private:
    bool CompileShader(
        const std::wstring& filePath,
        const char* entryPoint,
        const char* shaderModel,
        ComPtr<ID3DBlob>& shaderBlob) const;

    bool CreateMeshInternal(
        const std::string& resourceId,
        const void* vertices,
        UINT vertexCount,
        UINT vertexStride,
        const UINT* indices,
        UINT indexCount,
        D3D11_PRIMITIVE_TOPOLOGY topology);

    bool CreateTextureInternal(
        const std::string& resourceId,
        UINT width,
        UINT height,
        const void* pixels);

    ComPtr<ID3D11Device> m_device;
    std::unordered_map<std::string, ShaderProgramResource> m_shaderPrograms;
    std::unordered_map<std::string, MeshResource> m_meshes;
    std::unordered_map<std::string, TextureResource> m_textures;
};
