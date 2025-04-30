#include "GameOver.h"
#include "Utility.h"

GameOver::GameOver(bool is_win) : m_is_win(is_win) {}

void GameOver::initialise()
{
    // No need for map, players, or enemies on this screen
    m_game_state.map = nullptr;
    m_game_state.player = nullptr;
    m_game_state.enemies = nullptr;
}

void GameOver::update(float delta_time)
{
    // No updates needed for this screen
}

void GameOver::render(ShaderProgram *program)
{
    // Set text color based on win/lose
    
    
    // Display appropriate message
    if (m_is_win) {
        Utility::draw_text(program, Utility::load_texture("assets/font1.png"), "YOU WIN", 0.5f, 0.0f, glm::vec3(-2.0f, 2.0f, 0.0f));
    } else {
        Utility::draw_text(program, Utility::load_texture("assets/font1.png"), "YOU LOSE", 0.5f, 0.0f, glm::vec3(-2.0f, 2.0f, 0.0f));
    }
}

void GameOver::shoot_projectile()
{
   
}
