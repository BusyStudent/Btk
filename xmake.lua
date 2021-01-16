add_rules("mode.debug", "mode.release")
--add SDL require
add_requires("SDL2","SDL2_image","SDL2_ttf")
--try add extensions
add_requires("gif",{optional = true})
--add_requires("freetype2",{optional = true})

add_cxxflags("-std=c++17","-Wall","-Wextra","-fPIC")
add_includedirs("include")

if is_plat("linux") then
    -- linux has fontconfig
    add_defines("BTK_USE_FONTCONFIG")
end
if is_mode("release") then
    add_cxxflags("-march=native")
    add_cflags("-march=native")
    add_cxxflags("NDEBUG")
    add_cflags("NDEBUG")
else
    add_cxxflags("-rdynamic")
    add_cflags("-rdynamic")
end
target("btk")
    on_load(function(target)
        target:add(find_packages("SDL2","SDL2_image","SDL2_ttf"))
        target:add(find_packages("gif"))
    end)
    add_defines("BTK_USE_GFX")
    add_defines("USE_MMX")
    
    if is_plat("linux") then
        add_files("./src/platform/x11/*.cpp")
        add_links("fontconfig")
    elseif is_plat("win32") or is_plat("mingw") then
        add_files("./src/platform/win32/*.cpp")
    end
    --Add gif ext
    if has_package("gif") then
        add_files("./src/ext/gif.cpp")
    end

    add_links("SDL2","SDL2_image","SDL2_ttf")
    set_kind("shared")
    --core
    add_files("./src/impl/*.cpp")
    --basic 
    add_files("./src/*.cpp")
    --widgets
    add_files("./src/widgets/*.cpp")
    --SDL_gfx
    add_files("./src/thirdparty/*.c")
    --Themes
    add_files("./src/themes/*.cpp")
    --Platform
    add_files("./src/platform/*.cpp")
    --Threading
    add_files("./src/thread/*.cpp")
    --Utils
    add_files("./src/utils/*.cpp")
    --Msgboxs
    --add_files("./src/msgbox/*.cpp")
if is_mode("debug") then
    target("hello")
        set_kind("binary")
        add_files("./tests/hello.cpp")
        add_deps("btk")
    target("opfont")
        set_kind("binary")
        add_files("./tests/opfont.cpp")
        add_deps("btk")
    target("image")
        set_kind("binary")
        add_files("./tests/image.cpp")
        add_deps("btk")
    target("fn")
        set_kind("binary")
        add_files("./tests/fn.cpp")
        add_deps("btk")
    target("async")
        set_kind("binary")
        add_files("./tests/async.cpp")
        add_deps("btk")
    target("text")
        set_kind("binary")
        add_files("./tests/text.cpp")
        add_deps("btk")
    target("timer")
        set_kind("binary")
        add_files("./tests/timer.cpp")
        add_deps("btk")
end