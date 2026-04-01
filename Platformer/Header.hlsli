cbuffer ConstantBuffer : register(b0)
{
    matrix Model;
    matrix View;
    matrix Proj;
    float4 Color;
};

struct VS_INPUT
{
    float2 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};
