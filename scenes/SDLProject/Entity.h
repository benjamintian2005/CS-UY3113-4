#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include "Utility.h"

class Scene; // Forward declaration

enum EntityType { PLATFORM, PLAYER, ENEMY, PROJECTILE };
enum AIType     { WALKER, GUARD, BOSS1, BOSS2};
enum AIState    { WALKING, IDLE, ATTACKING };
enum EntityState    { WALK, IDLES, ATTACK };



enum AnimationDirection { LEFT, RIGHT, UP, DOWN };

class Entity
{
private:
    bool m_is_active = true;
    
    int m_walking[4][7]; // 4x4 array for walking animations

    EntityState m_state = IDLES;
    float m_attack_cooldown = 0.0f;
    bool m_is_attacking = false;
    float m_attack_timer = 0.0f;
    int m_health = 1000;
    
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;
    float m_rotation = 0.0f; // Rotation in degrees

    float     m_speed;
    
    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    float m_width = 1.0f,
          m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;

    Scene* m_scene = nullptr; // Pointer to the scene this entity belongs to

public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float animation_time,
        int animation_frames, int animation_index, int animation_cols,
           int animation_rows, float width, float height, EntityType EntityType);
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState); // AI constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;
    
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    
    // Overloading our methods to check for only the map
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    
    void update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map);
    void render(ShaderProgram* program);
    void render_health_bar(ShaderProgram* program, GLuint font_texture_id);

    void ai_activate(Entity *player);
    void ai_walk();
    void ai_guard(Entity *player);
    void ai_boss1(Entity *player);
    void ai_boss2(Entity *player);

    
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    void face_left() { m_animation_indices = m_walking[LEFT]; }
    void face_right() { m_animation_indices = m_walking[RIGHT]; }
    void face_up() { m_animation_indices = m_walking[UP]; }
    void face_down() { m_animation_indices = m_walking[DOWN]; }

    void move_left() { m_movement.x = -1.0f; face_left(); }
    void move_right() { m_movement.x = 1.0f;  face_right(); }
    void move_up() { m_movement.y = 1.0f;  face_up(); }
    void move_down() { m_movement.y = -1.0f; face_down(); }
    
    //Atack stuff//
    void attack();
    bool is_attacking() const { return m_is_attacking; }
    void take_damage(int damage);
    bool is_alive() const { return m_health > 0; }
    int get_health() const { return m_health; }
    void set_health(int health) { m_health = health; }
    void update_attack(float delta_time);
    
    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
    AIType     const get_ai_type()        const { return m_ai_type;       };
    AIState    const get_ai_state()       const { return m_ai_state;      };
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    int*      const get_animation_indices() const { return m_animation_indices; }
    int       const get_animation_index() const { return m_animation_index; }
    int       const get_walking_animation(int direction, int frame) const { return m_walking[direction][frame]; }
    bool      const has_collided_with_platform() const { return m_collided_top || m_collided_bottom || m_collided_left || m_collided_right; }
    float     const get_rotation() const { return m_rotation; }
    
    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    bool is_active() const { return m_is_active; }
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;};
    void const set_ai_type(AIType new_ai_type){ m_ai_type = new_ai_type;};
    void const set_ai_state(AIState new_state){ m_ai_state = new_state;};
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }
    void const set_rotation(float new_rotation) { m_rotation = new_rotation; }

    // Setter for m_walking
    void set_walking(int walking[4][7])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 7; ++j)
            {
                m_walking[i][j] = walking[i][j];
            }
        }
    }

    // Scene pointer getter and setter
    Scene* get_scene() const { return m_scene; }
    void set_scene(Scene* scene) { m_scene = scene; }
};

#endif // ENTITY_H
