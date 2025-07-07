#pragma once

namespace Velox {
struct Entity;
}

void setupObstacles(f32 gapOffset);

void updateObstacles(Velox::Entity& e, const double& deltaTime);

void setupSpawner();

void updateSpawner(Velox::Entity& e, const double& deltaTime);


