#pragma once
#include "Scene.h"

class GameOver : public Scene {
private:
    bool m_is_win;
    
public:
    // Constructor with win/lose parameter
    GameOver(bool is_win);
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void shoot_projectile() override;

};
