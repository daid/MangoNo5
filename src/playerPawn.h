#ifndef PLAYER_PAWN_H
#define PLAYER_PAWN_H

#include <sp2/scene/node.h>

class PlayerInput;
class PlayerPawn : public sp::Node
{
public:
    PlayerPawn(sp::P<sp::Node> parent, PlayerInput& controller, sp::string head_name);
    
    virtual void onFixedUpdate() override;
    virtual void onCollision(sp::CollisionInfo& info) override;
    
    void respawn();
    
    static sp::string getRandomHeadName();
private:
    PlayerInput& controller;
    int on_floor_counter = 0;
    int jump_request = 0;
    int double_jump = 0;
    int attacking = 0;
    int respawn_delay = 0;
    sp::P<sp::Node> head;

    //X parameters
    static constexpr double ground_acceleration = 70.0;
    static constexpr double air_acceleration = 50.0;
    static constexpr double ground_deceleration = 100.0;
    static constexpr double air_deceleration = 20.0;
    static constexpr double max_run_speed = 15.0;

    //Y parameters
    static constexpr double time_to_apex = 0.25;
    static constexpr double jump_height_max = 4.5;
    static constexpr double jump_height_min = 0.5;
    
    static constexpr double gravity = 2*jump_height_max/(time_to_apex*time_to_apex);
    static constexpr double v_jump = std::sqrt(2*gravity*jump_height_max);
    static constexpr double v_jump_release = std::sqrt(v_jump*v_jump+2*-gravity*(jump_height_max-jump_height_min));
};

#endif//PLAYER_PAWN_H
