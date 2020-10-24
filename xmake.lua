add_rules("mode.debug", "mode.release")
--add SDL require
add_requires("SDL2","SDL2_image","SDL2_ttf")

add_cxxflags("-std=c++17","-Wall","-Wextra")
add_includedirs("include")

if is_plat("linux") then
    -- linux has fontconfig
    add_defines("BTK_USE_FONTCONFIG")
end

target("btk")
    on_load(function(target)
        target:add(find_packages("SDL2","SDL2_image","SDL2_ttf","SDL2_gfx","fontconfig"))
    end)
    add_defines("BTK_USE_GFX")
    add_defines("USE_MMX")
    if is_plat("linux") then
        add_links("fontconfig")
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
    
target("hello")
    set_kind("binary")
    add_files("./tests/hello.cpp")
    add_deps("btk")
target("opfont")
    set_kind("binary")
    add_files("./tests/opfont.cpp")
    add_deps("btk")
target("fn")
    set_kind("binary")
    add_files("./tests/fn.cpp")
    add_deps("btk")