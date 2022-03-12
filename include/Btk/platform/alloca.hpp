#if !defined(_BTK_PLATFORM_ALLOCA_HPP_)
#define _BTK_PLATFORM_ALLOCA_HPP_

//Provide StackAlloc
//Platfrom infomation
#include <cstdlib>
#include <cstdint>
#include <climits>
#include "../string.hpp"


#if BTK_GCC
    #define Btk_StackAlloc(SIZE) __builtin_alloca(SIZE)
    #define Btk_StackFree(PTR) /*Donothing*/
#elif BTK_WIN32
    #include <malloc.h>
    #define Btk_StackAlloc(SIZE) _alloca(SIZE)
    #define Btk_StackFree(PTR) /*Donothing*/
#elif BTK_LINUX
    #include <alloca.h>
    #define Btk_StackAlloc(SIZE) alloca(SIZE)
    #define Btk_StackFree(PTR) /*Donothing*/
#else
    #define BTK_NO_SYSALLOCA
    //use malloc / free
    #define Btk_StackAlloc(SIZE) std::malloc(SIZE)
    #define Btk_StackFree(PTR) std::free(PTR)
#endif

/**
 * @brief Is size > it,it will be allocate in heap
 * 
 */
#ifndef BTK_SMALL_STACK_SIZE
    #define BTK_SMALL_STACK_SIZE 128
#endif
#ifndef BTK_SMALL_SIZE_T
    #define BTK_SMALL_SIZE_T ::size_t
#endif

#if BTK_WIN32
    #define Btk_SmallAlloc(SIZE) _malloca(SIZE)
    #define Btk_SmallFree(PTR)  _freea(PTR)
#elif defined(BTK_NO_SYSALLOCA)
    //system didnot provide alloca
    //So just use malloc / free
    #define Btk_SmallAlloc(SIZE) std::malloc(SIZE)
    #define Btk_SmallFree(PTR) std::free(PTR)
#else
    inline void _Btk_SmallFree(void *ptr) noexcept{
        if(ptr == nullptr){
            return;
        }
        using size_t = BTK_SMALL_SIZE_T;
        size_t *p = reinterpret_cast<size_t*>(
            static_cast<uint8_t*>(ptr) - sizeof(size_t)
        );
        if(*p > BTK_SMALL_STACK_SIZE){
            //In heap free it
            std::free(p);
        }
        else{
            Btk_StackFree(p);
        }
    }
    inline void *_Btk_SmallAllocHelper(void *ptr,BTK_SMALL_SIZE_T n) noexcept{
        if(ptr == nullptr){
            return nullptr;
        }
        using size_t = BTK_SMALL_SIZE_T;

        *static_cast<size_t*>(ptr) = n;

        return static_cast<uint8_t*>(ptr) + sizeof(size_t);
    }
    #if BTK_GCC
        //GCC ext
        #define Btk_SmallAlloc(SIZE) \
        ({\
            BTK_SMALL_SIZE_T n = SIZE;\
            void *ptr;\
            if(n > BTK_SMALL_STACK_SIZE){\
                ptr = std::malloc(n + sizeof(BTK_SMALL_SIZE_T));\
            }\
            else{\
                ptr = Btk_StackAlloc(n + sizeof(BTK_SMALL_SIZE_T));\
            }\
            _Btk_SmallAllocHelper(ptr,n);\
        })
    #else
        #define Btk_SmallAlloc(SIZE) \
            (_Btk_SmallAllocHelper( \
                SIZE > BTK_SMALL_STACK_SIZE \ 
                ? std::malloc(SIZE + sizeof(BTK_SMALL_SIZE_T)) \
                : Btk_StackAlloc(SIZE+ sizeof(BTK_SMALL_SIZE_T)) \ 
                , SIZE\
            ))
    #endif
    #define Btk_SmallFree(PTR) _Btk_SmallFree(PTR)
#endif

#define Btk_SmallCalloc(TYPE,N) static_cast<TYPE*>(Btk_SmallAlloc(sizeof(TYPE) * N))


template<class T>
inline T *_Btk_SmallStrndup(T *dst,const T *src,size_t n) noexcept{
    dst[n] = '\0';
    return static_cast<T*>(memcpy(dst,src,n * sizeof(T)));
}

template<class T>
struct _Btk_TypeofString;

template<class T>
struct _Btk_TypeofString<T*>{
    using type = T;
};
template<class T>
struct _Btk_TypeofString<const T*>{
    using type = T;
};
template<class T,size_t N>
struct _Btk_TypeofString<const T (&)[N]>{
    using type = T;
};

#if BTK_GCC && BTK_WIN32
    #define Btk_SmallStrndupTyped(T,STR,N) ({\
        BTK_SMALL_SIZE_T n = N;\
        BTK_SMALL_SIZE_T alloc_size = (n + 1) * sizeof(T);\
        void *ptr = _malloca(alloc_size);
        _Btk_SmallStrndup(static_cast<T*>(ptr),STR,n);\
    })
#elif BTK_GCC
    #define Btk_SmallStrndupTyped(T,STR,N) ({\
        BTK_SMALL_SIZE_T n = N;\
        BTK_SMALL_SIZE_T alloc_size = (n + 1) * sizeof(T);\
        void *ptr;\
        if(n > BTK_SMALL_STACK_SIZE){\
            ptr = std::malloc(alloc_size + sizeof(BTK_SMALL_SIZE_T));\
        }\
        else{\
            ptr = Btk_StackAlloc(alloc_size + sizeof(BTK_SMALL_SIZE_T));\
        }\
        ptr = _Btk_SmallAllocHelper(ptr,alloc_size);\
        _Btk_SmallStrndup(static_cast<T*>(ptr),STR,n);\
    })
#else
    #define Btk_SmallStrndupTyped(T,STR,N) _Btk_SmallStrndup(static_cast<T*>(Btk_SmallAlloc((N + 1) * sizeof(T))),STR,N)
#endif

#define Btk_SmallStrndup(STR,N) Btk_SmallStrndupTyped(typename _Btk_TypeofString<decltype(STR)>::type,STR,N);
#define Btk_SmallStrdup(STR) Btk_SmallStrndup(STR,Btk::strlen(STR))

#endif // _BTK_PLATFORM_ALLOCA_HPP_
