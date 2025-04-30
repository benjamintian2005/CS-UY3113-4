#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Utility.h"

// External declaration of the global current scene

void Entity::ai_activate(Entity *player)
{
    switch (m_ai_type)
    {
        case WALKER:
            ai_walk();
            break;
            
        case GUARD:
            ai_guard(player);
            break;
        case BOSS1:
            ai_boss1(player);
            break;
        case BOSS2:
            ai_boss2(player);
            break;
        default:
            break;
    }
}

void Entity::ai_walk()
{
    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Entity::ai_guard(Entity *player)
{
    switch (m_ai_state) {
        case IDLE: {
            if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
            break;
        }
            
        case WALKING: {
            // Calculate direction vector from enemy to player
            float dx = player->get_position().x - m_position.x;
            float dy = player->get_position().y - m_position.y;
            
            // Determine primary direction to move
            if (fabs(dx) > fabs(dy)) {
                // Move horizontally
                if (dx > 0) {
                    m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                    face_right();
                } else {
                    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                    face_left();
                }
            } else {
                // Move vertically
                if (dy > 0) {
                    m_movement = glm::vec3(0.0f, 1.0f, 0.0f);
                    face_up();
                } else {
                    m_movement = glm::vec3(0.0f, -1.0f, 0.0f);
                    face_down();
                }
            }
            break;
        }
            
        case ATTACKING: {
            break;
        }
            
        default: {
            break;
        }
    }
}

void Entity::ai_boss1(Entity *player)
{
    if (player) {
        // Calculate direction vector from boss to player
        float dx = player->get_position().x - m_position.x;
        float dy = player->get_position().y - m_position.y;
        
        // Calculate distance to player
        float distance = glm::distance(m_position, player->get_position());
        
        // Determine sped based on distance
        float current_speed_multiplier;
        
        if (distance > 2.0f) {
            // Speed up when far away (>6.0f)
            current_speed_multiplier = 7.5f;
        } else if (distance < 1.5f) {
            // Slow down when very close (<1.5f)
            current_speed_multiplier = 0.5f;
        } else {
            // Normal speed at medium distances
            current_speed_multiplier = 1.0f;
        }
        
        // Only proceed if player is not at the same position
        if (distance > 0.1f) {
            // Normalized direction vector
            float dirX = dx / distance;
            float dirY = dy / distance;
            
            // Apply speed multiplier
            m_movement = glm::vec3(dirX * current_speed_multiplier, dirY * current_speed_multiplier, 0.0f);
            
            // Update facing direction based on movement
            if (fabs(dirX) > fabs(dirY)) {
                // Horizontal movement is dominant
                if (dirX > 0) {
                    face_right();
                } else {
                    face_left();
                }
            } else {
                // Vertical movement is dominant
                if (dirY > 0) {
                    face_up();
                } else {
                    face_down();
                }
            }
        } else {
            // Player is very close, stop moving
            m_movement = glm::vec3(0.0f);
        }
    }
}

void Entity::ai_boss2(Entity *player)
{
    // Directions: 0 = right, 1 = left, 2 = up, 3 = down
    const float CHARGE_SPEED = 5.0f;         // Standard charge speed
    const float BURST_SPEED = 8.0f;          // Occasional speed bursts
    const float DIRECTION_CHANGE_CHANCE = 0.02f;  // 2% chance per frame to change direction
    
    // Check if we're not moving at all (initial state)
    bool not_moving = (fabs(m_movement.x) < 0.01f && fabs(m_movement.y) < 0.01f);
    
    // Random direction change based on probability
    bool random_direction_change = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) < DIRECTION_CHANGE_CHANCE;
    
    // Determine if we should change direction
    bool should_change_direction =
        m_collided_left ||
        m_collided_right ||
        m_collided_top ||
        m_collided_bottom ||
        not_moving ||
        random_direction_change;
    
    if (should_change_direction) {
        // Choose a random direction
        int new_direction = rand() % 4;
        
        // Make sure we don't pick the same direction if we just collided
        if (m_collided_right && new_direction == 0) new_direction = 1;    // Don't go right if we hit right
        if (m_collided_left && new_direction == 1) new_direction = 0;     // Don't go left if we hit left
        if (m_collided_top && new_direction == 2) new_direction = 3;      // Don't go up if we hit top
        if (m_collided_bottom && new_direction == 3) new_direction = 2;   // Don't go down if we hit bottom
        
        // Add some variation to movement patterns
        // 10% chance to do a diagonal movement
        bool diagonal_movement = (rand() % 10 == 0);
        
        // Set movement vector based on direction
        switch (new_direction) {
            case 0:  // Right
                if (diagonal_movement) {
                    // Diagonal up-right or down-right
                    m_movement = glm::vec3(0.7f, (rand() % 2 == 0) ? 0.7f : -0.7f, 0.0f);
                } else {
                    m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                }
                face_right();
                break;
            case 1:  // Left
                if (diagonal_movement) {
                    // Diagonal up-left or down-left
                    m_movement = glm::vec3(-0.7f, (rand() % 2 == 0) ? 0.7f : -0.7f, 0.0f);
                } else {
                    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                }
                face_left();
                break;
            case 2:  // Up
                if (diagonal_movement) {
                    // Diagonal up-right or up-left
                    m_movement = glm::vec3((rand() % 2 == 0) ? 0.7f : -0.7f, 0.7f, 0.0f);
                } else {
                    m_movement = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                face_up();
                break;
            case 3:  // Down
                if (diagonal_movement) {
                    // Diagonal down-right or down-left
                    m_movement = glm::vec3((rand() % 2 == 0) ? 0.7f : -0.7f, -0.7f, 0.0f);
                } else {
                    m_movement = glm::vec3(0.0f, -1.0f, 0.0f);
                }
                face_down();
                break;
        }
        
        // Normalize the movement vector for consistent speed
        float length = sqrt(m_movement.x * m_movement.x + m_movement.y * m_movement.y);
        if (length > 0) {
            m_movement.x /= length;
            m_movement.y /= length;
        }
        
        // Random chance for speed variation (20% chance for a speed burst)
        m_speed = (rand() % 5 == 0) ? BURST_SPEED : CHARGE_SPEED;
        
        // Reset collision flags after handling them
        m_collided_left = false;
        m_collided_right = false;
        m_collided_top = false;
        m_collided_bottom = false;
    }
    
    // Small chance to adjust speed during movement (without changing direction)
    if (rand() % 100 < 5) {  // 5% chance per frame
        // Random speed between 70% and 130% of base speed
        float speed_variation = 0.7f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.6f;
        m_speed *= speed_variation;
        
        // Clamp speed to reasonable range
        if (m_speed < 2.0f) m_speed = 2.0f;
        if (m_speed > 10.0f) m_speed = 10.0f;
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f),
    m_scene(nullptr)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

// Parameterized constructor
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_acceleration(acceleration), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_texture_id(texture_id), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType), m_scene(nullptr)
{
    face_right();
}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed,  float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType), m_scene(nullptr)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState): m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType), m_ai_type(AIType), m_ai_state(AIState), m_scene(nullptr)
{
// Initialize m_walking with zeros or any default value
for (int i = 0; i < SECONDS_PER_FRAME; ++i)
    for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (collidable_entity->is_active() and check_collision(collidable_entity))
        {
            
            if(collidable_entity->is_active() and collidable_entity->m_entity_type == ENEMY and m_entity_type == PLAYER)
            {
                take_damage(1);
            }
            else
            {
                float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
                float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
                
                if (m_position.y > collidable_entity->m_position.y)
                {
                    m_position.y += y_overlap;
                    m_velocity.y = 0;
                    m_collided_bottom = true;
                }
                else if (m_position.y < collidable_entity->m_position.y)
                {
                    m_position.y -= y_overlap;
                    m_velocity.y = 0;
                    m_collided_top = true;
                }
            }
        }
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (collidable_entity->is_active() and check_collision(collidable_entity))
        {
            if(collidable_entity->is_active() and collidable_entity->m_entity_type == ENEMY and m_entity_type == PLAYER)
            {
                take_damage(1);
            }
            else
            {
                float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
                float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
                
                if (m_position.x > collidable_entity->m_position.x)
                {
                    m_position.x += x_overlap;
                    m_velocity.x = 0;
                    m_collided_left = true;
                }
                else if (m_position.x < collidable_entity->m_position.x)
                {
                    m_position.x -= x_overlap;
                    m_velocity.x = 0;
                    m_collided_right = true;
                }
            }
        }
    }
}

void const Entity::check_collision_y(Map *map)
{
    // Check the tiles above and below
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // Check if we've collided with any of the map tiles
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= fabs(m_velocity.y * 0.1f);
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += fabs(m_velocity.y * 0.1f);
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
}

void const Entity::check_collision_x(Map *map)
{
    // Check the tiles to the left and right
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // Check if we've collided with any of the map tiles
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += fabs(m_velocity.x * 0.1f);
        m_velocity.x = 0;
        m_collided_left = true;
    }
    else if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= fabs(m_velocity.x * 0.1f);
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

void Entity::attack()
{
    if (!m_is_attacking && m_attack_cooldown <= 0.0f) {
        m_is_attacking = true;
        m_attack_timer = 0.3f;
        m_attack_cooldown = 0.5f;  // Can't attack again for this time
    }
}

void Entity::take_damage(int damage)
{
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_is_active = false;  // Disable entity when health reaches 0
    }
}

void Entity::update_attack(float delta_time)
{
    if (m_attack_cooldown > 0.0f) {
        m_attack_cooldown -= delta_time;
    }
    
    if (m_is_attacking) {
        m_attack_timer -= delta_time;
        if (m_attack_timer <= 0.0f) {
            m_is_attacking = false;
        }
    }
}

void Entity::update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map)
{
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
    
    
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
    
    
    
    // Add movement to velocity
    m_velocity = m_movement * m_speed;
    
    // Debug prints for projectiles
    if (m_entity_type == PROJECTILE) {
        //std::cout << "Projectile update - Position: (" << m_position.x << ", " << m_position.y << ")" << std::endl;
        //std::cout << "Projectile update - Movement: (" << m_movement.x << ", " << m_movement.y << ")" << std::endl;
        //std::cout << "Projectile update - Velocity: (" << m_velocity.x << ", " << m_velocity.y << ")" << std::endl;
    }
    
    // Update position based on velocity
    m_position.x += m_velocity.x * delta_time;
    check_collision_x(map);
    if (collidable_entities != nullptr) {
        check_collision_x(collidable_entities, collidable_entity_count);
    }

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(map);
    if (collidable_entities != nullptr) {
        check_collision_y(collidable_entities, collidable_entity_count);
    }
    
    if (m_entity_type == ENEMY) ai_activate(player);

    

    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
    
    if (m_animation_indices != nullptr)
    {
        if (glm::length(m_velocity) != 0)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;
            
            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;
                
                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }
    }
    
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
    
    // Update model matrix with current position and scale
    m_model_matrix = glm::mat4(1.0f);
    
    // Apply translation first
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    
    // Apply rotation if needed
    if (m_rotation != 0.0f) {
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // Then apply scale
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    
    // Debug print model matrix for projectiles
    if (m_entity_type == PROJECTILE) {
        //std::cout << "Updated model matrix for projectile:" << std::endl;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                //std::cout << m_model_matrix[i][j] << " ";
            }
            //std::cout << std::endl;
        }
    }
}


void Entity::render(ShaderProgram* program)
{
    // Set the model matrix before rendering
    program->set_model_matrix(m_model_matrix);
    
   

    if (m_animation_indices != nullptr && m_animation_index >= 0 && m_animation_index < m_animation_frames)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
    }
    else
    {
        // Fallback to rendering a simple quad if no animation is set up
        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }
    
    // Special rendering for projectiles to make them more visible
    if (m_entity_type == PROJECTILE)
    {
        // Draw a bright circle around the projectile
        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        
        // Use a bright color for the projectile
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // Bright red
        
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        
        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
        
        // Reset color
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void Entity::render_health_bar(ShaderProgram* program, GLuint font_texture_id)
{
    // Only render health for players and enemies
    if (m_entity_type != PLAYER && m_entity_type != ENEMY) return;
    
    // Create health text
    std::string health_text = std::to_string(m_health) + "HP";
    
    // Position the text above the entity
    glm::vec3 text_position = m_position;
    text_position.y += m_height/2 + 0.2f;
    
    // Draw the health text
    Utility::draw_text(program, font_texture_id, health_text, 0.4f, 0.0f, text_position);
}
