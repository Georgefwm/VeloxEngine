#include "Widgets.h"
#include <PCH.h>

#include "Types.h"
#include "UI.h"


UI::Comm UI::spacing(f32 layoutAxisSpacing)
{
    UI::BoxFlags flags = 0;

    UI::Box* box = UI::buildBoxFromString(flags, "");

    box->name = "spacer";
    
    box->preferredSize[box->parent->childLayoutAxis].kind = UI::UISizeKind_Pixels;
    box->preferredSize[box->parent->childLayoutAxis].value = layoutAxisSpacing;

    UI::Comm comm = UI::commFromBox(box);

    return comm;
}

UI::Comm UI::section(std::string string)
{
    UI::BoxFlags flags = 0;

    UI::Box* box = UI::buildBoxFromString(flags, string);

    UI::pushParent(box);

    box->name = "section";
    
    box->preferredSize[Axis2_X].kind = UI::UISizeKind_ParentPct;
    box->preferredSize[Axis2_X].value = 1.0f;

    box->preferredSize[Axis2_Y].kind = UI::UISizeKind_ChildrenSum;
    box->preferredSize[Axis2_Y].value = 1.0f;


    UI::Comm comm = UI::commFromBox(box);

    return comm;
}

UI::Comm UI::window(Velox::Rectangle rect, std::string label)
{
    UI::BoxFlags flags = UIBoxFlags_DrawBackground | UIBoxFlags_Floating;

    UI::Box* box = UI::buildBoxFromString(flags, label);

    UI::pushParent(box);

    box->name = label;

    box->preferredPosition[UI::Axis2_X] = rect.x;
    box->preferredPosition[UI::Axis2_Y] = rect.y;

    box->preferredSize[UI::Axis2_X].kind = UI::UISizeKind_Pixels;
    box->preferredSize[UI::Axis2_X].value = rect.w;

    box->preferredSize[UI::Axis2_Y].kind = UI::UISizeKind_Pixels;
    box->preferredSize[UI::Axis2_Y].value = rect.h;

    box->usingColor = COLOR_GRAY_DARK;

    UI::Comm comm = UI::commFromBox(box);

    return comm;
}

UI::Comm UI::button(std::string string)
{
    UI::BoxFlags flags =
        UIBoxFlags_DrawText |
        UIBoxFlags_Clickable |
        UIBoxFlags_DrawBackground;

    UI::Box* box = UI::buildBoxFromString(flags, string);

    box->name = string;

    box->preferredSize[UI::Axis2_X].kind = UI::UISizeKind_TextContent;
    box->preferredSize[UI::Axis2_Y].kind = UI::UISizeKind_TextContent;

    box->backgroundColor = COLOR_GRAY_MEDIUM;
    box->hoverColor      = COLOR_GRAY_LIGHT;
    box->usingColor      = box->backgroundColor;

    UI::Comm comm = UI::commFromBox(box);

    return comm;
}

UI::Comm UI::text(std::string string)
{
    UI::BoxFlags flags =
        UIBoxFlags_DrawText |
        UIBoxFlags_WrapText;

    UI::Box* box = UI::buildBoxFromString(flags, string);

    box->name = string;

    box->preferredSize[UI::Axis2_X].kind = UI::UISizeKind_ParentPct;
    box->preferredSize[UI::Axis2_X].value = 1.0f;

    box->preferredSize[UI::Axis2_Y].kind = UI::UISizeKind_TextContent;

    UI::Comm comm = UI::commFromBox(box);

    return comm;
}

