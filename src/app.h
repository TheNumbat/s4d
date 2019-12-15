
#pragma once

#include <map>
#include <string>

#include <SDL2/SDL.h>

#include "scene_object.h"

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

    std::map<Scene_Object::ID, Scene_Object> objs;
    Scene_Object::ID next_id = 1;
    Scene_Object::ID selected_id = 0;

    void render_gui();
    void gui_top();
    void gui_side();
    bool state_button(Mode mode, std::string name);

    void add_object(const Scene_Object& obj);
};
