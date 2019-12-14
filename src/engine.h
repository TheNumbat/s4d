
#pragma once

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

class Engine {

public:
    Engine();
    ~Engine();

    void loop();

private:
    void platform_init();
    void platform_shutdown();
    void begin_frame();
    void complete_frame();

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    
    bool has_gl_43 = false;
};
