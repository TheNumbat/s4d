
#pragma once

#include <SDL2/SDL.h>

#include "lib/math.h"
#include "app.h"

class Platform {

public:
    Platform();
    ~Platform();

    void loop(App& app);

    Vec2 window_dim();

    void capture_mouse();
    void release_mouse();
    void set_mouse(Vec2 pos);

private:
    void platform_init();
    void platform_shutdown();
    void begin_frame();
    void complete_frame();

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool has_gl_43 = false;
};
