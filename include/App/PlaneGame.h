#pragma once

enum GameStage : u8 {
    MainMenu   = 0,
    Simulation = 1,
    PostRound  = 2,
};

struct GameState {
    u8 gameStage = 0;

    // Menu
    i32 hoveredItem = -1;

    // Simulation
    float scrollSpeed = 10;
};

GameState* getGameState();

void changeGameStage(u8 newStage);
void runPlaneGame();

