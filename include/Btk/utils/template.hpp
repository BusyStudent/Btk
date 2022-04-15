#if !defined(_BTK_UTILS_TEMPLATE_HPP_)
#define _BTK_UTILS_TEMPLATE_HPP_
#include <memory>
#include <utility>
#include <cstddef>
#include "../defs.hpp"
#include "traits.hpp"


#define BTK_HAS_MEMBER_IMPL(RESULT,TYPE,MEMBER,ID) \
    template<class Type,class T = Btk::void_t<Type::MEMBER>> \
    struct ID{ static constexpr bool value = true}; \
    template<class Type,class T = Btk::void_t<>> \
    struct ID{ static constexpr bool value = false}; \
    \
    static constexpr bool RESULT = ID<TYPE>::value;
#define BTK_HAS_MEMBER(RESULT,TYPE,MEMBER) \
    BTK_HAS_MEMBER_IMPL(RESULT,TYPE,MEMBER,BTK_UNIQUE_NAME(_MemberDetect))

/**
 * @brief It contains some useful template
 * 
 */
namespace Btk{
    struct adopt_lock_t{};
    inline constexpr auto adopt_lock = adopt_lock_t{};
    /**
     * @brief Lock Guard
     * 
     * @tparam T 
     */
    template<class T>
    class LockGuard{
        public:
            LockGuard(T &p):ptr(p){
                p.lock();
            }
            LockGuard(T &p,adopt_lock_t):ptr(p){}
            LockGuard(const LockGuard &) = delete;
            ~LockGuard(){
                ptr.unlock();
            }
        private:
            T &ptr;
    };
    template<class T>
    using lock_guard = LockGuard<T>;

    template<class T,class Deleter = std::default_delete<T>>
    using unique_ptr = std::unique_ptr<T,Deleter>;
    /**
     * @brief Base for RefPtr
     * 
     */
    class RefPtrBase{
        protected:

        bool has_refcount() const noexcept{
            return refcount != nullptr;
        }
        /**
         * @brief Get the refcount object
         * 
         * @return int 
         */
        int  get_refcount() const noexcept{
            if(has_refcount()){
                return *refcount;
            }
            return -1;
        }
        /**
         * @brief add ref
         * 
         */
        void ref() const noexcept{
            if(not has_refcount()){
                refcount = new int(1);
            }
            else{
                ++(*refcount);
            }
        }
        /**
         * @brief Ref the base
         * 
         * @param base 
         */
        void ref(const RefPtrBase &base){
            if(base.has_refcount()){
                refcount = base.refcount;
                ++(*refcount);
            }
            else{
                refcount = nullptr;
            }
        }
        /**
         * @brief de ref
         * 
         * @return true on We should delete the object
         * @return false 
         */
        bool unref() const noexcept{
            if(has_refcount()){
                --(*refcount);
                //We should cleanup
                if(*refcount == 0){
                    delete refcount;
                    refcount = nullptr;
                    return true;
                }
                refcount = nullptr;
            }
            return false;
        }
        mutable int *refcount = nullptr;
    };
    /**
     * @brief Ref Pointer
     * 
     * @tparam T 
     */
    template<class T>
    class RefPtr:protected RefPtrBase{
        public:
            explicit RefPtr(T *);
            RefPtr() = default;
            RefPtr(const RefPtr &);
            ~RefPtr(){
                reset();
            }
            /**
             * @brief Get the data
             * 
             * @return T* 
             */
            T *get() const noexcept{
                return data;
            }
            /**
             * @brief Get refcount
             * 
             * @return int 
             */
            int refcount() const noexcept{
                return this->get_refcount();
            }
            void reset(){
                //True on cleanup
                if(this->unref()){
                    delete data;
                }
            }
            T *operator ->() const noexcept{
                return get();
            }
            T &operator *() const noexcept{
                return *get();
            }
            bool empty() const noexcept{
                return data == nullptr;
            }

            RefPtr &operator =(const RefPtr &);
            RefPtr &operator =(std::nullptr_t);
            RefPtr &operator =(T *);
        private:
            T *data = nullptr;
    };
    template<class T>
    inline RefPtr<T>::RefPtr(const RefPtr &p){
        this->ref(p);
        data = p.data;
    }
    template<class T>
    inline RefPtr<T>::RefPtr(T *p){
        reset();
        data = p;
        if(p != nullptr){
            this->ref();
        }
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(T *ptr){
        reset();
        data = ptr;
        if(data != nullptr){
            //Create a new refcount when it is not nullptr
            this->ref();
        }
        return *this;
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(std::nullptr_t){
        reset();
        return *this;
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(const RefPtr<T> &ptr){
        if(this == &ptr){
            return *this;
            
        }
        reset();
        this->ref(ptr);
        data = ptr.data;
        return *this;
    }
    /**
     * @brief Weak Pointer
     * 
     * @tparam T 
     */
    template<class T>
    class WeakPtr{

    };
    /**
     * @brief Holder for Object to erase the destructor
     * 
     * @tparam T 
     */
    template<class T>
    class Constructable{
        public:
            Constructable() = default;
            ~Constructable() = default;
            /**
             * @brief Construct the object
             * 
             * @tparam Args 
             * @param args 
             */
            template<class ...Args>
            void construct(Args &&...args){
                new(reinterpret_cast<void*>(buffer)) T(
                    std::forward<Args>(args)...
                );
            }
            /**
             * @brief Destruct the object
             * 
             */
            void destroy(){
                if constexpr(std::is_destructible_v<T>){
                    get()->~T();
                }
            }
            /**
             * @brief Make the memory in 0
             * 
             */
            void zero(){
                for(size_t n = 0;n < sizeof(T);++n){
                    buffer[n] = 0;
                }
            }
            //Get 
            T *get() noexcept{
                return reinterpret_cast<T*>(buffer);
            }
            const T *get() const noexcept{
                return reinterpret_cast<const T*>(buffer);
            }
            T *operator &() noexcept{
                return get();
            }
            T *operator ->() noexcept{
                return get();
            }
            T &operator *() noexcept{
                return *get();
            }
            const T *operator &() const noexcept{
                return get();
            }
            const T *operator ->() const noexcept{
                return get();
            }
            const T &operator *() const noexcept{
                return *get();
            }
        private:
            alignas(T) Uint8 buffer[sizeof(T)];
    };

    struct nullopt_t{};
    inline constexpr auto nullopt = nullopt_t{};
    /**
     * @brief Optional value
     * 
     * @tparam T 
     */
    template<class T>
    class Optional{
        public:
            Optional() = default;
            Optional(const T &);
            Optional(T &&);
            Optional(nullopt_t):Optional(){}
            Optional(const Optional &);
            ~Optional(){
                reset();
            }

            bool has_value() const noexcept{
                return _has_value;
            }

            void reset(){
                if(has_value()){
                    buffer.destroy();
                    _has_value = false;
                }
            }
            template<class ...Args>
            void emplace(Args &&...args){
                reset();
                buffer.construct(std::forward<Args>(args)...);
                _has_value = true;
            }
            
            operator bool() const noexcept{
                return _has_value;
            }
            T *operator ->() noexcept{
                return buffer.get();
            }
            T &operator &() noexcept{
                return *buffer.get();
            }
            const T *operator ->() const noexcept{
                return buffer.get();
            }
            const T &operator &() const noexcept{
                return *buffer.get();
            }
        private:
            //< buffer contain the value
            Constructable<T> buffer;
            bool _has_value = false;
    };
    template<class T>
    inline Optional<T>::Optional(const T &v){
        buffer.construct(std::forward<T>(v));
        _has_value = true;
    }
    template<class T>
    inline Optional<T>::Optional(T &&v){
        buffer.construct(std::forward<T>(v));
        _has_value = true;
    }

    template<class T>
    using shared_ptr = RefPtr<T>;

    template<class T>
    using optional = Optional<T>;

    //Wrap a member function to a normal function pointer
    //Impl
    template<auto Method,class Class,class RetT,class Object,class ...Args>
    RetT _MtInvoke(Object *self,Args ...args){
        return (static_cast<Class*>(self)->*Method)(
            std::forward<Args>(args)...
        );
    };

    //TODO the signature is too long ,simplify it
    template<class Object,auto Method,class T>
    struct _MemberFunctionWrapperImpl;
    //Const method
    template<class Object,auto Method,class RetT,class Class,class ...Args>
    struct _MemberFunctionWrapperImpl<Object,Method,RetT (Class::*)(Args...) const>{
        static constexpr auto Invoke = _MtInvoke<
            Method,
            const Class,
            RetT,
            const Object,
            Args...
        >;    
    };
    //Normal method
    template<class Object,auto Method,class RetT,class Class,class ...Args>
    struct _MemberFunctionWrapperImpl<Object,Method,RetT (Class::*)(Args...)>{
        static constexpr auto Invoke = _MtInvoke<
            Method,
            Class,
            RetT,
            Object,
            Args...
        >;    
    };
    /**
     * @brief Wrap a member function to RetT (*)(ObjectType *this,Args...)
     * 
     * @tparam Method The method <&A::b>
     * @tparam ObjectType The first arg f the generated function ptr(default on void)
     */
    template<auto Method,class ObjectType = void>
    struct MemberFunctionWrapper:
        public _MemberFunctionWrapperImpl<ObjectType,Method,decltype(Method)>{

    };


    [[noreturn]] void BTKAPI throwBadFunctionCall();

    //Virtual function
    template<class T,class C = void>
    class VirtualFunction;

    template<class Object,class RetT,class ...Args>
    class VirtualFunction<Object,RetT(Args...)>{
        public:
            VirtualFunction() = default;
            ~VirtualFunction() = default;
            //Bind function
            template<auto Method>
            void bind(){
                if constexpr(not std::is_same_v<Object,void>){
                    //Check type
                    static_assert(std::is_base_of_v<
                        Object,
                        typename MemberFunctionTraits<decltype(Method)>::object_type
                    >);
                }
                entry = MemberFunctionWrapper<Method,Object>::Invoke;
            }
            //Call it
            RetT call(Object *self,Args ...args) const{
                return entry(self,std::forward<Args>(args)...);
            }
            RetT operator ()(Object *self,Args ...args) const{
                return entry(self,std::forward<Args>(args)...);
            }
        private:
            using Entry = RetT (*)(Object *,Args ...);
            Entry entry = reinterpret_cast<Entry>(throwBadFunctionCall);
    };
    //Const ver
    template<class Object,class RetT,class ...Args>
    class VirtualFunction<Object,RetT(Args...) const>{
        public:
            VirtualFunction() = default;
            ~VirtualFunction() = default;
            //Bind function
            template<auto Method>
            void bind(){
                if constexpr(not std::is_same_v<Object,void>){
                    //Check type
                    static_assert(std::is_base_of_v<
                        Object,
                        typename MemberFunctionTraits<decltype(Method)>::object_type
                    >);
                }
                entry = MemberFunctionWrapper<Method,Object>::Invoke;
            }
            //Call it
            RetT call(const Object *self,Args ...args) const{
                return entry(self,std::forward<Args>(args)...);
            }
            RetT operator ()(const Object *self,Args ...args) const{
                return entry(self,std::forward<Args>(args)...);
            }
        private:
            using Entry = RetT (*)(const Object *,Args ...);
            Entry entry = reinterpret_cast<Entry>(throwBadFunctionCall);
    };
    //For no base class check
    template<class RetT,class ...Args>
    class VirtualFunction<RetT(Args...),void>:
        public VirtualFunction<void,RetT(Args...)>{

    };
    template<class RetT,class ...Args>
    class VirtualFunction<RetT(Args...) const,void>:
        public VirtualFunction<void,RetT(Args...) const>{

    };
    /**
     * @brief Call and delete self
     * 
     * @tparam Callable 
     * @tparam Args 
     */
    template<class Callable,class ...Args>
    struct _OnceInvoker:public std::tuple<Args...>{
        Callable callable;
        static void Run(void *__self){
            _OnceInvoker *self = static_cast<_OnceInvoker*>(__self);
            std::unique_ptr<_OnceInvoker> ptr(self);
            std::apply(std::forward<Callable>(ptr->callable),
                        std::forward<std::tuple<Args...>&&>(*ptr));
            
        }
    };
    template<class Callable,class ...Args>
    struct _GenericInvoker:public std::tuple<Args...>{
        Callable callable;
        static void Run(void *__self){
            _GenericInvoker *ptr = static_cast<_GenericInvoker*>(__self);
            std::apply(std::forward<Callable>(ptr->callable),
                        std::forward<std::tuple<Args...>&&>(*ptr));
        }
    };

    //Store Pod in a pointer
    template<class Pod,class ...Args>
    void NewPodInPointer(void **ptr,Args &&...args){
        if constexpr(sizeof(Pod) > sizeof(void*)){
            //Too big
            *ptr = new Pod(std::forward<Args>(args)...);
        }
        else{
            new(reinterpret_cast<void*>(ptr)) Pod(std::forward<Args>(args)...);
        }
    }
    template<class Pod>
    void DeletePodInPointer(void *ptr){
        if constexpr(sizeof(Pod) > sizeof(void*)){
            delete static_cast<Pod*>(ptr);
        }
        else{
            //Do nothing
        }
    }
    template<class Pod>
    void  StorePodInPointer(void **ptr,const Pod &v){
        if constexpr(sizeof(Pod) > sizeof(void*)){
            *static_cast<Pod*>(*ptr) = v;
        }
        else{
            *reinterpret_cast<Pod*>(
                ptr
            ) = v;
        }
    }
    template<class Pod>
    Pod  LoadPodInPointer(void *ptr){
        if constexpr(sizeof(Pod) > sizeof(void*)){
            return *static_cast<Pod*>(ptr);
        }
        else{
            return *reinterpret_cast<Pod*>(
                &ptr
            );
        }
    }
    template<class T>
    struct GenericDeleter{
        static void Invoke(void *t){
            delete static_cast<T*>(t);
        }
    };
}

#endif // _BTK_UTILS_TEMPLATE_HPP_
