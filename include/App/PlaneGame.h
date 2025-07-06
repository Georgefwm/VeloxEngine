#pragma once

enum GameStage : u8 {
    MainMenu   = 0,
    Simulation = 1,
    PostRound  = 2,
};

struct GameState {
    u8 gameStage = 0;
    bool paused = false;;

    // Menu
    i32 hoveredItem = -1;

    // Simulation
    float scrollSpeed = 700.0f;
};

enum EntityType : u32 {
    None    = 0,
    Plane   = 1,
    Spawner = 2,
    Spike   = 3,
};

GameState* getGameState();

void changeGameStage(u8 newStage);
void runPlaneGame();

