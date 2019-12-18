
project('s4d', 'cpp', default_options : ['cpp_std=c++17'])

cc 			= meson.get_compiler('cpp')
host_system = host_machine.system()

project_dir = meson.current_source_dir()
inc_dir 	= include_directories('deps')

sources = [
	'deps/imgui/imgui.cpp',
	'deps/imgui/imgui_draw.cpp',
	'deps/imgui/imgui_widgets.cpp',
	'deps/imgui/imgui_impl_sdl.cpp',
	'deps/imgui/imgui_impl_opengl3.cpp',
	'deps/glad/glad.cpp',
	'src/lib/lib.cpp',
	'src/lib/gl.cpp',
	'src/lib/camera.cpp',
	'src/platform.cpp',
	'src/app.cpp',
	'src/scene/scene.cpp',
	'src/scene/scene_object.cpp',
	'src/scene/proc_objects.cpp',
	'src/main.cpp']

link = []
args = []
deps = []

if host_system == 'windows'
	
	link += '../src/icon.res'
	args += ['-fp:fast', '-wd4201', '-wd4702']

	inc_dir = include_directories('deps', 'deps/win')
	sdl_dir = join_paths(project_dir, 'deps/win/SDL2/lib')
	deps += cc.find_library('SDL2', dirs : sdl_dir)
	deps += cc.find_library('SDL2main', dirs : sdl_dir)
	configure_file(input: 'deps/win/SDL2/lib/SDL2.dll', output: 'SDL2.dll', copy : true)

elif host_system == 'linux'

	deps += dependency('sdl2')
	link += ['-ldl', '-pthread']
	args += ['-ffast-math']

endif

executable('s4d', sources,
	dependencies : deps,
	include_directories : inc_dir, 
	cpp_args : args,
	link_args : link,
	gui_app : true)

