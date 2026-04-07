#include "Header.hlsli"

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
    return SpriteTexture.Sample(SpriteSampler, input.UV) * Color;
}
