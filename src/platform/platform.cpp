
#include "../lib/log.h"

#include "gl.h"
#include "platform.h"
#include "font.h"

#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

#ifdef _WIN32
#include <ShellScalingApi.h>
extern "C" {
	__declspec(dllexport) bool NvOptimusEnablement = true;
	__declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}
#endif

Platform::Platform() {
	platform_init();
}

Platform::~Platform() {
	platform_shutdown();
}

float Platform::dpi_scale() {

	const float sys =
#ifdef __APPLE__
		72.0f;
#else
		96.0f;
#endif

	float hdpi;
	int index = SDL_GetWindowDisplayIndex(window);
	if(index < 0) {
		warn("Failed to get window display index: %s", SDL_GetError());
		return 1.0f;
	}
	if(SDL_GetDisplayDPI(index, nullptr, &hdpi, nullptr)) {
		warn("Failed to get display DPI: %s", SDL_GetError());
		return 1.0f;
	}
	
	return hdpi / sys;
}

void Platform::platform_init() {

#ifdef _WIN32
	if(SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) != S_OK) 
		warn("Failed to set process DPI aware.");
#endif

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		die("Failed to initialize SDL: %s", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	Vec2 wsize = Vec2(900, 600);

	window = SDL_CreateWindow("s4d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)wsize.x, (int)wsize.y, 
							  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if(!window) {
		die("Failed to create window: %s", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	gl_context = SDL_GL_CreateContext(window);
	if(!gl_context) {
		die("Failed to create OpenGL 4.5 context: %s", SDL_GetError());
	}

	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	if(!gladLoadGL()) {
		die("Failed to load OpenGL functions.");
	}

	GL::setup();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init();
	
	set_dpi();
}

void Platform::set_dpi() {
	float scale = dpi_scale();
	if(prev_dpi == scale) return;
	if(prev_dpi == 0.0f) prev_dpi = 1.0f;
	
	Vec2 size = window_dim();
	size *= scale / prev_dpi;
	SDL_SetWindowSize(window, (int)size.x, (int)size.y);

	ImGuiStyle style;
	style.WindowRounding = 0.0f;
	style.ScaleAllSizes(scale);
	ImGui::GetStyle() = style;
	
	ImGuiIO& IO = ImGui::GetIO();
	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;
	IO.Fonts->Clear();
	IO.Fonts->AddFontFromMemoryTTF(font_ttf, font_ttf_len, 16.0f * scale, &config);
	IO.Fonts->Build();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	
	prev_dpi = scale;
}

void Platform::platform_shutdown() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	GL::shutdown();
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

		SDL_Event e;
		while(SDL_PollEvent(&e)) {

			ImGui_ImplSDL2_ProcessEvent(&e);

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
		set_dpi();
	}
}

Vec2 Platform::window_dim() {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return Vec2((float)w, (float)h);
}

void Platform::grab_mouse() {
	SDL_SetWindowGrab(window, SDL_TRUE);
}

void Platform::ungrab_mouse() {
	SDL_SetWindowGrab(window, SDL_FALSE);
}

Vec2 Platform::get_mouse() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	return Vec2(x, y);
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
