#if !defined(_BTK_FACTORY_HPP_)
#define _BTK_FACTORY_HPP_
#include <typeinfo>
#include "widget.hpp"

namespace Btk{
    class AbstractButton;
    class WidgetFactory{
        virtual ~WidgetFactory(){}
        virtual AbstractButton *create_button() = 0;
        virtual AbstractButton *create_check_button() = 0;
        virtual AbstractButton *create_radio_button() = 0;
        virtual Widget *        create(const std::type_info &info) = 0;
        virtual bool            has(const std::type_info &info) = 0;

        template<class T>
        auto create(){
            return create(typeid(T));
        }
        template<class T>
        bool has(){
            return has(typeid(T));
        }
    };
}


#endif // _BTK_FACTORY_HPP_
