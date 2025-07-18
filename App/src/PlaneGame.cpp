#include "PlaneGame.h"
#include <PCH.h>
#include <SDL3/SDL_scancode.h>

#include "Core.h"
#include "Entity.h"
#include "Event.h"
#include "Input.h"
#include "Menu.h"
#include "Obstacles.h"
#include "PostGameMenu.h"
#include "Plane.h"
#include "Rendering/Renderer.h"
#include "ScrollingBackground.h"
#include "Text.h"
#include "Timing.h"
#include <imgui.h>

static GameState s_gameState {};

GameState* getGameState() { return &s_gameState; }

// This is very unreal engine-y, not sure if good thing.
void changeGameStage(u8 newStage)
{
    if (s_gameState.gameStage == newStage)
        return;

    if (newStage == GameStage::Simulation)
    {
        Velox::getEntityManager()->destroyAllEntities();

        s_gameState.score = 0;

        setupScrollingBackground();
        setupPlane();
        setupSpawner();
    }

    if (newStage == GameStage::MainMenu)
        Velox::getEntityManager()->destroyAllEntities();

    if (newStage == GameStage::PostRound)
    {
        if (s_gameState.highScore < s_gameState.score)
        {
            s_gameState.highScore = s_gameState.score;

            // Something special for new highscore!
        }
    }

    s_gameState.gameStage = newStage;
}

void doPlaneGameUpdates(const double& deltaTime)
{
    if (s_gameState.gameStage != GameStage::Simulation)
        return;

    Velox::getEntityManager()->treeView.updateEntities(deltaTime);
}

void drawPlaneGame()
{
    Velox::getEntityManager()->drawEntities();

    if (s_gameState.gameStage == GameStage::Simulation)
    {
        Velox::TextDrawStyle style {};
        style.textSize = 100.0f;

        std::string scoreText = fmt::format("Score: {}", s_gameState.score);

        Velox::pushTextStyle(style);
        Velox::drawText(scoreText.c_str(), vec3(50.0f, 50.0f, 0.0f));
        Velox::popTextStyle();
    }

    if (s_gameState.gameStage == GameStage::MainMenu)
        drawMenu();

    if (s_gameState.gameStage == GameStage::PostRound)
        drawPostGameMenu();
}

void runPlaneGame()
{
    Velox::init();

    s_gameState = {};

    while (!Velox::quitRequested())
    {
        Velox::getEventPublisher()->processEvents();

        if (Velox::isKeyPressed(SDL_SCANCODE_ESCAPE))
            s_gameState.paused = !s_gameState.paused;

        if (!s_gameState.paused)
            Velox::updateGame(doPlaneGameUpdates);

        drawPlaneGame();

        ImGui::ShowDemoWindow();

        Velox::submitFrameData();

        // Calculate after frame present.
        Velox::calculateDeltaTime();
    }

    // Cleanup.
    Velox::deInit();
}

