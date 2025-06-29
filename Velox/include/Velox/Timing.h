#pragma once

#include <functional>

namespace Velox {

f64 DeltaTime();

void CalculateDeltaTime();

// Takes a function like "void update(double& dt)"
void UpdateGame(std::function<void(double&)> updateCallback);

void InitTimer();

}
