
project('s4d', 'cpp', default_options : ['cpp_std=c++17'])

cc 	        = meson.get_compiler('cpp')
host_system = host_machine.system()

project_dir = meson.current_source_dir()
inc_dir     = include_directories('deps')

sources = [
    'deps/imgui/imgui_compile.cpp',
    'deps/glad/glad.cpp',
    'deps/misc/misc.cpp',
    'src/platform/gl.cpp',
    'src/platform/platform.cpp',
    'src/app.cpp',
    'src/gui.cpp',
    'src/undo.cpp',
    'src/scene/scene.cpp',
    'src/scene/util.cpp',
    'src/main.cpp']

link = []
deps = []
args = []
nfd_dir = join_paths(project_dir, 'deps/nfd')

if host_system == 'windows'

    assert(cc.get_id() == 'msvc', 'Use the Visual Studio compiler on Windows.')

    inc_dir = include_directories('deps', 'deps/win')
    sdl_dir = join_paths(project_dir, 'deps/win/SDL2')
    assimp_dir = join_paths(project_dir, 'deps/win/assimp')

    if get_option('buildtype') == 'debug'
        link += ['/NODEFAULTLIB:MSVCRT']
        deps += cc.find_library('assimp_d', dirs : assimp_dir)
        deps += cc.find_library('zlib_d', dirs : assimp_dir)
        deps += cc.find_library('irrXML_d', dirs : assimp_dir)
    else
        deps += cc.find_library('assimp', dirs : assimp_dir)
        deps += cc.find_library('zlib', dirs : assimp_dir)
        deps += cc.find_library('irrXML', dirs : assimp_dir)
    endif

    link += ['../src/platform/icon.res', 'Shcore.lib', 'Winmm.lib', 'Version.lib', 'Setupapi.lib']
    deps += cc.find_library('SDL2-static', dirs : sdl_dir)
    deps += cc.find_library('SDL2main', dirs : sdl_dir)
    deps += cc.find_library('nfd', dirs : nfd_dir)

elif host_system == 'linux'

    assert(cc.get_id() == 'gcc' or cc.get_id() == 'clang', 'Use clang or GCC on linux.')

    deps += dependency('sdl2')
    deps += dependency('assimp')
    deps += dependency('gtk+-3.0')
    deps += cc.find_library('nfd_l', dirs : nfd_dir)
    link += ['-ldl', '-pthread']

elif host_system == 'darwin'

    assert(cc.get_id() == 'clang', 'Use clang on mac.')

    deps += dependency('sdl2')
    # deps += dependency('assimp')
    deps += dependency('gtk+-3.0')
    deps += cc.find_library('nfd_m', dirs : nfd_dir)

else
    assert(false, 'Only windows/linux/mac supported.')
endif

executable('s4d', sources,
    dependencies : deps,
    include_directories : inc_dir, 
    link_args : link,
    cpp_args : args,
    gui_app : true)

