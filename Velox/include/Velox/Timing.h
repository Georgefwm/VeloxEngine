#pragma once

#include <Velox.h>

#include <functional>

namespace Velox {

VELOX_API f64 getDeltaTime();

VELOX_API void calculateDeltaTime();

// Takes a function like "void update(double& dt)"
VELOX_API void updateGame(std::function<void(const double&)> updateCallback);

void initTimer();

}
