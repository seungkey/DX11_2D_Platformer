#pragma once

#include <string>
#include <vector>

#include "Component.h"

class Entity;
class RenderComponent;

struct SpriteAnimationFrame
{
    Vector2 uvOffset;
    Vector2 uvScale;

    SpriteAnimationFrame(
        const Vector2& inUvOffset = Vector2(0.0f, 0.0f),
        const Vector2& inUvScale = Vector2(1.0f, 1.0f))
        : uvOffset(inUvOffset),
          uvScale(inUvScale)
    {
    }
};

class SpriteAnimationComponent : public Component
{
public:
    std::string textureId;
    int columns;
    int rows;
    int startFrame;
    int frameCount;
    float framesPerSecond;
    bool looping;
    bool playing;

    SpriteAnimationComponent(
        const std::string& inTextureId = "player_test",
        int inColumns = 4,
        int inRows = 1,
        int inStartFrame = 0,
        int inFrameCount = 4,
        float inFramesPerSecond = 8.0f,
        bool inLooping = true)
        : textureId(inTextureId),
          columns(inColumns),
          rows(inRows),
          startFrame(inStartFrame),
          frameCount(inFrameCount),
          framesPerSecond(inFramesPerSecond),
          looping(inLooping),
          playing(true),
          m_elapsedTime(0.0f),
          m_currentFrame(0)
    {
    }

    void Update(Entity& owner, float dt = 0.0f) override;
    void SetFrames(const std::vector<SpriteAnimationFrame>& frames);

private:
    void AdvanceFrame();
    void ApplyFrame(RenderComponent& renderComponent) const;

    float m_elapsedTime;
    int m_currentFrame;
    std::vector<SpriteAnimationFrame> m_frames;
};
