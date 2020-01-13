
#pragma once

#include <SDL2/SDL.h>

#include "../lib/math.h"
#include "../app.h"

class Platform {

public:
	Platform();
	~Platform();

	void loop(App& app);

	Vec2 window_draw();
	Vec2 window_size();
	Vec2 scale_mouse(Vec2 mouse);

	void capture_mouse();
	void release_mouse();
	void set_mouse(Vec2 pos);
	Vec2 get_mouse();
	void grab_mouse();
	void ungrab_mouse();

private:
	float dpi_scale();
	float prev_dpi = 0.0f;

	void set_dpi();
	void platform_init();
	void platform_shutdown();
	void begin_frame();
	void complete_frame();

	SDL_Window* window = nullptr;
	SDL_GLContext gl_context = nullptr;
};
