#pragma once

#include <string>

#include "Component.h"

class RenderComponent : public Component
{
public:
    bool visible;
    std::string shaderId;
    std::string meshId;
    std::string textureId;
    Vector4 color;
    Vector2 uvOffset;
    Vector2 uvScale;

    RenderComponent(
        bool v = true,
        const std::string& inShaderId = "default",
        const std::string& inMeshId = "quad",
        const std::string& inTextureId = "white",
        const Vector4& inColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f),
        const Vector2& inUvOffset = Vector2(0.0f, 0.0f),
        const Vector2& inUvScale = Vector2(1.0f, 1.0f))
        : visible(v),
          shaderId(inShaderId),
          meshId(inMeshId),
          textureId(inTextureId),
          color(inColor),
          uvOffset(inUvOffset),
          uvScale(inUvScale)
    {
    }
};
