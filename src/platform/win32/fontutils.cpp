#include <windows.h>

#include "../../build.hpp"

#include <Btk/font.hpp>
namespace Btk{
namespace FontUtils{
    void Init(){

    };
    void Quit(){

    };
    std::string GetFileByName(std::string_view name){
        #ifdef 1
        //We doesn't impl it yet
        return "C:/Windows/Fonts/simsun.ttc";
        #endif
    };
};
};