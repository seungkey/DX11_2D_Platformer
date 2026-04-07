#include "pch.h"
#include "SpriteAnimationComponent.h"

#include "Entity.h"
#include "RenderComponent.h"

void SpriteAnimationComponent::Update(Entity& owner, float dt)
{
    auto* renderComponent = owner.GetComponent<RenderComponent>();
    if (!renderComponent)
    {
        return;
    }

    renderComponent->textureId = textureId;

    const bool hasCustomFrames = !m_frames.empty();
    if (!hasCustomFrames && (columns <= 0 || rows <= 0 || frameCount <= 0))
    {
        renderComponent->uvOffset = Vector2(0.0f, 0.0f);
        renderComponent->uvScale = Vector2(1.0f, 1.0f);
        return;
    }

    if (playing && framesPerSecond > 0.0f && dt > 0.0f)
    {
        const float frameDuration = 0.15f;
        m_elapsedTime += dt;

        while (m_elapsedTime >= frameDuration)
        {
            m_elapsedTime -= frameDuration;
            AdvanceFrame();
        }
    }

    ApplyFrame(*renderComponent);
}

void SpriteAnimationComponent::SetFrames(const std::vector<SpriteAnimationFrame>& frames)
{
    m_frames = frames;
    m_currentFrame = 0;
    m_elapsedTime = 0.0f;

    if (!m_frames.empty())
    {
        startFrame = 0;
        frameCount = static_cast<int>(m_frames.size());
    }
}

void SpriteAnimationComponent::AdvanceFrame()
{
    const int activeFrameCount = !m_frames.empty() ? static_cast<int>(m_frames.size()) : frameCount;
    if (activeFrameCount <= 0)
    {
        return;
    }

    ++m_currentFrame;
    if (m_currentFrame < activeFrameCount)
    {
        return;
    }

    if (looping)
    {
        m_currentFrame = 0;
        return;
    }

    m_currentFrame = activeFrameCount - 1;
    playing = false;
}

void SpriteAnimationComponent::ApplyFrame(RenderComponent& renderComponent) const
{
    if (!m_frames.empty())
    {
        const int frameIndex = (m_currentFrame < static_cast<int>(m_frames.size()))
                                   ? m_currentFrame
                                   : static_cast<int>(m_frames.size()) - 1;
        renderComponent.uvOffset = m_frames[frameIndex].uvOffset;
        renderComponent.uvScale = m_frames[frameIndex].uvScale;
        return;
    }

    const float frameWidth = 1.0f / static_cast<float>(columns);
    const float frameHeight = 1.0f / static_cast<float>(rows);
    const int frameIndex = startFrame + m_currentFrame;
    const int frameColumn = frameIndex % columns;
    const int frameRow = frameIndex / columns;

    renderComponent.uvOffset = Vector2(
        frameColumn * frameWidth,
        frameRow * frameHeight);
    renderComponent.uvScale = Vector2(frameWidth, frameHeight);
}
