#include "Header.hlsli"

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Proj;
}


PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    // Add the offset from the constant buffer to the vertex position
    output.Pos = mul(float4(input.Pos, 0.f, 1.f), World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Proj);
    return output;
}