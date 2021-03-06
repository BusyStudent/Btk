//nanovg files
#include <Btk/gl/glad.h>
#define NANOVG_GLES2_IMPLEMENTATION
#define NVG_NO_STB

#ifdef BTK_HAS_FREETYPE
    #define FONS_USE_FREETYPE
#endif

#include "../libs/nanovg.c"