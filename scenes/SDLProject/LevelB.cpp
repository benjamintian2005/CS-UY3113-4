#include "LevelB.h"
#include "Utility.h"
#include "vector"
#define LEVEL_B_WIDTH 14
#define LEVEL_B_HEIGHT 14  // Make the level square for top-down view

#define ENEMY_B_COUNT 1 // Increase number of enemies

constexpr char SPRITESHEET_B_FILEPATH[] = "assets/armyman.png",
           PLATFORM_B_FILEPATH[]    = "assets/platformPack_tile027.png",
           ENEMY_B_FILEPATH[]       = "assets/theface.png",
PROJECTILE_B_FILEPATH[] = "assets/projectile.png";

unsigned int LEVEL_B_DATA[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};



LevelB::~LevelB()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    
    // Clean up projectiles
    for (int i = 0; i < m_game_state.projectiles.size(); i++) {
        delete m_game_state.projectiles[i];
    }
    m_game_state.projectiles.clear();
    
    Mix_FreeChunk(m_game_state.pew_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/world_tileset.png");
    m_game_state.map = new Map(LEVEL_B_WIDTH, LEVEL_B_HEIGHT, LEVEL_B_DATA, map_texture_id, 1.0f, 16, 16);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_B_FILEPATH);
    
    // Load font texture for health display
    m_font_texture_id = Utility::load_texture("assets/font1.png");

    // Load projectile texture and verify it loaded correctly
    GLuint projectile_texture_id = Utility::load_texture(PROJECTILE_B_FILEPATH);
    if (projectile_texture_id == 0) {
        std::cout << "ERROR: Failed to load projectile texture from " << PROJECTILE_B_FILEPATH << std::endl;
    } else {
        std::cout << "Successfully loaded projectile texture with ID: " << projectile_texture_id << std::endl;
    }

    int player_walking_animation[4][7] =
    {
          // Left (row 3, frames 2-8)
        { 22, 23, 24, 25, 26, 27, 28},
        { 33, 34, 35, 36, 37, 38, 39 },// Right (row 2, frames 2-8)
        { 11, 12, 13, 14, 15, 16, 17 },   // Up (row 1, frames 2-8)
        { 0, 1, 2, 3, 4, 5, 6 }          // Down (row 0, frames 0-6, unchanged)
    };

    // Remove gravity for top-down movement
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.0f,                      // speed (adjusted for top-down)
        acceleration,              // acceleration (no gravity)
        0.0f,                      // animation time
        7,                         // animation frame amount
        0,                         // current animation index
        11,                         // animation column amount
        4,                         // animation row amount
        1.0f,                      // width (slightly smaller for top-down view)
        1.0f,                      // height (slightly smaller for top-down view)
        PLAYER
    );
    m_game_state.player->set_scene(this);
    
    // Set the walking animation after construction
    m_game_state.player->set_walking(player_walking_animation);
    m_game_state.player->face_right(); // Start facing right
    
    m_game_state.player->set_position(glm::vec3(5.0f, -2.0f, 0.0f));  // Start more centered

    // Remove jumping section since it's not needed for top-down
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_B_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_B_COUNT];

    for (int i = 0; i < ENEMY_B_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, BOSS1, IDLE);
        m_game_state.enemies[i].set_health(200);  // Set enemy health
        m_game_state.enemies[i].set_scene(this);
    }

    // Position the enemies in different parts of the map
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, -5.0f, 0.0f));

   

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/scaryloop.mp3");
    
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(100.0f);
    
    m_game_state.pew_sfx = Mix_LoadWAV("assets/bounce.wav");
    m_game_state.win_sfx =Mix_LoadWAV("assets/yay.mp3");

}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_B_COUNT, m_game_state.map);
    
    for (int i = 0; i < ENEMY_B_COUNT; i++)
    {
        Entity* enemy = &m_game_state.enemies[i];
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
       
        
    }
    
    // Update projectiles
    for (int i = 0; i < m_game_state.projectiles.size(); i++)
    {
        Entity* projectile = m_game_state.projectiles[i];
        
        // Call the projectile's update method instead of manually updating position
        projectile->update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
        
        // Debug output for projectile position
        if (i == 0) { // Only print for the first projectile to avoid spam
            //std::cout << "Projectile position: " << projectile->get_position().x << ", " << projectile->get_position().y << std::endl;
        }
        
        // Check for collisions with enemies
        for (int j = 0; j < ENEMY_B_COUNT; j++)
        {
            if (projectile->check_collision(&m_game_state.enemies[j]))
            {
                // Enemy hit by projectile
                m_game_state.enemies[j].take_damage(1);
                
                // Remove the projectile
                delete projectile;
                m_game_state.projectiles.erase(m_game_state.projectiles.begin() + i);
                i--; // Adjust index since we removed an element
                break;
            }
        }
        
        // Check for collisions with platforms
        if (projectile->has_collided_with_platform())
        {
            // Remove the projectile when it hits a platform
            delete projectile;
            m_game_state.projectiles.erase(m_game_state.projectiles.begin() + i);
            i--; // Adjust index since we removed an element
            continue;
        }
        
        
    }
}
bool LevelB::is_completed() {
    // For example, check if player reached a certain position
    for (int i = 0; i < ENEMY_B_COUNT; i++) {
        if (m_game_state.enemies[i].is_active()) {
            return false;
        }
    }
    return true;
}

void LevelB::render(ShaderProgram *g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    m_game_state.player->render_health_bar(g_shader_program, m_font_texture_id);
    
    for (int i = 0; i < ENEMY_B_COUNT; i++) {
        if(m_game_state.enemies[i].is_active())
        {
            m_game_state.enemies[i].render(g_shader_program);
            m_game_state.enemies[i].render_health_bar(g_shader_program, m_font_texture_id);
        }
    }
    
    // Render all projectiles
    // std::cout << "Rendering " << m_game_state.projectiles.size() << " projectiles" << std::endl;
    for (int i = 0; i < m_game_state.projectiles.size(); i++) {
        m_game_state.projectiles[i]->render(g_shader_program);
    }
}

void LevelB::shoot_projectile()
{
    if (m_game_state.projectiles.size() < 100) {
        // Create projectile on the heap with projectile texture
        GLuint projectile_texture_id = Utility::load_texture("assets/projectile.png");
        Entity* projectile = new Entity(projectile_texture_id, 5.0f, 1.0f, 1.0f, PROJECTILE);
        glm::vec3 player_pos = m_game_state.player->get_position();
        projectile->set_position(player_pos);
        
        // Get player's movement vector to determine direction
        glm::vec3 player_movement = m_game_state.player->get_movement();
        
        // If player is not moving, use the last direction they were facing
        if (player_movement.x == 0.0f && player_movement.y == 0.0f) {
            // Check which animation the player is currently using
            int* player_animation = m_game_state.player->get_animation_indices();
            
            // Compare with the walking animations for each direction
            // We need to compare the first element of the animation array
            if (player_animation[0] == m_game_state.player->get_walking_animation(RIGHT, 0)) {
                player_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            } else if (player_animation[0] == m_game_state.player->get_walking_animation(LEFT, 0)) {
                player_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            } else if (player_animation[0] == m_game_state.player->get_walking_animation(UP, 0)) {
                player_movement = glm::vec3(0.0f, 1.0f, 0.0f);
            } else if (player_animation[0] == m_game_state.player->get_walking_animation(DOWN, 0)) {
                player_movement = glm::vec3(0.0f, -1.0f, 0.0f);
            } else {
                // Default to right if no direction can be determined
                player_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }
        
        // Set the projectile's movement to match the player's direction
        projectile->set_movement(player_movement);
        
        // Set rotation based on direction
        if (player_movement.x > 0.0f) {
            // Right - no rotation needed
            projectile->set_rotation(0.0f);
            projectile->set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
        } else if (player_movement.x < 0.0f) {
            // Left - flip horizontally
            projectile->set_rotation(0.0f);
            projectile->set_scale(glm::vec3(-1.0f, 1.0f, 1.0f));
        } else if (player_movement.y > 0.0f) {
            // Up - rotate 90 degrees counterclockwise
            projectile->set_rotation(90.0f);
            projectile->set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
        } else if (player_movement.y < 0.0f) {
            // Down - rotate 90 degrees clockwise
            projectile->set_rotation(-90.0f);
            projectile->set_scale(glm::vec3(1.0f, 1.0f, 1.0f));
        }
        
        m_game_state.projectiles.push_back(projectile);
    }
}
