#include "../build.hpp"


extern "C"{
    #define LAY_IMPLEMENTATION
    //Replace
    #define LAY_MEMSET(A,B,C) SDL_memset(A,B,C)
    #define LAY_REALLOC(B,N) SDL_realloc(B,N)
    #define LAY_FREE(B) SDL_free(B)
    #define LAY_ASSERT BTK_ASSERT
    
    #define LAY_EXPORT static
    #define LAY_FLOAT 1

    #include "../libs/layout.h"
}
#define BTK_LAYOUT_INTERNAL

#include <Btk/impl/scope.hpp>
#include <Btk/layout.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>
#include <Btk/rect.hpp>
#include <list>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdbool>


namespace Btk{
    //Init / Delete context
    Layout::Layout(){
        attr.layout = true;
        lay_init_context(context());
    }
    Layout::~Layout(){
        lay_destroy_context(context());
    }
    bool Layout::handle(Event &event){
        if(Group::handle(event)){
            return true;
        }
        if(event.type() == Event::LayoutUpdate){
            update();
            return true;
        }
        return false;
    }
}
namespace Btk{
    GridLayout::GridLayout() = default;
    GridLayout::~GridLayout() = default;

    void GridLayout::update(){
        
    }
}