
#pragma once

#include <SDL2/SDL.h>

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
        model,
        render,
        rig,
        animate
    };

    struct Gui {
        Mode mode = Mode::model;
        bool sidebar_hidden = false;
    };
    Gui state;

    void render_gui();
    void gui_top();
    void gui_side();
};
