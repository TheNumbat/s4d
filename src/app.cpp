
#include "app.h"
#include "platform.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>

App::App(Platform& plt) : 
    plt(plt), 
    scene(plt.window_dim()) {
}

App::~App() {
}

void App::event(SDL_Event e) {
    
    ImGuiIO& io = ImGui::GetIO();

    switch(e.type) {
	case SDL_WINDOWEVENT: {
        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

            scene.apply_window_dim(Vec2((float)e.window.data1, (float)e.window.data2));
        }
    } break;

    case SDL_MOUSEMOTION: {
        if(state.mouse_captured) {
            float dx = (e.motion.x - state.mouse.x);
            float dy = (e.motion.y - state.mouse.y);
            scene.move_camera(Vec2(dx, dy));
        }
        state.mouse.x = (float)e.motion.x;
        state.mouse.y = (float)e.motion.y;
    } break;

    case SDL_MOUSEBUTTONDOWN: {
        if(!io.WantCaptureMouse) {
            state.mouse_captured = true;
            plt.capture_mouse();
        }
        state.last_mouse.x = (float)e.button.x;
        state.last_mouse.y = (float)e.button.y;
    } break;

    case SDL_MOUSEBUTTONUP: {
        if(!io.WantCaptureMouse && state.mouse_captured) {
            state.mouse_captured = false;
            plt.release_mouse();
            plt.set_mouse(state.last_mouse);
        }
    } break;
    }
}

void App::render() {

    scene.render();
    render_gui();
}

bool App::state_button(Mode mode, std::string name) {
    
    bool active = state.mode == mode;
    
    if(active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));

    bool clicked = ImGui::SmallButton(name.c_str());

    if(active) ImGui::PopStyleColor();

    return clicked;
}

void App::render_gui() {

    if(ImGui::BeginMainMenuBar()) {

        if(ImGui::BeginMenu("File")) {

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Edit")) {

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("View")) {

            ImGui::EndMenu();
        }

        if(state_button(Mode::scene, "Scene"))
            state.mode = Mode::scene;

        if(state_button(Mode::model, "Model"))
            state.mode = Mode::model;

        if(state_button(Mode::render, "Render"))
            state.mode = Mode::render;

        if(state_button(Mode::rig, "Rig"))
            state.mode = Mode::rig;

        if(state_button(Mode::animate, "Animate"))
            state.mode = Mode::animate;

        ImGui::EndMainMenuBar();
    }

    scene.gui();
}


