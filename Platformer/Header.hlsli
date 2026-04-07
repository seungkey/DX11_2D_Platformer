cbuffer ConstantBuffer : register(b0)
{
    matrix Model;
    matrix View;
    matrix Proj;
    float4 Color;
    float4 UVRect;
};

struct VS_INPUT
{
    float2 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};
