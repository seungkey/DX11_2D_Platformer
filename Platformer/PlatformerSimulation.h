#pragma once

#include <memory>
#include <vector>

class Entity;

class PlatformerSimulation
{
public:
    static void ResolveHorizontalCollisions(
        Entity& player,
        const std::vector<std::shared_ptr<Entity>>& platforms);

    static void ResolveVerticalCollisions(
        Entity& player,
        const std::vector<std::shared_ptr<Entity>>& platforms);

private:
    static void MarkCollision(Entity& player, Entity& platform);
};
