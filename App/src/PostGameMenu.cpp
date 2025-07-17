#include "PostGameMenu.h"
#include <PCH.h>

#include "Input.h"
#include "PlaneGame.h"
#include "Rendering/Renderer.h"
#include "Text.h"
#include "Util.h"

static constexpr Velox::TextDrawStyle TITLE_TEXT_STYLE = {
    .textSize = 120,
    .color = COLOR_WHITE,
};

static constexpr Velox::TextDrawStyle TEXT_STYLE = {
    .textSize = 70,
    .color = COLOR_GRAY_MEDIUM,
};

static constexpr Velox::TextDrawStyle HOVERED_TEXT_STYLE = {
    .textSize = 90,
    .color = COLOR_YELLOW,
};

static constexpr float ITEM_SPACING = 100.0f;

static std::vector<std::string> s_menuItems = { "Play Again?", "Main Menu" };

void drawPostGameMenu()
{
    GameState* gs = getGameState();
    vec3 menuPosition = vec3(200.0f, 200.0f, 0.0f);

    Velox::pushTextStyle(TITLE_TEXT_STYLE);
    Velox::drawText("YOU DIED", menuPosition);
    Velox::popTextStyle();

    menuPosition.y += ITEM_SPACING + 50.0f;

    Velox::pushTextStyle(TEXT_STYLE);
    
    for (i32 i = 0; i < s_menuItems.size(); i++)
    {
        Velox::Rectangle textBounds;
        Velox::getStringBounds(s_menuItems[i].c_str(), menuPosition, &textBounds);

        if (Velox::isMouseInArea(textBounds))
        {
            Velox::pushTextStyle(HOVERED_TEXT_STYLE);
            Velox::drawText(s_menuItems[i].c_str(), menuPosition);
            Velox::popTextStyle();

            if (Velox::isMouseButtonPressed(1))
            {
                switch (i)
                {
                case 0:
                    changeGameStage(GameStage::Simulation);
                    break;
                case 1:
                    changeGameStage(GameStage::MainMenu);
                    break;
                }

                return;
            }
        }
        else
        {
            Velox::drawText(s_menuItems[i].c_str(), menuPosition);
        }

        menuPosition.y += ITEM_SPACING;
    }

    Velox::popTextStyle();
}
