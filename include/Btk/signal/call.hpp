#if !defined(_BTK_SIGNAL_CALL_HPP_)
#define _BTK_SIGNAL_CALL_HPP_
#include "../utils/traits.hpp"
#include <type_traits>
#include <utility>

namespace Btk{
    //For call
    template<class Callable,class ...Args>
    auto _Call(Callable &&callable,Args &&...args) -> 
        std::invoke_result_t<Callable,Args...>{
        
        return callable(std::forward<Args>(args)...);
    }

    template<class RetT,class Class,class ...FnArgs,class ...Args>
    auto _Call(RetT (Class::*method)(FnArgs ...),Class *object,Args &&...args){

        return (object->*method)(std::forward<Args>(args)...);
    }
    template<class RetT,class Class,class ...FnArgs,class ...Args>
    auto _Call(RetT (Class::*method)(FnArgs ...) const,const Class *object,Args &&...args){

        return (object->*method)(std::forward<Args>(args)...);
    }

}


#endif // _BTK_SIGNAL_CALL_HPP_
