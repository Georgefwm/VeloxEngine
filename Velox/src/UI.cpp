#include "UI.h"
#include <PCH.h>

#include "Arena.h"
#include "Asset.h"
#include "Input.h"
#include "Rendering/Renderer.h"
#include "Event.h"

#include <SDL3/SDL.h>

#include <fmt/format.h>
#include <imgui.h>
#include "Text.h"
#include "Types.h"
#include "Util.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <xxhash.h>


static UI::UIState s_uiState;


UI::Key UI::keyNull()
{
    return { 0 };
}

std::string hashPartFromKeyString(std::string string)
{
    std::string result = string;

    size_t hashReplaceIndex = string.find("###");
    if (hashReplaceIndex < string.size())
        result = string.substr(0, hashReplaceIndex);

    return result;
}

UI::Key UI::keyFromString(std::string string)
{
    UI::Key result = { 0 };

    if (string.size() != 0)
    {   
        std::string hashPart = hashPartFromKeyString(string);
        result.U64[0] = XXH64(hashPart.data(), sizeof(hashPart.data()), (XXH64_hash_t)0);
    }

    return result;
}

bool UI::keyComp(UI::Key a, UI::Key b)
{
    return a.U64[0] == b.U64[0];
}

void setParent(UI::Box* box, UI::Box* parent)
{
    parent->childCount += 1;
    box->parent = parent;

    if (parent->first == nullptr)
    {
        parent->first = box;
        parent->last = box;

        return;
    }

    UI::Box* lastChild = parent->last;

    lastChild->next = box;
    box->prev = lastChild;

    parent->last = box;
}

UI::Box* UI::buildBoxFromString(UI::BoxFlags flags, std::string string)
{
    UI::Box* parent = UI::topParent();

    UI::Key key = UI::keyFromString(string);

    UI::Box* box = UI::buildBoxFromKey(flags, key);

    if (parent != nullptr)
        setParent(box, parent);

    if (flags & UIBoxFlags_DrawText)
    {
        UI::boxEquipDisplayString(box, string);
    }

    return box;
}

UI::Box* UI::buildBoxFromKey(UI::BoxFlags flags, UI::Key key)
{
    s_uiState.buildBoxCount += 1;

    UI::Box* parent = UI::topParent();

    UI::Box* box;
    // Cache stuff, ignore for now.
    // UI::BoxFlags lastFlags = 0;
    // UI::Box* box = UI::boxFromKey(key);
    // bool boxFirstBuildFrame = box == nullptr;
    // Ignore caching result
    bool boxFirstBuildFrame = true;

    bool boxIsTransient = UI::keyComp(key, UI::keyNull());

    if (boxFirstBuildFrame)
    {
        // box = !boxIsTransient ? s_uiState.firstFreeBox : nullptr;
        // box = s_uiState.firstFreeBox;

        //if (box != nullptr)
        //{
        //    s_uiState.boxStack.pop();
        //}
        //else
        //{
        //    box = s_uiState.arena.alloc<UI::Box>(1);
        //}

        box = s_uiState.arena.alloc<UI::Box>(1);

        // zero memory.
        std::memset((void*)box, 0, sizeof(UI::Box));

        box = new(box) UI::Box();
    }

    box->first = box->last = box->next = box->prev = box->parent = nullptr;
    box->childCount = 0;

    box->flags = 0;
    box->flags |= flags;

    // std::memset((void*)&box->preferredSize, 0, sizeof(UI::Size));
    // std::memset(&box->drawBucket, 0, sizeof(drawBucket));

    if (parent != nullptr)
    {
        parent->childCount += 1;
        box->parent = parent;
    }

    // Fill box
    box->key = key;

    if (boxFirstBuildFrame && !boxIsTransient)
    {
        
    }

    box->font          = s_uiState.fontStack.top();
    box->fontSize      = s_uiState.fontSizeStack.top();
    box->fontColor     = s_uiState.fontColorStack.top();
    box->textAlignment = s_uiState.textAlignmentStack.top();

    box->backgroundColor = COLOR_GRAY_MEDIUM;
    box->hoverColor      = COLOR_GRAY_LIGHT;

    return box;
}

UI::Box* boxFromKey(UI::Key key)
{
    return nullptr;
}

void UI::boxEquipDisplayString(UI::Box* box, std::string string)
{
    box->string = string;

    box->font = s_uiState.fontStack.top();
    box->fontSize = s_uiState.fontSizeStack.top();
    box->fontColor = s_uiState.fontColorStack.top();
    box->textAlignment = s_uiState.textAlignmentStack.top();
}

void UI::boxEquipChildLayoutAxis(UI::Box* box, UI::Axis2 axis)
{
}

void UI::pushParent(UI::Box* box)
{
    s_uiState.parentStack.push(box);
}

UI::Box* UI::popParent()
{
    UI::Box* parent = s_uiState.parentStack.top();

    if (s_uiState.parentStack.size() > 0)
        s_uiState.parentStack.pop();
    else
        LOG_WARN("Called pop on topmost parent, should not happen");

    return parent;
}

UI::Box* UI::topParent()
{
    if (s_uiState.parentStack.size() < 1)
        return nullptr;
    
    return s_uiState.parentStack.top();
}

UI::Comm UI::commFromBox(UI::Box* box)
{
    UI::Comm comm = { box };

    bool taken = false;

    bool mouseInBox = Velox::isMouseInArea(box->rect);

    bool mouse1Pressed = Velox::isMouseButtonPressed(1);  // 1 == left mouse button;
    bool mouse1Released = !Velox::isMouseButtonPressed(1);  // 1 == left mouse button;
    
    // Mouse clicked in box.
    if (box->flags & UIBoxFlags_Clickable && mouseInBox && mouse1Pressed)
    {
        s_uiState.hotBoxKey = box->key;
        s_uiState.activeBoxKey[1] = box->key;
        comm.pressed = true;

        taken = true;
    }

    // Mouse released inside box.
    if (box->flags & UIBoxFlags_Clickable && mouseInBox && mouse1Released)
    {
        s_uiState.activeBoxKey[1] = UI::keyNull();
        comm.clicked = true;
        comm.released = true;

        taken = true;
    }

    // Mouse release outside of box.
    if (box->flags & UIBoxFlags_Clickable && !mouseInBox && mouse1Released)
    {
        s_uiState.activeBoxKey[1] = UI::keyNull();
        comm.released = true;

        taken = true;
    }

    if (mouseInBox)
    {
        comm.hovering = true;
    }

    return comm;
}

void UI::beginBuild()
{
    s_uiState.arena.reset();

    s_uiState.parentStack = {};

    s_uiState.fontStack = {};
    s_uiState.fontStack.push(Velox::getAssetManager()->getFontRef("spicy_kebab.ttf"));

    s_uiState.fontSizeStack = {};
    s_uiState.fontSizeStack.push(80);

    s_uiState.fontColorStack = {};
    s_uiState.fontColorStack.push(COLOR_WHITE);

    s_uiState.textAlignmentStack = {};
    s_uiState.textAlignmentStack.push(UI::TextAlignment_Left);

    s_uiState.root = nullptr;
    s_uiState.prevBuildBoxCount = s_uiState.buildBoxCount;
    s_uiState.buildBoxCount = 0;

    ivec2 windowSize = Velox::getWindowSize();
    Velox::Rectangle windowRect = { 0, 0, (f32)windowSize.x, (f32)windowSize.y };

    // Setup root.
    u64 someWindowID = 202983740;
    UI::Box* root = UI::buildBoxFromString(0, fmt::format("###{}", someWindowID));

    root->name = "root";
    root->childLayoutAxis = Axis2_X;
    root->preferredSize[Axis2_X] = { UISizeKind_Pixels, (f32)windowSize.x, 1.0f };
    root->preferredSize[Axis2_Y] = { UISizeKind_Pixels, (f32)windowSize.y, 1.0f };

    UI::pushParent(root);
    s_uiState.root = root;
}

void fillTextStyleFromBox(UI::Box* box, Velox::TextDrawStyle* style)
{
    style->font = box->font;
    style->textSize = box->fontSize;
    style->color = box->fontColor;
    style->wrapText = true;
    style->wrapXSize = box->textWrapSize;
}

UI::Box* boxDepthFirstPreOrder(UI::Box* box, UI::Box* root)
{
    if (box->first != nullptr) return box->first;

    for (UI::Box* p = box; p != nullptr && p != root; p = p->parent)
        if (p->next != nullptr) return p->next;

    return nullptr;
}

UI::Box* boxDepthFirstPostOrder(UI::Box* box, UI::Box* root)
{
    if (box->last != nullptr) return box->last;

    for (UI::Box* p = box; p != nullptr && p != root; p = p->parent)
        if (p->prev != nullptr) return p->prev;

    return nullptr;
}

void calculateSizesStandalone(UI::Box* root, UI::Axis2 axis)
{
    for (UI::Box* box = root; box != nullptr; box = boxDepthFirstPreOrder(box, root)) 
    {
        switch (box->preferredSize[axis].kind)
        {
            case UI::UISizeKind_Pixels: 
            {
                box->fixedSize[axis].value = box->preferredSize[axis].value;
                break;
            }

            case UI::UISizeKind_TextContent:
            {
                Velox::TextDrawStyle style;
                fillTextStyleFromBox(box, &style);

                vec2 stringSize = Velox::getStringSize(box->string.c_str(), style);

                box->preferredSize[axis].value = stringSize[axis] + (box->padding[axis] * 2.0f);
                box->fixedSize[axis].value     = stringSize[axis] + (box->padding[axis] * 2.0f);

                break;
            }
                
            default: break;
        }
    }
}

void calculateSizesUpwardsDependant(UI::Box* root, UI::Axis2 axis)
{
    for (UI::Box* box = root; box != nullptr; box = boxDepthFirstPreOrder(box, root)) 
    {
        switch (box->preferredSize[axis].kind)
        {
            case UI::UISizeKind_ParentPct:
            {
                box->fixedSize[axis].value = box->preferredSize[axis].value;

                UI::Box* fixedParent;
                for (UI::Box* parent = box->parent; parent != nullptr; parent = parent->parent)
                {
                    if (parent->flags & UI::UIBoxFlags_FixedWidth ||
                        parent->preferredSize[axis].kind == UI::UISizeKind_Pixels ||
                        parent->preferredSize[axis].kind == UI::UISizeKind_TextContent ||
                        parent->preferredSize[axis].kind == UI::UISizeKind_ParentPct)
                    {
                        fixedParent = parent;
                        break;
                    }
                }

                f32 size =
                    (fixedParent->fixedSize[axis].value - (fixedParent->padding[axis] * 2.0f)) *
                    box->preferredSize[axis].value;

                box->fixedSize[axis].value = size;
            }

            default: break;
        }
    }
}

void calculateSizesDownwardsDependant(UI::Box* root, UI::Axis2 axis)
{
    for (UI::Box* box = root; box != nullptr; box = boxDepthFirstPreOrder(box, root)) 
    {
        switch (box->preferredSize[axis].kind)
        {
            case UI::UISizeKind_ChildrenSum:
            {
                f32 sum = 0.0f;

                for (UI::Box* child = box->first; child != nullptr; child = child->next)
                {
                    if (axis == box->childLayoutAxis)
                        sum += child->fixedSize[axis].value;
                    else
                        sum = glm::max(sum, child->fixedSize[axis].value);
                }

                // Account for padding.
                sum += box->padding[axis] * 2.0f;

                box->fixedSize[axis].value = sum;
            }

            default: break;
        }
    }
}

void enforceLayoutConstraints(UI::Box* root, UI::Axis2 axis)
{
    for (UI::Box* box = root; box != nullptr; box = boxDepthFirstPreOrder(box, root)) 
    {
        // Fixup child sizes along non-layout axis.
        if (axis != box->childLayoutAxis && !(box->flags & (UI::UIBoxFlags_AllowOverflowX << axis)))
        {
            f32 allowedSize = box->fixedSize[axis].value;
                
            for (UI::Box* child = box->first; child != nullptr; child = child->next)
            {
                f32 childSize = child->fixedSize[axis].value;
                f32 violation = childSize - allowedSize;
                f32 maxFixup = childSize;
                f32 fixup = glm::clamp(violation, 0.0f, maxFixup);

                if (fixup > 0)
                    child->fixedSize[axis].value -= fixup;
            }
        }

        // Fixup child sizes along layout axis.
        if (axis == box->childLayoutAxis && !(box->flags & (UI::UIBoxFlags_AllowOverflowX << axis)))
        {
            f32 totalAllowedSize = box->fixedSize[axis].value;
            f32 totalSize = 0.0f;
            f32 totalWeightedSize = 0.0f;

            for (UI::Box* child = box->first; child != nullptr; child = child->next)
            {
                totalSize += child->fixedSize[axis].value;
                totalWeightedSize += totalSize * child->preferredSize[axis].strictness;
            }

            f32 violation = totalSize - totalAllowedSize;
            if (violation > 0 && totalWeightedSize > 0)
            {
                f32 childFixupSum = 0.0f;
                f32 childFixups[box->childCount];

                u32 childIndex = 0;
                for (UI::Box* child = box->first; child != nullptr; child = child->next, childIndex++)
                {
                    f32 fixupSizeThisChild = child->fixedSize[axis].value * 
                        (1 - child->preferredSize[axis].strictness);

                    fixupSizeThisChild = fixupSizeThisChild < 0.0f ? 0.0f : fixupSizeThisChild;
                    childFixups[childIndex] = fixupSizeThisChild;
                    childFixupSum += fixupSizeThisChild;
                }

                childIndex = 0;
                for (UI::Box* child = box->first; child != nullptr; child = child->next, childIndex++)
                {
                    f32 fixupPercent = violation / totalWeightedSize;
                    fixupPercent = glm::clamp(fixupPercent, 0.0f, 1.0f);
                    child->fixedSize[axis].value -= childFixups[childIndex] * fixupPercent;
                }
            }
        }

        // Fixup upwards relative sizes.
        if (box->flags & (UI::UIBoxFlags_AllowOverflowX << axis))
        {
            for (UI::Box* child = box->first; child != nullptr; child = child->next)
            {
                if (child->preferredSize[axis].kind != UI::UISizeKind_ParentPct)
                    continue;

                child->fixedSize[axis].value =
                    box->fixedSize[axis].value * child->fixedSize[axis].value;
            }
        }

        // Enforce clamps.
        for (UI::Box* child = box->first; child != nullptr; child = child->next)
        {
            child->fixedSize[axis].value =
                glm::max(child->fixedSize[axis].value, child->minSize[axis].value);
        }
    }
}

#define USE_FLOOR 0

void setLayoutPositions(UI::Box* root, UI::Axis2 axis)
{
    for (UI::Box* box = root; box != nullptr; box = boxDepthFirstPreOrder(box, root)) 
    {
        f32 layoutPosition = box->padding[axis];
        f32 bounds = 0.0f;

        for (UI::Box* child = box->first; child != nullptr; child = child->next)
        {
            f32 originalPosition = axis == UI::Axis2_X ? child->rect.x : child->rect.y;

            if (child->flags & UI::UIBoxFlags_Floating)
            {
                child->fixedPosition[axis].value = child->preferredPosition[axis];
                continue;
            }

            child->fixedPosition[axis].value = layoutPosition;

            if (box->childLayoutAxis == axis)
            {
                layoutPosition += child->fixedSize[axis].value;
                bounds += child->fixedSize[axis].value;
            }
            else
                bounds = glm::max(bounds, child->fixedSize[axis].value); 

            // if animating on axis
            // else

            child->fixedPosition[axis].value = box->fixedPosition[axis].value + child->fixedPosition[axis].value;

            //if (axis == UI::Axis2_X)
            //{
            //    child->fixedPosition[axis].value = box->fixedPosition[axis].value + child->fixedPosition[axis].value;
            //    child->rect.w = box->fixedSize[axis].value;
            //}
            //        // - !(child->flags&(UIBoxFlags_SkipViewOffX<<axis))*floor_f32(box->view_off.v[axis])
            //else
            //{
            //    child->rect.y = box->fixedPosition[axis].value + child->fixedPosition[axis].value;
            //    child->rect.h = box->fixedSize[axis].value;
            //}
    //
            //if (axis == UI::Axis2_X)
            //{
            //    child->rect.w = child->rect.w + child->fixedSize[axis].value;
            //    child->rect.w = box->fixedSize[axis].value;
            //}
            //else
            //{
            //    child->rect.h = child->rect.h + child->fixedSize[axis].value;
            //    child->rect.h = box->fixedSize[axis].value;
            //}

#if USE_FLOOR == 1
            child->rect.x = glm::floor(child->rect.x);
            child->rect.y = glm::floor(child->rect.y);
            child->rect.w = glm::floor(child->rect.w);
            child->rect.h = glm::floor(child->rect.h);
#endif

            f32 positionDelta = (axis == UI::Axis2_X ? child->rect.x : child->rect.y) - originalPosition;
        }
    } 
}

void UI::endBuild()
{
    // Generate layout
    for (UI::Axis2 axis = UI::Axis2_X; axis < UI::Axis2_COUNT; axis = (Axis2)(axis + 1))
    {
        calculateSizesStandalone(s_uiState.root, axis);
        calculateSizesUpwardsDependant(s_uiState.root, axis);
        calculateSizesDownwardsDependant(s_uiState.root, axis);
        enforceLayoutConstraints(s_uiState.root, axis);
        setLayoutPositions(s_uiState.root, axis);
    }

    // Set text wrap.
    for (UI::Box* box = s_uiState.root; box != nullptr; box = boxDepthFirstPreOrder(box, s_uiState.root)) 
    {
        if (box->flags & UIBoxFlags_DrawText)
            box->textWrapSize = box->fixedSize[Axis2_X].value;
    }

}

void drawBoxRecurse(UI::Box* box)
{
    if (!UI::keyComp(box->key, s_uiState.root->key))
    {
        Velox::Rectangle rect = {
            .x = box->fixedPosition[UI::Axis2_X].value,
            .y = box->fixedPosition[UI::Axis2_Y].value,
            .w = box->fixedSize[UI::Axis2_X].value,
            .h = box->fixedSize[UI::Axis2_Y].value,
        };

        if (box->flags & UI::UIBoxFlags_DrawBackground)
        {
            Velox::drawQuad(vec3(rect.x, rect.y, 0.0f), vec2(rect.w, rect.h), box->usingColor);
        }
        
        if (box->flags & UI::UIBoxFlags_DrawText)
        {
            Velox::TextDrawStyle style;
            fillTextStyleFromBox(box, &style);

            Velox::drawText(box->string.c_str(), vec3(rect.x, rect.y, 0.0f), style);
        }
    }


    for (UI::Box* child = box->first; child != nullptr; child = child->next)
    {
        drawBoxRecurse(child);
    }
}

void drawBoxRecurseDebug(UI::Box* box)
{
    UI::Box* p = box->parent;

    Velox::Rectangle rect = {
        .x = box->fixedPosition[UI::Axis2_X].value,
        .y = box->fixedPosition[UI::Axis2_Y].value,
        .w = box->fixedSize[UI::Axis2_X].value,
        .h = box->fixedSize[UI::Axis2_Y].value,
    };

    // LOG_INFO(fmt::format("Box: {}, parent: {}, {}", box->name.c_str(), p != nullptr ? p->name : "None", rect));

    if (!UI::keyComp(box->key, s_uiState.root->key))
    {
        Velox::drawRect(
            vec3(rect.x, rect.y, 0.0f),
            vec2(rect.w, rect.h),
            COLOR_BLUE);

        Velox::drawRect(
            vec3(rect.x + box->padding[UI::Axis2_X], rect.y + box->padding[UI::Axis2_Y], 0.0f),
            vec2(rect.w - box->padding[UI::Axis2_X], rect.h - box->padding[UI::Axis2_Y]),
            COLOR_GREEN);
    }

    for (UI::Box* child = box->first; child != nullptr; child = child->next)
    {
        drawBoxRecurseDebug(child);
    }
}

void UI::drawBoxes()
{
    drawBoxRecurse(s_uiState.root);

    if (s_uiState.debug)
        drawBoxRecurseDebug(s_uiState.root);
}


void Velox::initUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Window* window = GetWindow();
    void* glContext = GetGLContext();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();

    float baseFontSize = 24.0f;
    float displayScale = getDisplayScale();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(displayScale);

    // io.FontGlobalScale = displayScale;

    ImFontConfig fontConfig {};
    fontConfig.RasterizerDensity = displayScale;

    Arena tempData(2048);

    const size_t pathSize = 1024;
    char* absolutePath = tempData.alloc<char>(pathSize);

    SDL_strlcpy(absolutePath, SDL_GetBasePath(), pathSize);
    SDL_strlcat(absolutePath, "assets\\fonts\\", pathSize);
    SDL_strlcat(absolutePath, "commit_mono.ttf", pathSize);

    ImFont* commitMonoFont = 
        io.Fonts->AddFontFromFileTTF(absolutePath, baseFontSize * displayScale, &fontConfig);

    // Begin first frame state.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    SubscribeInfo subInfo {
        .name = "UI (+ imgui)",
        // Leaving this blank for now, will see how it behaves.
        //.eventRangeStart = u32(0x200), // SDL input event range.
        //.eventRangeEnd   = u32(0x300),
        .callback = uiEventCallback,
        .priority = 3,
    };

    getEventPublisher()->subscribe(subInfo);
}

bool Velox::uiEventCallback(SDL_Event& event)
{ 
    ImGui_ImplSDL3_ProcessEvent(&event);
    return false;
}

ImDrawData* Velox::getUIDrawData()
{
    return ImGui::GetDrawData();
}

void Velox::deInitUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
