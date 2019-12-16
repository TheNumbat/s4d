
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
        float dx = (e.motion.x - state.mouse.x);
        float dy = (e.motion.y - state.mouse.y);
        
        if(state.cam_mode == Camera_Control::orbit) {
            scene.camera_orbit(Vec2(dx, dy));
        } else if(state.cam_mode == Camera_Control::move) {
            scene.camera_move(Vec2(dx, dy));
        }

        state.mouse.x = (float)e.motion.x;
        state.mouse.y = (float)e.motion.y;
    } break;

    case SDL_MOUSEBUTTONDOWN: {
        if(e.button.button == SDL_BUTTON_LEFT) break;

        if(!io.WantCaptureMouse) {
            if(e.button.button == SDL_BUTTON_RIGHT) {
                state.cam_mode = Camera_Control::orbit;
            } else if(e.button.button == SDL_BUTTON_MIDDLE) {
                state.cam_mode = Camera_Control::move;
            }
            plt.capture_mouse();
        }
        state.last_mouse.x = (float)e.button.x;
        state.last_mouse.y = (float)e.button.y;
    } break;

    case SDL_MOUSEBUTTONUP: {
        if(!io.WantCaptureMouse && state.cam_mode != Camera_Control::none) {
            state.cam_mode = Camera_Control::none;
            plt.release_mouse();
            plt.set_mouse(state.last_mouse);
        }
    } break;

    case SDL_MOUSEWHEEL: {
        if(!io.WantCaptureMouse) {
            scene.camera_radius((float)e.wheel.y);
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
            if(ImGui::MenuItem("Reload Shaders")) {
                scene.reload_shaders();
            }
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


