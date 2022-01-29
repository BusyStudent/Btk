#if !defined(_BTK_UTILS_TRAITS_HPP_)
#define _BTK_UTILS_TRAITS_HPP_
#include <cstddef>
//Typetraits
namespace Btk{

    template<size_t Index,class T,class ...Args>
    struct _IndexArgsPackImpl{
        using type = typename _IndexArgsPackImpl<Index - 1,Args...>::type;
    };
    template<class T,class ...Args>
    struct _IndexArgsPackImpl<0,T,Args...>{
        using type = T;
    };
    /**
     * @brief Get Elem from type args package
     * 
     * @tparam Index 
     * @tparam Args 
     */
    template<size_t Index,class ...Args>
    struct IndexArgsPack{
        static_assert(Index < sizeof ...(Args),"Out of range");
        using type = typename _IndexArgsPackImpl<Index,Args...>::type;
    };
    // /**
    //  * @brief Helper for Unpack 
    //  * 
    //  */
    // struct _TieValuePackHelper{
    //     template<size_t Index,class ...Args>
    //     static decltype(auto) Get(Args &&...args){
    //         return Get<Index - 1>(std::forward<Args>(args)...);
    //     }
    //     template<class T,class ...Args>
    //     static T Get< 0,T,Args...>(){

    //     }
    // };

    // template<size_t Index,class ...Args>
    // decltype(auto) TieValuePack(Args &&...args){
    //     return TieValuePack<Index - 1,Args...>(std::forward<Args>(args)...);
    // }
    // template<class T,class ...Args>
    // T TieValuePack<0,T,Args...>(T &&value,Args &&...){
    //     return value;
    // }


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
        using arg_type = typename IndexArgsPack<Index,Args...>::type;
    };
    /**
     * @brief Get info of a function
     * 
     * @tparam RetT 
     * @tparam Args 
     */
    template<class RetT,class ...Args>
    struct FunctionTraits<RetT(Args...)>{
        using result_type = RetT;
        static constexpr size_t args_count = sizeof ...(Args);

        template<size_t Index>
        using arg_type = typename IndexArgsPack<Index,Args...>::type;
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
        using class_type  = Class;
        static constexpr size_t args_count = sizeof ...(Args);
        static constexpr bool is_const = false;

        template<size_t Index>
        using arg_type = typename IndexArgsPack<Index,Args...>::type;
    };
    //for const method
    template<class RetT,class Class,class ...Args>
    struct MemberFunctionTraits<RetT (Class::*)(Args...) const>{
        using result_type = RetT;
        using object_type = const Class;
        using class_type  = Class;
        static constexpr size_t args_count = sizeof ...(Args);
        static constexpr bool is_const = true;
        
        template<size_t Index>
        using arg_type = typename IndexArgsPack<Index,Args...>::type;
    };
    //C++17 void_t
    template<class ...Args>
    using void_t = void;
}


#endif // _BTK_UTILS_TRAITS_HPP_
