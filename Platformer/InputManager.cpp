#include "pch.h"
#include "InputManager.h"

#include <algorithm>

std::unique_ptr<InputManager> InputManager::s_instance = nullptr;

InputManager* InputManager::GetInstance()
{
    if (!s_instance)
    {
        s_instance.reset(new InputManager());
    }

    return s_instance.get();
}

void InputManager::Initialize(HWND hwnd)
{
    m_hwnd = hwnd;
    ResetStates();
}

void InputManager::Update()
{
    m_previousKeys = m_currentKeys;

    if (!m_hwnd || GetForegroundWindow() != m_hwnd)
    {
        m_hasFocus = false;
        ResetStates();
        return;
    }

    m_hasFocus = true;

    for (int virtualKey = 0; virtualKey < static_cast<int>(m_currentKeys.size()); ++virtualKey)
    {
        m_currentKeys[virtualKey] = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
    }
}

void InputManager::AddActionBinding(const std::string& actionName, int virtualKey)
{
    if (!IsValidKey(virtualKey))
    {
        return;
    }

    auto& bindings = m_actionBindings[actionName];
    if (std::find(bindings.begin(), bindings.end(), virtualKey) == bindings.end())
    {
        bindings.push_back(virtualKey);
    }
}

void InputManager::ClearActionBindings(const std::string& actionName)
{
    m_actionBindings.erase(actionName);
}

bool InputManager::IsKeyDown(int virtualKey) const
{
    return IsValidKey(virtualKey) && m_currentKeys[virtualKey];
}

bool InputManager::IsKeyPressed(int virtualKey) const
{
    return IsValidKey(virtualKey) && m_currentKeys[virtualKey] && !m_previousKeys[virtualKey];
}

bool InputManager::IsKeyReleased(int virtualKey) const
{
    return IsValidKey(virtualKey) && !m_currentKeys[virtualKey] && m_previousKeys[virtualKey];
}

bool InputManager::IsActionDown(const std::string& actionName) const
{
    return IsActionTriggered(actionName, &InputManager::IsKeyDown);
}

bool InputManager::IsActionPressed(const std::string& actionName) const
{
    return IsActionTriggered(actionName, &InputManager::IsKeyPressed);
}

bool InputManager::IsActionReleased(const std::string& actionName) const
{
    return IsActionTriggered(actionName, &InputManager::IsKeyReleased);
}

float InputManager::GetAxis(const std::string& negativeAction, const std::string& positiveAction) const
{
    float axis = 0.0f;

    if (IsActionDown(negativeAction))
    {
        axis -= 1.0f;
    }

    if (IsActionDown(positiveAction))
    {
        axis += 1.0f;
    }

    return axis;
}

bool InputManager::IsValidKey(int virtualKey) const
{
    return virtualKey >= 0 && virtualKey < static_cast<int>(m_currentKeys.size());
}

void InputManager::ResetStates()
{
    m_currentKeys.fill(false);
    m_previousKeys.fill(false);
}

bool InputManager::IsActionTriggered(
    const std::string& actionName,
    bool (InputManager::*stateCheck)(int virtualKey) const) const
{
    const auto it = m_actionBindings.find(actionName);
    if (it == m_actionBindings.end())
    {
        return false;
    }

    for (const int virtualKey : it->second)
    {
        if ((this->*stateCheck)(virtualKey))
        {
            return true;
        }
    }

    return false;
}
