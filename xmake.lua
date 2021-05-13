add_rules("mode.debug", "mode.release")
--add SDL require
if is_plat("linux") then
    add_defines("USE_MMX")
    add_requires("SDL2","SDL2_image","SDL2_ttf")
    --Linux X11
    add_requires("dbus-1")

    --try add extensions
    add_requires("gif",{optional = true})
    --add_requires("freetype2",{optional = true})
    add_cxxflags("-Wall","-Wextra","-fPIC")
else
    --VCPKG
    --add_requires("vcpkg::SDL2",{alias = "SDL2"})
    --add_requires("vcpkg::SDL2-image",{alias = "SDL2_image"})
    --add_requires("vcpkg::SDL2-ttf",{alias = "SDL2_ttf"})
    --add_requires("vcpkg::gif",{optional = true,alias = "gif"})
    --using xmake repo
    add_requires("libsdl","freetype","libsdl_image")
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
    add_cxxflags("/utf-8")
    add_cflags("/utf-8")
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
        add_cxxflags("-fvisibility=hidden")
        add_cflags("-fvisibility=hidden")
    end
end
target("btk")
    add_defines("BTK_USE_GFX")
    
    if is_plat("linux") then
        add_files("./src/platform/x11/*.cpp")
        --Dbus
        add_packages("dbus-1")
        add_links("fontconfig")
    elseif is_plat("windows") or is_plat("mingw") then
        --xmake repo
        add_packages("libsdl","libsdl_image","freetype")

        add_files("./src/platform/win32/*.cpp")
        add_links("user32","shell32","advapi32","ole32","oleaut32")
        add_links("gdi32","winmm","imm32","setupapi","version")
        --add_links("freetype","bz2","brotlidec-static","brotlicommon-static")
        --add_links("libpng16","zlib")
    end
    --Add gif ext
    if has_package("gif") then
        add_files("./src/ext/gif.cpp")
    else
        add_defines("BTK_NGIF")
    end

    add_links("SDL2","SDL2_image")
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
    --Utils
    add_files("./src/utils/*.cpp")
    --Msgboxs
    add_files("./src/msgbox/*.cpp")
    add_packages("SDL2","SDL2_image","gif")
    --Mixer
    add_files("./src/mixer/mixer.cpp")
    add_files("./src/mixer/raw.cpp")
    --GL
    add_files("./src/gl/*.cpp")
    --Render
    add_files("./src/render/render_gles2.cpp")
    add_files("./src/render/nanovg.cpp")
    --Font
    add_files("./src/font/fontstash.cpp")
    add_files("./src/font/core.cpp")
    add_files("./src/font/ft2.cpp")
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
    target("draw")
        set_kind("binary")
        add_files("./tests/draw.cpp")
        add_deps("btk")
    target("scroll")
        set_kind("binary")
        add_files("./tests/scroll.cpp")
        add_deps("btk")
end
target("btk-rcc")
    set_kind("binary")
    add_files("./tools/btk-rcc.cpp")
--Do you need CAPI
--If not,omit it
if false  then 
    target("btk_capi")
        if not has_package("gif") then
            add_defines("BTK_NGIF")
        end

        set_kind("shared")
        add_files("./src/ext/capi.cpp")
        add_deps("btk")
end