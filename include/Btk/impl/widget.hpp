#if !defined(_BTKIMPL_WIDGET_HPP_)
#define _BTKIMPL_WIDGET_HPP_
#include <SDL2/SDL_rect.h>
#include "atomic.hpp"
namespace Btk{
    struct WindowImpl;
    //the basic widget
    struct WidgetImpl{
        inline WidgetImpl():
            parent(nullptr),
            win(nullptr),
            rect({
                0,0,0,0
            }),
            refcount(1){

        }
        virtual ~WidgetImpl();
        virtual void draw() = 0;
        //unref
        void unref(){
            --refcount;
            if(refcount == 0){
                delete this;
            }
        }
        //master of this widget
        WidgetImpl *parent;
        //widget window
        WindowImpl *win;
        //widgets postions
        SDL_Rect rect;
        //refcount
        Atomic refcount;
    };
};


#endif // _BTKIMPL_WIDGET_HPP_
