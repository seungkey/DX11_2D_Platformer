#pragma once
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "Component.h"
#include "TransformComponent.h"
#include "BoxColliderComponent.h"
#include "RenderComponent.h"

class Entity
{
public:
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        m_components[typeid(T)] = std::move(component);
        return *ptr;
    }

    template <typename T>
    T* GetComponent()
    {
        auto it = m_components.find(typeid(T));
        if (it != m_components.end())
        {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    void UpdateComponents()
    {
        for (auto& componentPair : m_components)
        {
            auto& component = componentPair.second;
            if (component)
            {
                component->Update(*this);
            }
        }
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
};
