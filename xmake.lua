add_rules("mode.debug", "mode.release")
--add SDL require
add_requires("libsdl")
add_requires("freetype","libsdl_image",{optional = true})
add_packages("libsdl")

is_windows = is_plat("windows") or is_plat("mingw")

if is_plat("linux") then
    --Linux X11
    add_requires("dbus-1")

    --try add extensions
    add_requires("gif",{optional = true})
    add_requires("webp",{optional = true})
    --add_requires("freetype2",{optional = true})
    add_cxxflags("-Wall","-Wextra","-fPIC")
else
    --VS UTF8
    if is_plat("windows") then
        add_cxxflags("/utf-8")
        add_cxxflags("/Zc:__cplusplus")
        add_cflags("/utf-8")
    end
end
set_languages("c++17")
add_includedirs("./include")

if is_plat("linux") then
    -- linux has fontconfig freetype2
    add_links("GL")
    add_defines("BTK_HAS_FREETYPE")
    add_defines("BTK_USE_FONTCONFIG")
    --Check modes
    if is_mode("release") then
        add_cxxflags("-fvisibility=hidden","-march=native")
        add_cflags("-fvisibility=hidden","-march=native")
    -- elseif is_mode("debug") then
    --     add_cxxflags("-fsanitize=address")
    --     add_cflags("-fsanitize=address")
    --     add_links("asan")
    --     --Debug mode or etc
    end
end

if is_mode("release") then
    add_defines("NDEBUG")
end

-- Option
option("software_renderer")
    set_default(false)
    set_showmenu(true)
    set_description("Add software renderer in DeviceList")
    set_configvar("BTK_HAVE_SOFTWARE_DEVICE",true)
option("opengl_renderer")
    set_default(true)
    set_showmenu(true)
    set_description("Add opengl renderer in DeviceList")
    set_configvar("BTK_HAVE_OPENGL_DEVICE",true)
option("stb_truetype")
    set_default(false)
    set_showmenu(true)
    set_description("Force to use stb_truetype")
    set_configvar("BTK_USE_STB_TRUETYPE",true)
option("svg_parser")
    set_default(true)
    set_showmenu(true)
    set_description("Render svg")
    set_configvar("BTK_USE_SVG_PARSER",true)
option("opengles2")
    set_default(false)
    set_showmenu(true)
    set_description("Use OpenGLES2")
    set_configvar("BTK_USE_OPENGLES2",true)
option("precompiled_header")
    set_default(false)
    set_showmenu(true)
    set_description("Use Precompiled header")
option("sdl_image")
    set_default(has_package("libsdl_image"))
    set_showmenu(true)
    set_description("Use SDL_image")
    set_configvar("BTK_HAVE_SDL_IMAGE",true)
option("stb_image")
    set_default(not has_config("sdl_image"))
    set_showmenu(true)
    set_description("Use stb_image")
    set_configvar("BTK_HAVE_STB_IMAGE",true)
option("c_interface")
    set_default(false)
    set_showmenu(true)
    set_description("Add C interface")
-- Win32 Option
option("directx_renderer")
    set_default(is_windows)
    set_showmenu(true)
    set_description("Add DirectX renderer in DeviceList")
    set_configvar("BTK_HAVE_DIRECTX_DEVICE",true)
option("wincodec")
    set_default(is_windows)
    set_showmenu(true)
    set_description("Add WIC")
    set_configvar("BTK_HAVE_WINCODEC",true)
target("btk")
    -- Import option
    add_options("precompiled_header")
    add_options("software_renderer")
    add_options("directx_renderer")
    add_options("opengl_renderer")
    add_options("stb_truetype")
    add_options("stb_image")
    add_options("sdl_image")
    add_options("svg_parser")
    add_options("opengles2")
    add_options("wincodec")
    add_options("c_interface")

    -- Add configurations files
    add_configfiles("include/Btk/detail/config.hpp.in")
    set_configdir("include/Btk/detail")

    
    if is_plat("linux") then
        add_files("./src/platform/x11/*.cpp")
        --Dbus
        add_packages("dbus-1")
        add_links("fontconfig","SDL2","X11")
    elseif is_plat("windows") or is_plat("mingw") then
        --xmake repo
        add_packages("libsdl","libsdl_image","freetype")

        add_files("./src/platform/win32/*.cpp")
        add_links("user32","shell32","advapi32","ole32","oleaut32","uuid")
        add_links("gdi32","winmm","imm32","setupapi","version","comctl32")
        --add_links("freetype","bz2","brotlidec-static","brotlicommon-static")
        --add_links("libpng16","zlib")
    end
    --Add gif ext
    if has_package("gif") then
        add_files("./src/images/gif.cpp")
    end

    -- Install copy the headers

    after_install(
        function(target)
            if is_plat("linux") then 
                os.cp("$(scriptdir)/include/*","/usr/local/include")
            end
        end
    )

    --Uninstall 

    after_uninstall(
        function(target)
            if is_plat("linux") then 
                os.rm("/usr/local/include/Btk.hpp")
                os.rm("/usr/local/include/Btk.h")
                os.rm("/usr/local/include/Btk")
            end
        end
    )

    -- Package

    -- after_package(
    --     function(target)
    --         -- os.cp("$(scriptdir)/include/Btk","$(buildir)/Btk")
    --     end
    -- )

    set_kind("shared")
    --core
    add_files("./src/detail/*.cpp")
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
    --Utils
    add_files("./src/utils/*.cpp")
    --Mixer
    add_files("./src/mixer/mixer.cpp")
    add_files("./src/mixer/raw.cpp")
    --GL
    add_files("./src/gl/*.cpp")
    --Render
    add_files("./src/render/nanovg.cpp")

    if has_config("software_renderer") then
        add_files("./src/render/render_sw.cpp")
    end

    if has_config("opengl_renderer") then
        add_files("./src/render/render_gles2.cpp")
    end
    
    if has_config("directx_renderer") then
        add_files("./src/render/render_dx11.cpp")
        add_links("dxguid")
    end

    -- --SVG
    -- if not has_config("svg_parser") then
    --     add_defines("BTK_DISABLE_SVG")
    -- end

    --Font
    add_files("./src/font/fontstash.cpp")
    if not has_config("stb_truetype") then
        --We should use freetype
        set_configvar("BTK_USE_FREETYPE",true)
        add_packages("freetype")
    else
        set_configvar("BTK_USE_STB_TRUETYPE",true)
    end
    --Image
    add_files("./src/images/adapter.cpp")
    --Check Image Library
    
    if has_package("libpng") then
        add_packages("libpng")
        add_files("./src/images/png.cpp")
    end

    if has_package("webp") then
        add_packages("webp")
        add_files("./src/images/webp.cpp")
    end

    if has_config("wincodec") then
        add_files("./src/images/wincodec.cpp")
        add_links("Windowscodecs")
    end

    --SDL_image support
    if has_config("sdl_image") then
        add_files("./src/images/sdl_image.cpp")
        add_packages("libsdl_image")
    end

    if has_config("stb_image") then
        --No SDL_image use stb_image instead
        add_files("./src/images/stb_image.cpp")
    end

    if has_config("precompiled_header") then
        add_defines("_BTK_PRECOMPILED_HEADER")
        set_pcxxheader("./src/build.hpp")
    end

    if has_config("c_interface") then 
        add_files("./src/ext/capi.cpp")
    end
if is_mode("debug") then
    target("hello")
        set_kind("binary")
        add_files("./tests/hello.cpp")
        add_deps("btk")
    target("text")
        set_kind("binary")
        add_files("./tests/text.cpp")
        add_deps("btk")
    target("calc")
        set_kind("binary")
        add_files("./tests/calc.cpp")
        add_deps("btk")
    target("pixmap")
        set_kind("binary")
        add_files("./tests/pixmap.cpp")
        add_deps("btk")
    target("sliderable")
        set_kind("binary")
        add_files("./tests/sliderable.cpp")
        add_deps("btk")
end
target("btk-rcc")
    set_kind("binary")
    add_files("./tools/btk-rcc.cpp")