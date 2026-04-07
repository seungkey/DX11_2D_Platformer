#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <directxtk/SimpleMath.h>

class InputManager
{
public:
    static InputManager* GetInstance();

    void Initialize(HWND hwnd);
    void Update();

    void AddActionBinding(const std::string& actionName, int virtualKey);
    void ClearActionBindings(const std::string& actionName);

    bool IsKeyDown(int virtualKey) const;
    bool IsKeyPressed(int virtualKey) const;
    bool IsKeyReleased(int virtualKey) const;

    bool IsActionDown(const std::string& actionName) const;
    bool IsActionPressed(const std::string& actionName) const;
    bool IsActionReleased(const std::string& actionName) const;

    float GetAxis(const std::string& negativeAction, const std::string& positiveAction) const;
    bool HasFocus() const { return m_hasFocus; }

private:
    InputManager() = default;

    bool IsValidKey(int virtualKey) const;
    void ResetStates();
    bool IsActionTriggered(
        const std::string& actionName,
        bool (InputManager::*stateCheck)(int virtualKey) const) const;

    static std::unique_ptr<InputManager> s_instance;

    HWND m_hwnd = nullptr;
    bool m_hasFocus = false;
    std::array<bool, 256> m_currentKeys{};
    std::array<bool, 256> m_previousKeys{};
    std::unordered_map<std::string, std::vector<int>> m_actionBindings;
};

