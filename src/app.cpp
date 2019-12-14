
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

    for(auto& entry : objs) {
        entry.second.render();
    }
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

        if(ImGui::SmallButton("Scene"))
            state.mode = Mode::scene;

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

    Vec2 dim = plt.window_dim();

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos({0.0, 18.0});
    ImGui::SetNextWindowSize({dim.x / 5.0f, dim.y});

    ImGui::Begin("Objects", nullptr, flags);

    if(ImGui::Button("Create Object")) {

    }
    if(ImGui::Button("Load Object")) {

    }

    if(objs.size() > 0)
        ImGui::Separator();

    int i = 0;
    for(auto& entry : objs) {

        ImGui::PushID(entry.first);

        if(entry.first == selected_id) {
            ImGui::Text("[Object %d]", entry.first);
        } else {
            ImGui::Text("Object %d", entry.first);
        }
        
        if(ImGui::SmallButton("Select")) {
            selected_id = entry.first;
        }
        if(i++ != objs.size() - 1) ImGui::Separator();

        ImGui::PopID();
    }

    ImGui::End();
}

void App::add_object(const Scene_Object& obj) {
    objs.insert({next_id++, obj});
}

