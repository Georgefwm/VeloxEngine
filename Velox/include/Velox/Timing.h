#pragma once

#include <functional>

namespace Velox {

f64 getDeltaTime();

void calculateDeltaTime();

// Takes a function like "void update(double& dt)"
void updateGame(std::function<void(double&)> updateCallback);

void initTimer();

}
