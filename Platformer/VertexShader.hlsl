#include "Header.hlsli"

PS_INPUT VS_Main(VS_INPUT input)
{
    PS_INPUT output;
    float4 pos = float4(input.Pos, 0.0f, 1.0f);
    pos = mul(pos, Model);
    pos = mul(pos, View);
    pos = mul(pos, Proj);
    output.Pos = pos;
    output.UV = UVRect.xy + (input.UV * UVRect.zw);
    return output;
}
