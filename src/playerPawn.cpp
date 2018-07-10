#include "playerPawn.h"
#include "playerInput.h"

#include <sp2/alignment.h>
#include <sp2/random.h>
#include <sp2/engine.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/camera.h>
#include <sp2/graphics/spriteAnimation.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/collision/2d/box.h>


class HeadInfo
{
public:
    sp::string name;
    sp::Vector2f size;
    sp::Alignment alignment;
    sp::Color color;
};
HeadInfo head_info[] = {
    {"elephant", sp::Vector2f(424, 342), sp::Alignment::Center, sp::Color::fromString("#bdb6b6")},
    {"giraffe",  sp::Vector2f(382, 370), sp::Alignment::Bottom, sp::Color::fromString("#e9bb00")},
    {"hippo",    sp::Vector2f(333, 344), sp::Alignment::Center, sp::Color::fromString("#4178c2")},
    {"monkey",   sp::Vector2f(392, 284), sp::Alignment::Center, sp::Color::fromString("#915b32")},
    {"panda",    sp::Vector2f(392, 331), sp::Alignment::Bottom, sp::Color::fromString("#eeeeee")},
    {"parrot",   sp::Vector2f(284, 284), sp::Alignment::Center, sp::Color::fromString("#d23007")},
    {"penguin",  sp::Vector2f(284, 284), sp::Alignment::Center, sp::Color::fromString("#365a66")},
    {"pig",      sp::Vector2f(359, 305), sp::Alignment::Bottom, sp::Color::fromString("#d987d9")},
    {"rabbit",   sp::Vector2f(284, 414), sp::Alignment::Bottom, sp::Color::fromString("#bdbdbd")},
    {"snake",    sp::Vector2f(284, 334), sp::Alignment::Top,    sp::Color::fromString("#66cc33")},
};

PlayerPawn::PlayerPawn(sp::P<sp::Node> parent, PlayerInput& controller)
: sp::Node(parent), controller(controller)
{
    animation = sp::SpriteAnimation::load("player/body.txt");
    animation->play("Stand");

    HeadInfo& i = head_info[sp::irandom(0, 9)];
    
    sp::Node* head = new sp::Node(this);
    head->render_data.type = sp::RenderData::Type::Normal;
    head->render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(i.size.x / 256.0, i.size.y / 256.0));
    head->render_data.texture = sp::texture_manager.get("player/head/" + i.name + ".png");
    head->render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.color = i.color;
    
    //head->setRotation(sp::Quaterniond::fromAxisAngle(sp::Vector3d(0, 1, 0), 35));
    if (i.alignment == sp::Alignment::Center)
        head->setPosition(sp::Vector3d(0, 0.25, 0.1));
    if (i.alignment == sp::Alignment::Bottom)
        head->setPosition(sp::Vector3d(0, 0.25 + (i.size.y / 256.0 - 1) * 0.5, 0.1));
    if (i.alignment == sp::Alignment::Top)
        head->setPosition(sp::Vector3d(0, 0.25 - (i.size.y / 256.0 - 1) * 0.5, 0.1));
    
    sp::collision::Box2D shape(0.8, 1.25, 0, -0.125);
    shape.fixed_rotation = true;
    setCollisionShape(shape);
}

void PlayerPawn::onFixedUpdate()
{
    sp::Vector2d velocity = getLinearVelocity2D();
    
    if (controller.attack.getDown() && attacking == 0)
    {
        attacking = 5;
        sp::Vector2d pos = getPosition2D();
        if (animation->getFlags() & sp::SpriteAnimation::FlipFlag)
            pos.x -= 1.0;
        else
            pos.x += 1.0;
        getScene()->queryCollision(pos - sp::Vector2d(0.5,0.5), pos + sp::Vector2d(0.5,0.5), [this](sp::P<sp::Node> object) -> bool
        {
            sp::P<PlayerPawn> player = object;
            if (player && player != this)
            {
                if (animation->getFlags() & sp::SpriteAnimation::FlipFlag)
                    player->setLinearVelocity(player->getLinearVelocity2D() + sp::Vector2d(-30, 15));
                else
                    player->setLinearVelocity(player->getLinearVelocity2D() + sp::Vector2d(30, 15));
                return false;
            }
            return true;
        });
    }
    
    //X handling
    double movement_request = controller.right.getValue() - controller.left.getValue();
    double acceleration = air_acceleration;
    double decaleration = air_deceleration;
    if (on_floor_counter > 0)
    {
        acceleration = ground_acceleration;
        decaleration = ground_deceleration;
    }
    
    bool x_flip = velocity.x < 0;
    if (x_flip)
    {
        velocity.x = -velocity.x;
        movement_request = -movement_request;
    }
    
    velocity.x = std::max(0.0, velocity.x - ((1.0 - movement_request) * decaleration * sp::Engine::fixed_update_delta));
    if (velocity.x < max_run_speed)
        velocity.x += movement_request * acceleration * sp::Engine::fixed_update_delta;
    
    if (x_flip)
    {
        velocity.x = -velocity.x;
        movement_request = -movement_request;
    }

    //Y handling
    velocity.y -= gravity * sp::Engine::fixed_update_delta;
    if (controller.jump.getDown())
        jump_request = 5;
    if (jump_request)
    {
        jump_request--;
        if (on_floor_counter > 0 || double_jump > 0)
        {
            velocity.y += v_jump + gravity * sp::Engine::fixed_update_delta;
            jump_request = 0;
            double_jump = 0;
        }
    }
    if (controller.jump.getUp() && velocity.y > v_jump_release)
        velocity.y = v_jump_release;

    //Final.
    setLinearVelocity(velocity);
    if (movement_request < 0)
        animation->setFlags(sp::SpriteAnimation::FlipFlag);
    else if (movement_request > 0)
        animation->setFlags(0);
    if (on_floor_counter > 0)
    {
        if (movement_request)
            animation->play("Walk", std::abs(velocity.x) / max_run_speed);
        else
            animation->play("Stand");
    }
    else
    {
        animation->play("Jump");
    }
    if (attacking > 0)
    {
        attacking--;
        animation->play("Attack");
    }
    
    if (on_floor_counter > 0)
    {
        double_jump = 1;
        on_floor_counter -= 1;
    }
        
    
    //TMP
    sp::Vector2d camera_position = getScene()->getCamera()->getPosition2D();
    if (getPosition2D().y < camera_position.y - 12)
    {
        setPosition(camera_position + sp::Vector2d(-10, 9.5));
    }
}

void PlayerPawn::onCollision(sp::CollisionInfo& info)
{
    if (info.other && info.other->isSolid())
    {
        if (info.normal.y < -0.4)
        {
            if (sp::P<PlayerPawn>(info.other))
            {
                if (getLinearVelocity2D().y < 0)
                {
                    if (controller.jump.get())
                        setLinearVelocity(getLinearVelocity2D() + sp::Vector2d(0, v_jump));
                    else
                        setLinearVelocity(getLinearVelocity2D() + sp::Vector2d(0, (v_jump_release + v_jump) * 0.5));
                }
            }
            else
                on_floor_counter = 3;
        }
    }
}
