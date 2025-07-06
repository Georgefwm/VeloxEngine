#pragma once

#include "Velox.h"

SDL_EVENT_FWD_DECL

#include <Entity.h>


void setupPlane();

void updatePlane(Velox::Entity& e, const double& deltaTime);

