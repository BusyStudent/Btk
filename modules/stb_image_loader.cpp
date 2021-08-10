#include <Btk/impl/codec.hpp>
#include <Btk/module.hpp>
#include <Btk/string.hpp>

//STBIMAGE Dymaic modules
#define BTK_HAS_STBII
#include "../src/images/stb_image.cpp"

BTK_MODULE_INIT(module_info){
    module_info.name = "stbimage_loader";
    Btk::RegisterSTBII();
}
