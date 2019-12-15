
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

    struct Gui {
        Mode mode = Mode::scene;
    };
    Gui state;

    Scene scene;

    void render_gui();
    bool state_button(Mode mode, std::string name);
};
