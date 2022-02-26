add_rules("mode.debug", "mode.release")
--add SDL require
if is_plat("linux") then
    add_requires("SDL2")
    --Linux X11
    add_requires("dbus-1")

    --try add extensions
    add_requires("gif",{optional = true})
    add_requires("webp",{optional = true})
    add_requires("SDL2_image",{optional = true})
    --add_requires("freetype2",{optional = true})
    add_cxxflags("-Wall","-Wextra","-fPIC")
else
    --VCPKG
    --add_requires("vcpkg::SDL2",{alias = "SDL2"})
    --add_requires("vcpkg::SDL2-image",{alias = "SDL2_image"})
    --add_requires("vcpkg::SDL2-ttf",{alias = "SDL2_ttf"})
    --add_requires("vcpkg::gif",{optional = true,alias = "gif"})
    --using xmake repo
    add_requires("libsdl")
    add_requires("freetype","libsdl_image",{optional = true})
    add_packages("libsdl","freetype","libsdl_image")

    --add_packages("SDL2","SDL2-image","SDL2-ttf")
    --add_includedirs("E:/VisualStudio/VCPKG/vcpkg-master/installed/x86-windows/include")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2-image_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2-ttf_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/freetype_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/libpng_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/zlib_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/bzip2_x64-windows-static/lib")
    --add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/brotli_x64-windows-static/lib")
    --add_requires("SDL2","SDL2_ttf","SDL2_image")
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
    add_requires("freetype2")
    add_packages("freetype2")
    add_links("freetype")
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
option("opengl_renderer")
    set_default(true)
    set_showmenu(true)
    set_description("Add opengl renderer in DeviceList")
option("stb_truetype")
    set_default(false)
    set_showmenu(true)
    set_description("Force to use stb_truetype")
option("svg_parser")
    set_default(true)
    set_showmenu(true)
    set_description("Render svg")
option("opengles2")
    set_default(false)
    set_showmenu(true)
    set_description("Use OpenGLES2")
option("precompiled_header")
    set_default(false)
    set_showmenu(true)
    set_description("Use Precompiled header")
-- Win32 Option
option("directx_renderer")
    if is_plat("windows") or is_plat("mingw") then
        set_default(true)
    else
        set_default(false)
    end
    set_showmenu(true)
    set_description("Add DirectX renderer in DeviceList")
option("wincodec")
    if is_plat("windows") or is_plat("mingw") then
        set_default(true)
    else
        set_default(false)
    end
    set_showmenu(true)
    set_description("Add WIC")

target("btk")

    -- Import option
    add_options("software_renderer")
    add_options("directx_renderer")
    add_options("opengl_renderer")
    add_options("stb_truetype")

    
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
        add_defines("BTK_HAS_GIF")
    else
        add_defines("BTK_NGIF")
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
        add_defines("BTK_USE_SWDEVICE")
    end

    if has_config("opengl_renderer") then
        if has_config("opengles2") then
            add_defines("BTK_USE_GLES2")
        end
        add_files("./src/render/render_gles2.cpp")
    else
        add_defines("BTK_NO_GLDEVICE")
    end
    
    if has_config("directx_renderer") then
        add_files("./src/render/render_dx11.cpp")
        add_defines("BTK_USE_DXDEVICE")
    end

    --SVG
    if not has_config("svg_parser") then
        add_defines("BTK_DISABLE_SVG")
    end

    --Font
    add_files("./src/font/fontstash.cpp")
    if has_config("stb_truetype") then
        add_defines("BTK_USE_STBTT")
    end
    -- add_files("./src/font/cache.cpp")
    -- add_files("./src/font/ft_font.cpp")
    --Image
    add_files("./src/images/adapter.cpp")
    --Check Image Library
    
    if has_package("libpng") then
        add_defines("BTK_HAS_PNG")
        add_packages("libpng")
        add_files("./src/images/png.cpp")
    end

    if has_package("webp") then
        add_defines("BTK_HAS_WEBP")
        add_packages("webp")
        add_files("./src/images/webp.cpp")
    end

    if has_config("wincodec") then
        add_defines("BTK_HAS_WIC")
        add_files("./src/images/wincodec.cpp")
        add_links("Windowscodecs")
    end

    --SDL_image support
    if has_package("libsdl_image") or has_package("SDL2_image") then
        add_defines("BTK_HAS_SDLIMG")
        add_files("./src/images/sdl_image.cpp")
        add_links("SDL2_image")
    else
        --No SDL_image use stb_image instead
        add_defines("BTK_HAS_STBII")
        add_files("./src/images/stb_image.cpp")
    end

    if has_config("precompiled_header") then
        add_defines("_BTK_PRECOMPILED_HEADER")
        set_pcxxheader("./src/build.hpp")
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
end
target("btk-rcc")
    set_kind("binary")
    add_files("./tools/btk-rcc.cpp")
--Do you need CAPI
--If not,omit it
if true and not is_plat("windows")then 
    target("btk_capi")
        if not has_package("gif") then
            add_defines("BTK_NGIF")
        end

        set_kind("shared")
        add_files("./src/ext/capi.cpp")
        add_deps("btk")
end