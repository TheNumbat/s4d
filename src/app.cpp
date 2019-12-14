
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

    render_gui();
}

void App::render_gui() {

    gui_side();
    gui_top();
}

void App::gui_top() {

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

        if(ImGui::SmallButton("Model"))
            state.mode = Mode::model;

        if(ImGui::SmallButton("Render"))
            state.mode = Mode::render;

        if(ImGui::SmallButton("Rig"))
            state.mode = Mode::rig;

        if(ImGui::SmallButton("Animate"))
            state.mode = Mode::animate;

        ImGui::EndMainMenuBar();
    }
}

void App::gui_side() {

    v2 dim = plt.window_dim();

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos({0.0, 18.0});
    ImGui::SetNextWindowSize({dim.x / 5.0f, dim.y});
    ImGui::SetNextWindowCollapsed(state.sidebar_hidden);

    switch(state.mode) {
    case Mode::model: {
        ImGui::Begin("Model", nullptr, flags);
    } break;
    case Mode::render: {
        ImGui::Begin("Render", nullptr, flags);
    } break;
    case Mode::rig: {
        ImGui::Begin("Rig", nullptr, flags);
    } break;
    case Mode::animate: {
        ImGui::Begin("Animate", nullptr, flags);
    } break;
    }

    state.sidebar_hidden = ImGui::IsWindowCollapsed();
    ImGui::End();
}

