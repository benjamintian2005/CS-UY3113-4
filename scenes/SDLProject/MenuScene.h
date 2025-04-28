#pragma once
#include "Scene.h"
#include "Utility.h"
#include "LevelA.h"

class MenuScene : public Scene {
public:
    // ————— DESTRUCTOR ————— //
    ~MenuScene() {
        // Clean up any resources if needed
    }
    
    // ————— METHODS ————— //
    void initialise() override {
        // Load font texture for menu text
        m_font_texture_id = Utility::load_texture("assets/font1.png");
        
        // Initialize game state
        m_game_state.next_scene_id = -1;  // -1 means stay in menu
    }
    
    void update(float delta_time) override {
        // Check for space key to start game
        const Uint8 *key_state = SDL_GetKeyboardState(NULL);
        if (key_state[SDL_SCANCODE_SPACE])
        {
            m_start_game = true;
            m_game_state.next_scene_id = 1;  // 1 will be LevelA
        }
    }
    
    void render(ShaderProgram *program) override {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw title
        Utility::draw_text(program, m_font_texture_id, "TOP DOWN SHOOTER", 0.5f, 0.0f, glm::vec3(-2.0f, 1.0f, 0.0f));
        
        // Draw instructions
        Utility::draw_text(program, m_font_texture_id, "PRESS SPACE TO START", 0.3f, 0.0f, glm::vec3(-1.5f, -0.5f, 0.0f));
        Utility::draw_text(program, m_font_texture_id, "ARROWS/WASD TO MOVE", 0.3f, 0.0f, glm::vec3(-1.5f, -1.0f, 0.0f));
        Utility::draw_text(program, m_font_texture_id, "SPACE TO SHOOT", 0.3f, 0.0f, glm::vec3(-1.5f, -1.5f, 0.0f));
    }
    
private:
    GLuint m_font_texture_id;
    bool m_start_game = false;
}; 