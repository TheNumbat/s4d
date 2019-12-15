
#include "app.h"
#include "platform.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>

App::App(Platform& plt) : plt(plt) {
}

App::~App() {
}

void App::event(SDL_Event e) {
    
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

    scene.gui(plt.window_dim());
}


