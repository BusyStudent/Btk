#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include <map>
#include "rect.hpp"
#include "widget.hpp"
#include "container.hpp"


namespace Btk{
    class Layout:public Group{
        public:
            Layout();
            ~Layout();

            //update each widgets postions
            virtual void update() = 0;
        private:
            struct [[maybe_unused]] _InternalContext{
                void *_hidden1;
                void *_hidden2;
                Uint32 _hidden3;
                Uint32 _hidden4;
            };

            #ifdef BTK_LAYOUT_INTERNAL
            //Internal defs
            typedef lay_context context_t;

            static_assert(
                sizeof(lay_context) == sizeof(_InternalContext),
                "Wrong def"
            );
            #else
            typedef _InternalContext context_t;
            #endif

            context_t _lay_ctxt;
        protected:
            context_t *context() noexcept{
                return &_lay_ctxt;
            }
    };
    class BTKAPI BoxLayout:public Layout{

    };
    class BTKAPI VBoxLayout:public BoxLayout{

    };
    class BTKAPI HBoxLayout:public BoxLayout{

    };
    class BTKAPI GridLayout:public Layout{
        public:
            GridLayout();
            GridLayout(const GridLayout &) = delete;
            ~GridLayout();

            void update();
            bool add_row(Widget *widget);
            void add();
        private:
            
    };
};


#endif // _BTK_LAYOUT_HPP_
