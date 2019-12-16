
#include "platform.h"
#include "lib/log.h"
#include "lib/gl.h"

#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

Platform::Platform() {
	platform_init();
}

Platform::~Platform() {
	platform_shutdown();
}

void Platform::platform_init() {

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		die("Failed to initialize SDL: %s", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("s4d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(!window) {
		die("Failed to create window: %s", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	gl_context = SDL_GL_CreateContext(window);
	if(!gl_context) {
		warn("Failed to create OpenGL 4.3 context. Trying OpenGL 3.3...");

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		gl_context = SDL_GL_CreateContext(window);

		if(!gl_context) {
			die("Failed to create OpenGL 3.3 context: %s", SDL_GetError());
		}
	} else {
		has_gl_43 = true;
	}

	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	if(!gladLoadGL()) {
		die("Failed to load OpenGL functions.");
	}

	if(has_gl_43) {
		GL::setup_debug_proc();
	}

	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 0.0f;
}

void Platform::platform_shutdown() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	GL::check_leaked_handles();
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	window = nullptr;
	gl_context = nullptr;
	SDL_Quit();	
}

void Platform::complete_frame() {

	GL::Framebuffer::bind_screen();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

void Platform::begin_frame() {

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}

void Platform::loop(App& app) {

	bool running = true;
	while(running) {
	
		begin_frame();
		ImGuiIO& io = ImGui::GetIO();

		SDL_Event e;
		while(SDL_PollEvent(&e)) {

			ImGui_ImplSDL2_ProcessEvent(&e);
			if(io.WantCaptureMouse && (e.type == SDL_MOUSEWHEEL || e.type == SDL_MOUSEBUTTONDOWN)) 
				continue;
			if(io.WantCaptureKeyboard && (e.type == SDL_TEXTINPUT || e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)) 
				continue;

			switch(e.type) {
			case SDL_QUIT: {
				running = false;
			} break;
			case SDL_KEYDOWN: {
				if(e.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
			} break;
			}

			app.event(e);
		}

		app.render();

		complete_frame();
	}
}

Vec2 Platform::window_dim() {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return Vec2((float)w, (float)h);
}

void Platform::capture_mouse() {
	SDL_CaptureMouse(SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Platform::release_mouse() {
	SDL_CaptureMouse(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Platform::set_mouse(Vec2 pos) {
	SDL_WarpMouseInWindow(window, (int)pos.x, (int)pos.y);
}
