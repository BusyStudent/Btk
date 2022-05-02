#include "../build.hpp"

#include <Btk/detail/thread.hpp>
#include <Btk/themes.hpp>
#include <Btk/Btk.hpp>
#include <atomic>
#include <mutex>

#include "builtins.inl"

namespace Btk{
    Theme::Theme(){

    }
    Theme::Theme(const Theme &) = default;
    Theme::~Theme() = default;

    static bool has_theme_inited = false;
    static Constructable<RefPtr<Theme>> global_theme;

    static void theme_cleanup(){
        global_theme.destroy();
    }

    RefPtr<Theme> CurrentTheme(){
        if(not has_theme_inited){
            global_theme.construct(new Theme);
            construct_theme_default(**global_theme);
            has_theme_inited = true;
            AtExit(theme_cleanup);
        }
        return *global_theme;
    }
}