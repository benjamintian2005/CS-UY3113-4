#include "Menu.h"
#include "Utility.h"

Menu::~Menu()
{
    // Nothing to clean up in this simple menu
}

void Menu::initialise()
{
    // Load font texture
    m_font_texture_id = Utility::load_texture("assets/font1.png");
    
    // No map, player, or enemies in the menu scene
    m_game_state.map = nullptr;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
    
    // Set the next scene ID to 1 (assuming LevelA is scene 1)
    m_game_state.next_scene_id = 1;
}

void Menu::update(float delta_time)
{
}

void Menu::render(ShaderProgram *program)
{
    // Render game title
    Utility::draw_text(program, m_font_texture_id, "PLATFORM ADVENTURE", 0.5f, 0.0f, glm::vec3(-4.0f, 2.0f, 0.0f));
    
    // Render "Press Enter to start" text
    Utility::draw_text(program, m_font_texture_id, "Press Enter to start", 0.4f, 0.0f, glm::vec3(-3.5f, 0.0f, 0.0f));
}

void Menu::shoot_projectile()
{
   
}
