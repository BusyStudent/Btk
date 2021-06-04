#if !defined(_BTK_UTILS_TRAITS_HPP_)
#define _BTK_UTILS_TRAITS_HPP_
#include <cstddef>
#include <tuple>
//Typetraits
namespace Btk{
    template<class T>
    struct FunctionTraits{};
    /**
     * @brief Traits for Function
     * 
     * @tparam RetT 
     * @tparam Args 
     */
    template<class RetT,class ...Args>
    struct FunctionTraits<RetT(*)(Args...)>{
        using result_type = RetT;
        static constexpr size_t args_count = sizeof ...(Args);

        template<size_t Index>
        using arg_type = std::tuple_element_t<Index,std::tuple<Args...>>;
    };

    template<class T>
    struct MemberFunctionTraits{};
    /**
     * @brief Member function traits
     * 
     * @tparam RetT 
     * @tparam Class 
     * @tparam Args 
     */
    template<class RetT,class Class,class ...Args>
    struct MemberFunctionTraits<RetT (Class::*)(Args...)>{
        using result_type = RetT;
        using object_type = Class;
        static constexpr size_t args_count = sizeof ...(Args);
        static constexpr bool is_const = false;

        template<size_t Index>
        using arg_type = std::tuple_element_t<Index,std::tuple<Args...>>;
    };
    //for const method
    template<class RetT,class Class,class ...Args>
    struct MemberFunctionTraits<RetT (Class::*)(Args...) const>{
        using result_type = RetT;
        using object_type = Class;
        static constexpr size_t args_count = sizeof ...(Args);
        static constexpr bool is_const = true;
        
        template<size_t Index>
        using arg_type = std::tuple_element_t<Index,std::tuple<Args...>>;
    };
}


#endif // _BTK_UTILS_TRAITS_HPP_
