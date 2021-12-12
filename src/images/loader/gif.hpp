/* Generated by dllimport.py */
#include <Btk/impl/loadso.hpp>
#include <Btk/impl/scope.hpp>

#ifdef BTK_GIF_DYMAIC
    #define BTK_GIF_LOAD()   _lib_GIF.load()
    #define BTK_GIF_UNLOAD() _lib_GIF.unload()
#else
    #define BTK_GIF_LOAD()   
    #define BTK_GIF_UNLOAD()   
#endif

//Platform depended dymaic library path
#ifdef __WIN32
    #define BTK_GIF_LIBNAME "gif.dll"
#endif
#ifdef __linux
    #define BTK_GIF_LIBNAME "libgif.so"
#endif

#ifdef BTK_GIF_DYMAIC
//Library Elem
struct _GIFLibrary {
    BTK_DYMAIC_LIBRARY(BTK_GIF_LIBNAME);
    BTK_DYMAIC_FUNCTION(DGifOpen);
    BTK_DYMAIC_FUNCTION(GifErrorString);
    BTK_DYMAIC_FUNCTION(DGifCloseFile);
};
BTK_MAKE_DYLIB(_GIFLibrary,_lib_GIF);
//Function macro
#define DGifOpen _lib_GIF->DGifOpen
#define GifErrorString _lib_GIF->GifErrorString
#define DGifCloseFile _lib_GIF->DGifCloseFile
#endif