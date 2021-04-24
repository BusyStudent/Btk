#include "../build.hpp"

extern "C"{
    #define FONS_USE_FREETYPE
    #define NVG_NO_STB
    #include "../libs/nanovg.h"
    #include "../libs/nanovg.c"
}