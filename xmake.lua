add_rules("mode.debug", "mode.release")
--add SDL require
if not is_plat("windows") then
    add_defines("USE_MMX")
    add_requires("SDL2","SDL2_image","SDL2_ttf")
    --try add extensions
    add_requires("gif",{optional = true})
    --add_requires("freetype2",{optional = true})
    add_cxxflags("-std=c++17","-Wall","-Wextra","-fPIC")
else
    --VCPKG
    add_requires("vcpkg::SDL2",{alias = "SDL2"})
    add_requires("vcpkg::SDL2-image",{alias = "SDL2_image"})
    add_requires("vcpkg::SDL2-ttf",{alias = "SDL2_ttf"})
    add_requires("vcpkg::gif",{optional = true,alias = "gif"})
    
    add_includedirs("E:/VisualStudio/VCPKG/vcpkg-master/installed/x86-windows/include")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2-image_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/sdl2-ttf_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/freetype_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/libpng_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/zlib_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/bzip2_x64-windows-static/lib")
    add_linkdirs("E:/VisualStudio/VCPKG/vcpkg-master/packages/brotli_x64-windows-static/lib")
    add_cxxflags("/std:c++latest")
end

add_includedirs("./include")

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
    add_defines("BTK_USE_GFX")
    
    if is_plat("linux") then
        add_files("./src/platform/x11/*.cpp")
        add_links("fontconfig")
    elseif is_plat("windows") or is_plat("mingw") then
        add_files("./src/platform/win32/*.cpp")
        add_links("user32","shell32","advapi32","ole32","oleaut32")
        add_links("gdi32","winmm","imm32","setupapi","version")
        add_links("freetype","bz2","brotlidec-static","brotlicommon-static")
        add_links("libpng16","zlib")
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
    add_packages("SDL2","SDL2_ttf","SDL2_image","gif")
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