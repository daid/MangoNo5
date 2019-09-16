#include "main.h"
#include "playerInput.h"
#include "playerPawn.h"
#include "tileNames.h"

#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/logging.h>
#include <sp2/random.h>
#include <sp2/tweak.h>
#include <sp2/io/directoryResourceProvider.h>
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/graphics/mesh/obj.h>
#include <sp2/graphics/spriteAnimation.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/audio/sound.h>
#include <sp2/audio/music.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/tilemap.h>
#include <sp2/scene/camera.h>
#include <sp2/collision/2d/box.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/collision/2d/chains.h>
#include <sp2/io/keybinding.h>
#include <sp2/tween.h>
#include <sp2/math/plane.h>
#include <sp2/script/environment.h>

sp::P<sp::Window> window;

sp::io::Keybinding escape_key{"exit", "Escape"};

static void luaf_setTile(int x, int y, int tile_index, bool non_solid);
static void luaf_setPlatformTile(int x, int y, int tile_index);
static void luaf_removeTile(int x, int y);
static void luaf_setExit(double x, double y);
static sp::Vector2d luaf_getCameraPosition();
static void luaf_setCameraPosition(sp::Vector2d pos);
static void luaf_setTilemapRotation(sp::Vector2d center, double angle);
static int luaf_yield(lua_State* L)
{
    return lua_yield(L, 0);
}

class Challange : public sp::script::Environment
{
public:
    Challange(sp::string name, sp::Vector2d offset)
    : offset(offset)
    {
        tilemap = new sp::Tilemap(sp::Scene::get("MAIN")->getRoot(), "tilemap.png", 1.0, 1.0, 16, 16);
        tilemap->setPosition(offset);

        setGlobal("random", sp::random);
        setGlobal("irandom", sp::irandom);
        setGlobal("setTile", luaf_setTile);
        setGlobal("setPlatformTile", luaf_setPlatformTile);
        setGlobal("removeTile", luaf_removeTile);
        setGlobal("exit", luaf_setExit);
        setGlobal("getCameraPosition", luaf_getCameraPosition);
        setGlobal("setCameraPosition", luaf_setCameraPosition);
        setGlobal("setTilemapRotation", luaf_setTilemapRotation);
        setGlobal("yield", luaf_yield);

        active = this;
        load(sp::io::ResourceProvider::get("challanges/tilenames.lua"));
        load(sp::io::ResourceProvider::get("challanges/" + name + ".lua"));
        
        coroutine = callCoroutine("run");
    }
    
    ~Challange()
    {
        tilemap.destroy();
    }

    bool update()
    {
        active = this;
        if (coroutine && coroutine->resume())
            return false;
        return true;
    }
    
    static Challange* active;
    
    sp::P<sp::Tilemap> tilemap;
    sp::Vector2d exit;
    sp::Vector2d offset;
private:
    sp::script::CoroutinePtr coroutine;
};
Challange* Challange::active;

static void luaf_setTile(int x, int y, int tile_index, bool non_solid)
{
    Challange::active->tilemap->setTile(x, y, tile_index, non_solid ? sp::Tilemap::Collision::Open : sp::Tilemap::Collision::Solid);
}
static void luaf_setPlatformTile(int x, int y, int tile_index)
{
    Challange::active->tilemap->setTile(x, y, tile_index, sp::Tilemap::Collision::Platform);
}
static void luaf_removeTile(int x, int y)
{
    Challange::active->tilemap->setTile(x, y, -1);
}
static void luaf_setExit(double x, double y)
{
    Challange::active->exit = Challange::active->offset + sp::Vector2d(x, y);
}
static sp::Vector2d luaf_getCameraPosition()
{
    return sp::Scene::get("MAIN")->getCamera()->getPosition2D() - Challange::active->offset;
}
static void luaf_setCameraPosition(sp::Vector2d pos)
{
    sp::Scene::get("MAIN")->getCamera()->setPosition(Challange::active->offset + pos);
}
static void luaf_setTilemapRotation(sp::Vector2d center, double angle)
{
    Challange::active->tilemap->setRotation(angle);
    Challange::active->tilemap->setPosition(Challange::active->offset + center - center.rotate(angle));
}

class PlayerInfo
{
public:
    sp::string head_name = "elephant";
    int lives = 10;
    bool active = false;
    
    sp::P<PlayerPawn> pawn;
    sp::P<sp::gui::Widget> hud;
};
std::vector<PlayerInfo> player_info;

class GameScene : public sp::Scene
{
public:
    GameScene()
    : sp::Scene("MAIN")
    {
        camera = new sp::Camera(getRoot());
        setDefaultCamera(camera);
        camera->setOrtographic(14.0, sp::Camera::Direction::Horizontal);
        camera->setPosition(sp::Vector2d(14, 10));
        
        sp::collision::Chains2D camera_collision;
        sp::collision::Chains2D::Path path{{-14,-15}, {14,-15}, {14,13}, {-14,13}};
        camera_collision.loops.push_back(path);
        camera->setCollisionShape(camera_collision);
        
        active_challange = new Challange("start", sp::Vector2d(0, 0));
        hud = sp::gui::Loader::load("gui/hud.gui", "HUD");

        for(unsigned int n=0; n<player_info.size(); n++)
        {
            PlayerInfo& info = player_info[n];
            if (!info.active)
                continue;
            
            info.pawn = new PlayerPawn(getRoot(), PlayerInput::controllers[n], info.head_name);
            info.pawn->setPosition(sp::Vector2d(8, 8));
            info.hud = sp::gui::Loader::load("gui/hud.gui", "ENTRY", hud);
            info.hud->getWidgetWithID("IMAGE")->setAttribute("texture", "player/head/" + info.head_name + ".png");
            info.hud->getWidgetWithID("COUNTER")->setAttribute("caption", sp::string(info.lives));
            info.hud->getWidgetWithID("BAR")->setAttribute("value", sp::string("1.0"));
        }
    }
    
    ~GameScene()
    {
        hud.destroy();
    }
    
    virtual void onFixedUpdate() override
    {
        if (active_challange->update())
        {
            previous_challange.destroy();
            previous_challange = active_challange;
            
            if (sp::random(0, 100) < 50)
                active_challange = new Challange("platforms", previous_challange->exit);
            else if (sp::random(0, 100) < 50)
                active_challange = new Challange("tumbler", previous_challange->exit);
            else
                active_challange = new Challange("goingup", previous_challange->exit);
        }

        for(unsigned int n=0; n<player_info.size(); n++)
        {
            PlayerInfo& info = player_info[n];
            if (!info.active)
                continue;
            if (!info.pawn)
            {
                info.lives--;
                info.pawn = new PlayerPawn(getRoot(), PlayerInput::controllers[n], info.head_name);
                info.pawn->respawn();
                
                info.hud->getWidgetWithID("COUNTER")->setAttribute("caption", sp::string(info.lives));
                
                int max_lives = 0;
                for(PlayerInfo& i : player_info)
                    if (i.active)
                        max_lives = std::max(max_lives, i.lives);

                for(PlayerInfo& i : player_info)
                    if (i.active)
                        i.hud->getWidgetWithID("BAR")->setAttribute("value", sp::string(float(i.lives) / float(max_lives)));
            }
        }
    }

private:
    sp::P<sp::Camera> camera;
    sp::P<Challange> active_challange;
    sp::P<Challange> previous_challange;
    
    sp::P<sp::gui::Widget> hud;
};

int main(int argc, char** argv)
{
    sp::P<sp::Engine> engine = new sp::Engine();
    //Create resource providers, so we can load things.
    new sp::io::DirectoryResourceProvider("resources");

    sp::texture_manager.setDefaultSmoothFiltering(false);

    //Create a window to render on, and our engine.
    window = new sp::Window(4.0/3.0);
#ifndef DEBUG
    window->setFullScreen(true);
    window->hideCursor();
#endif
    
    sp::gui::Theme::loadTheme("default", "gui/theme/basic.theme.txt");
    new sp::gui::Scene(sp::Vector2d(640, 480));

    sp::P<sp::SceneGraphicsLayer> scene_layer = new sp::SceneGraphicsLayer(1);
    scene_layer->addRenderPass(new sp::BasicNodeRenderPass());
#ifdef DEBUG
    scene_layer->addRenderPass(new sp::CollisionRenderPass());
#endif
    window->addLayer(scene_layer);
    window->setClearColor(sp::Color(0.25,0.25,0.25));

    player_info.resize(3);
    for(unsigned int n=0; n<player_info.size(); n++)
    {
        for(bool retry=true; retry;)
        {
            retry = false;
            player_info[n].head_name = PlayerPawn::getRandomHeadName();
            for(unsigned int m=0; m<n; m++)
                if (player_info[n].head_name == player_info[m].head_name)
                    retry = true;
        }
    }
    player_info[0].active = true;
    //player_info[1].active = true;
    //player_info[2].active = true;

    new GameScene();
    
    engine->run();

    return 0;
}
