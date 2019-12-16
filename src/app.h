
#pragma once

#include <map>
#include <string>

#include <SDL2/SDL.h>

#include "scene/scene.h"

class Platform;
class App {
public:
    App(Platform& plt);
    ~App();

    void render();
    void event(SDL_Event e);

private:
    Platform& plt;

    enum class Mode {
        scene,
        model,
        render,
        rig,
        animate
    };
    enum class Camera_Control {
        none,
        orbit,
        move
    };

    struct Gui {
        Mode mode = Mode::scene;
        
        Camera_Control cam_mode = Camera_Control::none;
        Vec2 mouse, last_mouse;
    };
    Gui state;

    Scene scene;

    void render_gui();
    bool state_button(Mode mode, std::string name);
};
