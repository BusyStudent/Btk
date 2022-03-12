#if !defined(_BTK_SIGNAL_BIND_HPP_)
#define _BTK_SIGNAL_BIND_HPP_

//Import std::bind
#include <type_traits>
#include <functional>

#include "base.hpp"

namespace Btk{
    //For bind(&App::method,etc...);
    template<class T>
    struct _MemFunctionBinder:_BindWithMemFunction{
        _MemFunctionBinder(Object *o,T &&b):binder(b){
            object_ptr = o;
        }
        T   binder;

        template<class ...Args>
        auto operator ()(Args &&...args){
            return binder(std::forward<Args>(args)...);
        }
    };

    template<class ...Args>
    inline auto Bind(Args &&...args){
        return std::bind(std::forward<Args>(args)...);
    }
    template<class RetT,class Class,class ...FnArgs,class ...Args>
    auto Bind(RetT (Class::*method)(FnArgs ...),Class *object,Args &&...args){

        return _MemFunctionBinder{
            object,
            std::bind(method,object,std::forward<Args>(args)...)        
        };
    }
    template<class RetT,class Class,class ...FnArgs,class ...Args>
    auto Bind(RetT (Class::*method)(FnArgs ...) const,const Class *object,Args &&...args){

        return _MemFunctionBinder{
            object,
            std::bind(method,object,std::forward<Args>(args)...)
        };
    }

}


#endif // _BTK_SIGNAL_BIND_HPP_
