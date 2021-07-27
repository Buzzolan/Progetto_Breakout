/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "2.game.h"
#include "game\resource_manager.h"
#include "game\sprite_renderer.h"
#include "game\game_object.h"
#include "game\5.1.ball_object_collisions.h"
#include "game\particle_generator.h"
#include "game\post_processor.h"
#include <game\irrklang/irrKlang.h>
#include "game\text_renderer.h"
#include <sstream>
#include <iostream>
using namespace std;

//carico audio 
using namespace irrklang;
ISoundEngine* SoundEngine_open = createIrrKlangDevice();
ISoundEngine* SoundEngine_active = createIrrKlangDevice();
ISoundEngine* SoundEngine_2 = createIrrKlangDevice();
bool Ok_audio_menu = true;
bool Ok_audio_active = true;
bool Ok_audio_win = true;

//carico testo
TextRenderer* Text;

// Game-related State data
ParticleGenerator* Particles; // generatore particelle
SpriteRenderer* Renderer;// creo oggetto Renderer dalla classe sprite(il mio background)
GameObject* Player; // Player è il mio pad
BallObject* Ball; // Oggetto palla
PostProcessor* Effects;



float ShakeTime = 0.0f;// resetto varibile shake





Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_MENU), Keys(), Width(width), Height(height)
{ 

}

Game::~Game()
{   
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
    SoundEngine_open->drop();
    
}

void Game::Init()//inizializzo il gico
{
    this->Lives = 3;
    //carcio testo
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("Breakout_game_project/Lovelo/TTF/Lovelo-Black.ttf", 24); 
    
   
    // load shaders per sprite
    ResourceManager::LoadShader("Breakout_game_project/Texture_Shader/sprite.vs", "Breakout_game_project/Texture_Shader/sprite.fs", nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    // invece di Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite")); uso:
    Shader ShaderSprite;
    ShaderSprite = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(ShaderSprite);

    // load textures
    //ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/32hat.png", true, "face");
    // load textures  pergamena.png straw_hat_menu.jpg
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/onepiece_background.jpg", false, "background");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/trafalga_law_flag.png", true, "face");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/block.png", false, "block");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/straw_hat_menu.jpg", false, "menu_background");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/pergamena.png", true, "woods");

    //texture powerup
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/powerup_passthrough.png", true, "powerup_passthrough");

    // load levels
    GameLevel one; one.Load("Breakout_game_project/levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("Breakout_game_project/levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("Breakout_game_project/levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("Breakout_game_project/levels/four.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0;

    //carico particelle
    ResourceManager::LoadShader("Breakout_game_project/Texture_Shader/particle.vs", "Breakout_game_project/Texture_Shader/particle.fs", nullptr, "particle");
    //configuro shader particelle
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    //carico texture paticelle
    ResourceManager::LoadTexture("Breakout_game_project/Texture_Shader/smoke_07.png", true, "particle");
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), 
        ResourceManager::GetTexture("particle"),
        500// metti 1000 per un effetto grafico migliore
    );

    //configuro shader per post_prcessing
    ResourceManager::LoadShader("Breakout_game_project/Texture_Shader/post_processing.vs", "Breakout_game_project/Texture_Shader/post_processing.fs", nullptr, "postprocessing");
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);
    // configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

    // configuro la mia palla da gioco
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));

   
}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width);

    // check for collisions ad ogni frame
    this->DoCollisions();
    if (Ball->Position.y >= this->Height) // quando la palla raggiunge il fondo
    {   
        --this->Lives;
        // controllo se il giocatore ha ancora vite: Game over
        if (this->Lives == 0)
        {
            this->ResetLevel();
            this->State = GAME_MENU;
        }
        
        this->ResetPlayer();
    }
    // update particles
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
    //la particella è collegata alla palla
    //aggiorno poweup
    this->UpdatePowerUps(dt);
    //decremto la varibili shake time fino a zero, dopo disattivo l'effeto shake
    if (ShakeTime > 0.0f)
    {
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f)
            Effects->Shake = false;
    }

    //controllo se vinco
    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = true;
        this->State = GAME_WIN;
    }
    
}
void Game::ResetLevel()
{
    this->Lives = 3;//assegno le vite quando resetto livello, resetto livello se non ho più vite
    if (this->Level == 0)
        this->Levels[0].Load("Breakout_game_project/levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("Breakout_game_project/levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("Breakout_game_project/levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("Breakout_game_project/levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

    //disabilito poweup attivi
    Effects->Chaos = Effects->Confuse = false;
    Ball->PassThrough = Ball->Sticky = false;
    Player->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f);
}

void Game::ProcessInput(float dt)
{   
    //muovo il player a dx o sx, controllo anche se sono arrivato a fine schermo 
    if (this->State == GAME_ACTIVE)
    {   
        
        
       
        //gestione del suono:
        if (Ok_audio_active) {
            SoundEngine_open->stopAllSounds();
            SoundEngine_active->play2D("Breakout_game_project/audio/onepiece-game_2.mp3", true);
            Ok_audio_active = false;
            Ok_audio_menu = true;

        }

        float velocity = PLAYER_VELOCITY * dt;
        // move playerboard
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0.0f)
            {
                Player->Position.x -= velocity;
                //aggiorno anche posizione ball
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }

        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }
    // se sono nel menu scelgo che livello giocare
    if (this->State == GAME_MENU)
    {
        if (Ok_audio_menu) {
            //carico audio:
            SoundEngine_active->stopAllSounds();//stoppo audio del gioco
            SoundEngine_open->play2D("Breakout_game_project/audio/Opening.mp3", true);
            SoundEngine_open->setSoundVolume(0.6f);
            Ok_audio_menu = false;
            Ok_audio_active = true;

        }
        //cerco di elaboare tasti che sono stati elaborati già una volta, poi dobbia resettare il tasto keyprocess, cosi da riprocessarlo di nuovo
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;// inizio a giocare se premo invio
            this->KeysProcessed[GLFW_KEY_ENTER] = true;// il tasto è stato premuto
        }
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
        {
            this->Level = (this->Level + 1) % 4;// cosi sono nel range tra 0 e 3
            this->KeysProcessed[GLFW_KEY_W] = true;
        }
        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
        {
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = 3;
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
        
    }
    if (this->State == GAME_WIN)// quando vinco posso ritornare al menu
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;
            this->State = GAME_MENU;
        }
    }
   
}

void Game::Render()
{
    // Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200.0f, 200.0f) posizione nello schermo , glm::vec2(300.0f, 400.0f) dimensione immagine , 45.0f rotazione, glm::vec3(0.0f, 1.0f, 0.0f) colore);
    
    if (this->State == GAME_ACTIVE)
    {   
        Effects->BeginRender();

        

        
        // draw background
        Texture2D myTextureRender;
        myTextureRender = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(myTextureRender, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        // draw player
        Player->Draw(*Renderer);

        // draw particles	
        Particles->Draw();

        // draw la palla
        Ball->Draw(*Renderer);

        //renderizzo powerups
        for (PowerUp& powerUp : this->PowerUps)
            if (!powerUp.Destroyed)
                powerUp.Draw(*Renderer);

        //disegno scritte:
        std::stringstream ss; ss << this->Lives;
        Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);

        Effects->EndRender();
        Effects->Render(glfwGetTime());

        //Ciò che è interessante notare qui sono le funzioni BeginRender ed EndRender
        //Poiché dobbiamo eseguire il rendering dell'intera scena di gioco nel framebuffer, 
        //possiamo chiamare convenzionalmente BeginRender() e EndRender() rispettivamente prima e dopo il codice di rendering della scena. 
        //La classe gestirà quindi le operazioni del framebuffer dietro le quinte.
    }
    if (this->State == GAME_MENU)
    {   
        
        
        Texture2D myTextureRenderMenu;
        myTextureRenderMenu = ResourceManager::GetTexture("menu_background");
        Renderer->DrawSprite(myTextureRenderMenu, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        Texture2D woods;
        woods = ResourceManager::GetTexture("woods");
        Renderer->DrawSprite(woods, glm::vec2(Width / 2-170.0f, Height / 2-25.0f), glm::vec2(340.0f, 100.0f), 0.0f);

        Text->RenderText("Press ENTER to start", Width/2-150.0f, Height / 2, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        Text->RenderText("Press W or S to select level", Width / 2 - 150.0f, Height / 2 + 20.0f, 0.75f, glm::vec3(0.0f, 0.0f, 0.0f));
        std::stringstream ss1; ss1 << this->Level;
        Text->RenderText("Livello selezionato:" + ss1.str(), Width / 2 - 150.0f, Height / 2 + 40.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
        std::stringstream ss2; ss2 << this->Lives;
        Text->RenderText("Lives:" + ss2.str(), 5.0f, 5.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
    
    }

    if (this->State == GAME_WIN)
    {
        Text->RenderText("Hai vinto, il Paese di Wano è libero", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0));
        Text->RenderText( "Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0));
    }


}
//-------------------------------------------------GESTIONE DEI POWEUPS---------------------------------------------

bool ShouldSpawn(unsigned int chance)
{
    unsigned int random = rand() % chance;
    return random == 0;
}
void Game::SpawnPowerUps(GameObject& block) //ogni volta che distruggo un blocco voglio dare una chance di generare un powerup
{
    
    if (ShouldSpawn(75)) // possibilità uno su 75
        //la durata la metto a zero cosi nel tempo piu poweu prendo e piu veloce va rendendo difficle il gioco 
        this->PowerUps.push_back(
            PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")
            )); //imposto prorpieta del powerup, tipo, colore, durata, posizione e texture
    //se durata =0.0 allora è infinito e passo posizione del blocco distrutto 
    if (ShouldSpawn(75))
        this->PowerUps.push_back(
            PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")
            ));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(
            PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")
            ));
    if (ShouldSpawn(5))
        this->PowerUps.push_back(
            PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 10.0f, block.Position, ResourceManager::GetTexture("powerup_increase")
            ));
    if (ShouldSpawn(15)) // negative powerups should spawn more often
        this->PowerUps.push_back(
            PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")
            ));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(
            PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")
            )); 
    
}


void ActivatePowerUp(PowerUp& powerUp)// attivazione powerup 
{
    if (powerUp.Type == "speed")
    {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!Effects->Chaos)
            Effects->Confuse = true; // only activate if chaos wasn't already active
    }
    else if (powerUp.Type == "chaos")
    {
        if (!Effects->Confuse)
            Effects->Chaos = true;
    }
}

bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)//verifico se ho altri powerup attavi
{
    for (const PowerUp& powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}

void Game::UpdatePowerUps(float dt) // muovo powerup e controllo se sono scaduti
{
    for (PowerUp& powerUp : this->PowerUps)//per ogni powerup
    {
        powerUp.Position += powerUp.Velocity * dt;//muovo in base al tempo
        if (powerUp.Activated)// controllo se attivo 
        {
            powerUp.Duration -= dt;// decremento la durata

            if (powerUp.Duration <= 0.0f)
            {
                // remove powerup from list (will later be removed), intanto lo disattivo
                powerUp.Activated = false;
                // deactivate effects, gestico le disattivazioni dei vari effetti
                if (powerUp.Type == "sticky")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                    }
                }
                else if (powerUp.Type == "pad-size-increase")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "pad-size-increase"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Player->Size = PLAYER_SIZE;
                    }
                }
            }
        }
    }
    //Alla fine di UpdatePowerUps, eseguiamo un ciclo attraverso il vettore PowerUps 
    //e cancelliamo ogni potenziamento se vengono distrutti e disattivati. 
    //Usiamo la funzione remove_if dall'intestazione dell'algoritmo per cancellare questi elementi dato un predicato. 
    //La funzione remove_if sposta tutti gli elementi per i quali il predicato lambda è vero alla fine dell'oggetto contenitore e restituisce un iteratore all'inizio di questo intervallo di elementi rimossi. La funzione di cancellazione 
    //del contenitore utilizza quindi questo iteratore e l'iteratore finale del vettore per rimuovere tutti gli elementi tra questi due iteratori. 
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}



//----------------------------------------------------GESTIONE DELLE COLLSIONI--------------------------------------------------------




Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);//calcolando il prodotto scalare tra il target(differnce) e una 
        //delle 4 direzioni la direzione più probabile è quando i due vettori sono paralleli, cos(0)=1 e minima quando angolo tra i due è 90, cos(90)=0
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

float clamp(float value, float min, float max) { // calcolo la clamped.x e la clamped.y
    return std::max(min, std::min(max, value));
}
Collision CheckCollision(BallObject& one, GameObject& two) // AABB - Circle collision, overloda vecchia collsion
{
    
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x,
        two.Position.y + aabb_half_extents.y
    );
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;

    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);//ritorna una tupla di informazioni
    //true=avviene collisione, che direzione?, V=punto più vicino a collisione - centro sfera
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
     
}
bool CheckCollision(GameObject& one, GameObject& two) // AABB - AABB collision tra due rettangoli
{
    //guardo se due rettengoli si sovrappongono, ricordati che 0,0 è in alto a sx
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

void Game::DoCollisions()
{
    for (GameObject& box : this->Levels[this->Level].Bricks)//per ogni brick del mio livello constrollo se ho la collisone
    {
        if (!box.Destroyed)//controllo se è già stato distrutto
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision))// se si toccano allo ho distrutto mattoncino 
            {
                //controllo se non è solido
                if (!box.IsSolid)
                {
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                    //quando distruggo mattoncino
                    SoundEngine_active->play2D("Breakout_game_project/audio/explosion.mp3", false);
                }
                else
                {
                    //se il blocco è solito attivo lo shake effect
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                    //quando becco blocco solido
                    SoundEngine_active->play2D("Breakout_game_project/audio/solid.wav", false);
                }
                //Estrpolo direzione e vettore V=P-C
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);//vettore R
                if (!(Ball->PassThrough && !box.IsSolid))// se il powerup è attivo non eseguo il rimbalzo della pallina 
                {
                    if (dir == LEFT || dir == RIGHT)//collsione orizzontale
                    {
                        Ball->Velocity.x = -Ball->Velocity.x;// inverto direzione orizzontale
                        // ri-alloco la palla 
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // rialloco spostamento palla a dx
                        else
                            Ball->Position.x -= penetration; // muovo palla a sx
                    }
                    else // ho una collsione verticale 
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; //inverto direzione verticale
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball back up
                        else
                            Ball->Position.y += penetration; // move ball back down
                    }
                }
            }
            Collision result = CheckCollision(*Ball, *Player);
            if (!Ball->Stuck && std::get<0>(result))
            {
                // check where it hit the board, and change velocity based on where it hit the board
                float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
                float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
                float percentage = distance / (Player->Size.x / 2.0f);
                // then move accordingly
                float strength = 2.0f;
                glm::vec2 oldVelocity = Ball->Velocity;
                Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;

                //Ball->Velocity.y = -Ball->Velocity.y;// applico il mio trick
                Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);// cosi ho y sempre negativa 
                Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);

                //effetto appiccicoso
                Ball->Stuck = Ball->Sticky;

                SoundEngine_2->setSoundVolume(0.02f);
                SoundEngine_2->play2D("Breakout_game_project/audio/bleep.wav", false);
                
            }
            for (PowerUp& powerUp : this->PowerUps)
            {
                if (!powerUp.Destroyed)//se il powerup è attivo
                {
                    if (powerUp.Position.y >= this->Height)//se la posizione del mio powerup arriva alla fine lo distruggo
                        powerUp.Destroyed = true;
                    if (CheckCollision(*Player, powerUp))//controllo collisone tra il pad e il pomerup
                    {	// collided with player, now activate powerup
                        ActivatePowerUp(powerUp);//attivo il powerup
                        powerUp.Destroyed = true;//distruggo il powerup
                        powerUp.Activated = true;//etichetto il poweup come attivo
                        cout << "powerup attivo= " << powerUp.Type ;
                        cout << "powup durata= " << powerUp.Duration;
                        //attivo suono poweup
                        SoundEngine_active->play2D("Breakout_game_project/audio/gearsecond.mp3", false);
                    }
                }
            }
        }
    }
}





