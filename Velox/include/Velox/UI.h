#pragma once

/*
 * UI system very heavily inspired from Ryan Fluery @ EpicGameTools
 *
 */

#include <Velox.h>
#include "Arena.h"
#include "Types.h"
#include <stack>

SDL_EVENT_FWD_DECL
struct ImDrawData;

// #define UI_DEFER_LOOP(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

namespace Velox {
struct Event;
struct Font;
}

namespace UI {

enum Axis2 : i32 {
    Axis2_Invalid = -1,
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT,
};

enum TextAlignment {
    TextAlignment_Left,
    TextAlignment_Center,
    TextAlignment_Right,
    TextAlignment_COUNT,
};

typedef u64 BoxFlags;
enum UIBoxFlags_ : u64 {
    UIBoxFlags_Floating        = (UI::BoxFlags)(1ull << 0),
    UIBoxFlags_Clickable       = (UI::BoxFlags)(1ull << 1),
    UIBoxFlags_DrawBorder      = (UI::BoxFlags)(1ull << 2),
    UIBoxFlags_DrawText        = (UI::BoxFlags)(1ull << 3),
    UIBoxFlags_DrawBackground  = (UI::BoxFlags)(1ull << 4),
    UIBoxFlags_HotAnimation    = (UI::BoxFlags)(1ull << 5),
    UIBoxFlags_ActiveAnimation = (UI::BoxFlags)(1ull << 6),
    UIBoxFlags_FixedWidth      = (UI::BoxFlags)(1ull << 7),
    UIBoxFlags_FixedHeight     = (UI::BoxFlags)(1ull << 8),
    UIBoxFlags_AllowOverflowX  = (UI::BoxFlags)(1ull << 9),
    UIBoxFlags_AllowOverflowY  = (UI::BoxFlags)(1ull << 10),
};

struct Key {
    u64 U64[1];
};

Key keyNull();
Key keyFromString(std::string string);
bool keyComp(Key a, Key b);

enum SizeKind {
    UISizeKind_Null,
    UISizeKind_Pixels,
    UISizeKind_TextContent,
    UISizeKind_ParentPct,
    UISizeKind_ChildrenSum,
};

struct Size {
    UI::SizeKind kind = UISizeKind_Pixels;
    f32 value = 0.0f;
    f32 strictness = 1.0f;
};

struct Box {
    // tree links
    UI::Box* first; // first child
    UI::Box* last;  // last child
    UI::Box* next;  // next sibling
    UI::Box* prev;  // prev sibling
    UI::Box* parent;
    u64 childCount;

    // hash links
    UI::Box* nextHash;
    UI::Box* prevHash;

    i32 pushCount;
    i32 popCount;

    // key+generation info
    UI::Key key = keyNull();
    u64 lastFrameTouchedIndex;
    std::string name;

    // per-frame info provided by builders
    UI::BoxFlags flags;
    std::string string;
    UI::Size preferredSize[Axis2_COUNT];
    f32 preferredPosition[Axis2_COUNT];
    UI::Size fixedSize[Axis2_COUNT];
    UI::Size fixedPosition[Axis2_COUNT];
    UI::Size minSize[Axis2_COUNT];
    UI::Axis2 childLayoutAxis;

    // computed every frame
    f32 computedRelPosition[Axis2_COUNT];
    f32 computedSize[Axis2_COUNT];
    Velox::Rectangle rect;

    // persistent data
    f32 hot_t;
    f32 active_t;

    // styling
    Velox::Font* font;
    f32 fontSize;
    vec4 fontColor;

    vec4 backgroundColor;
    vec4 hoverColor;
    vec4 usingColor = backgroundColor;

    vec2 padding = vec2(0.0f);
    
    UI::TextAlignment textAlignment;
};

UI::Box* buildBoxFromString(UI::BoxFlags flags, std::string string);
UI::Box* buildBoxFromKey(UI::BoxFlags flags, UI::Key key);
UI::Box* boxFromKey(UI::Key key);

void boxEquipDisplayString(UI::Box* widget, std::string string);
void boxEquipChildLayoutAxis(UI::Box* widget, Axis2 axis);

// managing the parent stack
void pushParent(UI::Box* widget);
UI::Box* popParent();
UI::Box* topParent();

struct Comm
{
    UI::Box* box;

    vec2 mouse;
    vec2 drag_delta;
    bool clicked;
    bool double_clicked;
    bool right_clicked;
    bool pressed;
    bool released;
    bool dragging;
    bool hovering;

    // For window position.
    vec2 windowPositionRequest;
};

UI::Comm commFromBox(UI::Box* box);

struct UIState {
    Velox::Arena arena = Velox::Arena(64000);

    i32 buildBoxCount = 0;
    i32 prevBuildBoxCount = 0;

    UI::Box* root;
    UI::Box* firstFreeBox;

    UI::Key hotBoxKey;
    std::unordered_map<u8, UI::Key> activeBoxKey {};

    std::stack<UI::Box*> boxStack {};
    std::stack<UI::Box*> parentStack {};

    std::stack<Velox::Font*> fontStack {};
    std::stack<f32> fontSizeStack {};
    std::stack<vec4> fontColorStack {};
    std::stack<UI::TextAlignment> textAlignmentStack {};

    bool debug = false;
};

void beginBuild();
void endBuild();

void drawBoxes();
}

namespace Velox {

void initUI();

bool uiEventCallback(SDL_Event& event);

// Must be called after Velox::EndFrame()!
ImDrawData* getUIDrawData();

void deInitUI();

}
