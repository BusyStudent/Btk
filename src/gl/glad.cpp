#include <Btk/gl/gl.hpp>

#ifdef BTK_NEED_GLAD
#define GLAD_GLAPI_EXPORT
#define GLAD_GLAPI_EXPORT_BUILD
extern "C"{
    #include "../libs/glad.c"
}
#endif