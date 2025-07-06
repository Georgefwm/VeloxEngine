#pragma once

namespace Velox {
struct Entity;
};

void setupScrollingBackground();

void updateScrollingBackground(Velox::Entity& e, const double& deltaTime);
