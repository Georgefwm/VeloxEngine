#include "PlaneGame.h"
#include <PCH.h>

#include "Core.h"
#include "Entity.h"
#include "Event.h"
#include "Menu.h"
#include "PostGameMenu.h"
#include "Plane.h"
#include "Rendering/Renderer.h"
#include "Timing.h"

static GameState s_gameState {};

GameState* getGameState() { return &s_gameState; }

void changeGameStage(u8 newStage)
{
    if (s_gameState.gameStage == newStage)
        return;

    if (newStage == GameStage::Simulation)
    {
        // Enter simulation logic.
        Velox::getEntityManager()->destroyAllEntities();
        setupPlane();
    }

    if (newStage == GameStage::MainMenu)
    {
        // Enter main menu logic.
        Velox::getEntityManager()->destroyAllEntities();
    }

    if (newStage == GameStage::PostRound)
    {
        // Enter post round menu logic.
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
    if (s_gameState.gameStage == GameStage::MainMenu)
    {
        drawMenu();
    }

    if (s_gameState.gameStage == GameStage::PostRound)
    {
        drawPostGameMenu();
    }
    
    Velox::getEntityManager()->drawEntities();
}

void runPlaneGame()
{
    Velox::init();

    s_gameState = {};

    while (!Velox::quitRequested())
    {
        Velox::getEventPublisher()->processEvents();

        // In this example updateGame handles drawing as well, but it doesn't have to.
        Velox::updateGame(doPlaneGameUpdates);

        drawPlaneGame();

        Velox::submitFrameData();

        // Calculate after frame present.
        Velox::calculateDeltaTime();
    }

    // Cleanup.
    Velox::deInit();
}

