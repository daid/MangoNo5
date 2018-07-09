#ifndef PLAYER_PAWN_H
#define PLAYER_PAWN_H

#include <sp2/scene/node.h>

class PlayerInput;
class PlayerPawn : public sp::Node
{
public:
    PlayerPawn(sp::P<sp::Node> parent, PlayerInput& controller);
    
    virtual void onFixedUpdate() override;
    virtual void onCollision(sp::CollisionInfo& info) override;
private:
    PlayerInput& controller;
    int on_floor_counter = 0;
    int jump_request = 0;

    //X parameters
    static constexpr double ground_acceleration = 150.0;
    static constexpr double air_acceleration = 100.0;
    static constexpr double ground_deceleration = 100.0;
    static constexpr double air_deceleration = 10.0;
    static constexpr double max_run_speed = 12.0;

    //Y parameters
    static constexpr double time_to_apex = 0.6;
    static constexpr double jump_height_max = 5.5;
    static constexpr double jump_height_min = 0.5;
    
    static constexpr double gravity = 2*jump_height_max/(time_to_apex*time_to_apex);
    static constexpr double v_jump = std::sqrt(2*gravity*jump_height_max);
    static constexpr double v_jump_release = std::sqrt(v_jump*v_jump+2*-gravity*(jump_height_max-jump_height_min));
};

#endif//PLAYER_PAWN_H
