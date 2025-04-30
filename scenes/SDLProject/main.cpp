#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "GameOver.h"


#include "Menu.h"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.08f,    // Minimal red
            BG_GREEN   = 0.04f,    // Minimal green
            BG_BLUE    = 0.01f,    // Barely any blue
            BG_OPACITY = 1.0f;     // Keep fully opaque

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_lit.glsl",
           F_SHADER_PATH[] = "shaders/fragment_lit.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };
enum GameMode { MENU, GAME };

// ————— GLOBAL VARIABLES ————— //
GameMode g_current_mode = MENU;
Scene *g_current_scene;
LevelA *g_level_a;
LevelB *g_level_b;
LevelC *g_level_c;
GameOver *g_game_over_lose;
GameOver *g_game_over_win;


Menu *g_menu_scene;

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise();
void process_input();
void update();
void render();
void shutdown();

void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, Scenes!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— SCENES ————— //
    g_menu_scene = new Menu();
    g_level_a = new LevelA();
    g_level_b = new LevelB();
    g_level_c = new LevelC();
    
    g_game_over_lose = new GameOver(false);
    g_game_over_win = new GameOver(true);
    
    
    
    // Start with menu scene
    switch_to_scene(g_menu_scene);
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // Only reset player movement if we're in a scene with a player
    if (g_current_scene->get_state().player != nullptr) {
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    }
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // ————— KEYSTROKES ————— //
        switch (event.type) {
            // ————— END GAME ————— //
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                    case SDLK_RETURN:
                        // Switch to game mode if in menu
                        if (g_current_mode == MENU) {
                            g_current_mode = GAME;
                            switch_to_scene(g_level_a);
                        }
                        break;
                    case SDLK_SPACE:
                        // Attack with spacebar
                        if (g_current_scene != g_menu_scene) {
                            g_current_scene->shoot_projectile();
                        }
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    // ————— KEY HOLD ————— //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    // Only process movement if we're in the game scene
    if (g_current_scene != g_menu_scene && g_current_scene->get_state().player != nullptr) {
        // Arrow key movement
        if (key_state[SDL_SCANCODE_LEFT])        g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_RIGHT])  g_current_scene->get_state().player->move_right();
        
        // WASD movement
        if (key_state[SDL_SCANCODE_A])           g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_D])      g_current_scene->get_state().player->move_right();
        
        if (key_state[SDL_SCANCODE_UP])          g_current_scene->get_state().player->move_up();
        else if (key_state[SDL_SCANCODE_DOWN])   g_current_scene->get_state().player->move_down();
        
        if (key_state[SDL_SCANCODE_W])           g_current_scene->get_state().player->move_up();
        else if (key_state[SDL_SCANCODE_S])      g_current_scene->get_state().player->move_down();
         
        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
}

void update()
{
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // ————— PLAYER CAMERA ————— //
    g_view_matrix = glm::mat4(1.0f);
    
    // Only center camera on player if we're in the game scene and player exists
    if (g_current_scene != g_menu_scene && g_current_scene->get_state().player != nullptr)
    {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(
            -g_current_scene->get_state().player->get_position().x,
            -g_current_scene->get_state().player->get_position().y,
            0.0f
        ));
    }
    if (g_current_scene->get_state().player != nullptr && g_current_mode == GAME) {
            if (g_current_scene->get_state().player->get_health() <= 0) {
                        
                        switch_to_scene(g_game_over_lose);
            }
            else
            {
                if (g_current_scene == g_level_a && ((LevelA*)g_current_scene)->is_completed()) {
                    
                    switch_to_scene(g_level_b);  // Move to Level B
                }
                else if (g_current_scene == g_level_b && ((LevelB*)g_current_scene)->is_completed()) {
                    switch_to_scene(g_level_c);
                }
                else if (g_current_scene == g_level_c && ((LevelC*)g_current_scene)->is_completed()) {
                    // Game finished - back to menu or show victory screen
                   
                    switch_to_scene(g_game_over_win);
                }
            }
        }
    if(g_current_scene!= g_menu_scene && g_current_scene!= g_game_over_win && g_current_scene!= g_game_over_lose)
    {
        
        g_shader_program.set_light_position_matrix(g_current_scene->get_state().player->get_position());
    }
    else
    {
        glm::vec3 default_light_pos = glm::vec3(0.0f, 0.0f, 0.0f);  // Center of the screen
                g_shader_program.set_light_position_matrix(default_light_pos);
    }
}

void render()
{
   
    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_shader_program.get_program_id());

    
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    
    
    
    
    g_current_scene->render(&g_shader_program);

    // Only render player health bar if we're in the game scene and player exists
    if (g_current_scene == g_level_a && g_current_scene->get_state().player != nullptr)
    {
       // g_current_scene->get_state().player->render_health_bar(&g_shader_program, );
    }
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    // ————— DELETING SCENE DATA ————— //
    delete g_level_a;
    delete g_level_b;
    delete g_menu_scene;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
