#pragma once

#include "Core.h"

namespace Velox {

// Converts pixel coords to normalised shader coords (-1...1).
// Currently ignores z value because we aren't using it yet.
vec3 ToShaderCoords(vec3 vector, bool flipY);

float DeltaTime();

double DeltaTimePrecise();

float RefreshDeltaTime();

}
