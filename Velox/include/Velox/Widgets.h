#pragma once
#include <Velox.h>

#include "UI.h"

namespace UI {

UI::Comm spacing(f32 layoutAxisSpacing = 10.0f);
UI::Comm section(std::string string = "");
UI::Comm window(Velox::Rectangle rect, std::string label);
UI::Comm button(std::string string);
UI::Comm text(std::string string);

}
