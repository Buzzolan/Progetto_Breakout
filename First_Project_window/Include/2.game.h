/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game/game_level.h"
#include "game/power_up.h"

// Rappresenta lo stato corrente del gioco, ci permette di tenere traccia dello stato in cui mi trovo,
// in modo da aggiustare il rendering e il processing basto sullo stato del gioco, renderizziamo cose diverse quando sono nel menu
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 14.5f;

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
    // game state
    GameState               State;	
    bool                    Keys[1024];
    unsigned int            Width, Height;
    std::vector<GameLevel>  Levels; // aggiungo i livelli al mio gioco 
    unsigned int            Level;
    std::vector<PowerUp>  PowerUps;// vettore per tenere traccia dei poweup attivi
    unsigned int Lives; //vite del giocatore
    bool KeysProcessed[1024];// gestione tasto cambio livelli
    
    void SpawnPowerUps(GameObject& block);//genero poweup da un mattoncino
    void UpdatePowerUps(float dt);// gestisco poweup attivi
    // constructor/destructor
    Game(unsigned int width, unsigned int height);// inizializzo il gioco con la risoluzione che voglio 
    ~Game();
    // initialize game state (carico tutti gli shader shaders/textures/levels)
    void Init();
    // game loop
    void ProcessInput(float dt);// Possiamo elaborare l'input come memorizzato all'interno dell'array Keys
    void Update(float dt);// aggiorniamo tutti gli eventi del gioco(movimento giocatore e palla)
    void Render();// renderizziamo il gioco
    // NB: abbaimo diviso la logica di movimento (update) da quella di rendering(rendr)
    void DoCollisions();
    void ResetLevel();
    void ResetPlayer();
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

#endif