#if !defined(_BTK_OBJECT_HPP_)
#define _BTK_OBJECT_HPP_
#include "defs.hpp"
#include "impl/invoker.hpp"
#include <type_traits>
#include <cstddef>
#include <memory>
#include <chrono>
#include <tuple>
#include <mutex>
#include <list>
namespace Btk{
    //TODO:The SpinLock is not safe to lock recursively
    //So we are better to write our own recursive spinlock
    BTKAPI void DeferCall(void(* fn)(void*),void *data);
    //MemberFunction traits

    template<class T>
    struct _MemberFunction{};
    /**
     * @brief Get detail information of the function
     * 
     * @tparam RetT 
     * @tparam Class 
     * @tparam Args 
     */
    template<class RetT,class Class,class ...Args>
    struct _MemberFunction<RetT (Class::*)(Args...)>{
        using result_type = RetT;
        using object_type = Class;

    };
    //for const method
    template<class RetT,class Class,class ...Args>
    struct _MemberFunction<RetT (Class::*)(Args...) const>{
        using result_type = RetT;
        using object_type = Class;
    };

    class SignalBase;
    template<class RetT>
    class Signal;
    class Object;
    /**
     * @brief Functor in Object for cleaningup data
     * 
     */
    struct _Functor{
        enum {
            Unknown,//< Unkonw callback
            Timer,//< Callback for removeing timer
            Signal//< Callback for removeing signal
        }magic = Unknown;
        //Userdata
        void *user1;
        void *user2;
        /**
         * @brief Call the _Functor,cleanup will not be called after it
         * 
         */
        void (*call)(_Functor&) = nullptr;//<It could be nullptr
        /**
         * @brief Cleanup The _Functor
         * 
         */
        void (*cleanup)(_Functor&) = nullptr;//<It could be nullptr

        bool reserved = false;//< Flag for call from Object
        bool reserved1 = false;
        bool reserved2 = false;
        bool reserved3 = false;

        //methods
        void _call(){
            if(call != nullptr){
                call(*this);
            }
        }
        void _cleanup(){
            if(cleanup != nullptr){
                cleanup(*this);
            }
        }
    };
    /**
     * @brief FunctionLocation
     * 
     */
    struct _FunctorLocation{
        std::list<_Functor>::iterator iter;
        
        _Functor *operator ->(){
            return iter.operator->();
        }
    };
    struct _TimerID{
        int id;
        /**
         * @brief Stop the timer 
         * 
         */
        void stop();
    };
    /**
     * @brief Basic slot
     * 
     */
    class _SlotBase{
        protected:
            //< pointer for delete the slot
            typedef void (*CleanupFn)(void *self,bool from_object);
            CleanupFn cleanup_ptr;
            
            _SlotBase(CleanupFn f):cleanup_ptr(f){};
        private:
            /**
             * @brief Delete the slot
             * 
             * @param from_object Is the Object::~Object call the method?
             */
            void cleanup(bool from_object = false){
                cleanup_ptr(this,from_object);
            }
        template<class RetT>
        friend class Signal;
        friend class SignalBase;
        friend class Connection;
    };
    /**
     * @brief Slot interface for emit
     * 
     * @tparam RetT 
     * @tparam Args 
     */
    template<class RetT,class ...Args>
    class _Slot:public _SlotBase{
        protected:
            typedef RetT (*InvokeFn)(const void *self,Args ...args);
            InvokeFn invoke_ptr;
            /**
             * @brief Call the slot
             * 
             * @param args 
             * @return RetT 
             */
            RetT invoke(Args ...args) const{
                return invoke_ptr(this,std::forward<Args>(args)...);
            }
            _Slot(CleanupFn c,InvokeFn i):_SlotBase(c){
                invoke_ptr = i;
            }
        template<class T>
        friend class Signal;
        friend class Connection;
    };
    /**
     * @brief Slot for callable object
     * 
     * @tparam RetT 
     * @tparam Args 
     */
    template<class Callable,class RetT,class ...Args>
    class _MemSlot:public _Slot<RetT,Args...>{
        protected:
            mutable Callable callable;
            /**
             * @brief Delete self
             * 
             */
            static void Delete(void *self,bool){
                delete static_cast<_MemSlot*>(self);
            }
            static RetT Invoke(const void *self,Args ...args){
                return static_cast<const _MemSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args) const{
                return callable(std::forward<Args>(args)...);
            }

            _MemSlot(Callable &&callable):
                _Slot<RetT,Args...>(Delete,Invoke),
                callable(std::forward<Callable>(callable)){

            }
        template<class T>
        friend class Signal;
        friend class Object;
    };
    /**
     * @brief Slot for Btk::HasSlots's member function
     * 
     * @tparam Callable 
     * @tparam Method
     * @tparam RetT 
     * @tparam Args 
     */
    template<class Class,class Method,class RetT,class ...Args>
    class _ClassSlot:public _Slot<RetT,Args...>{
        protected:
            Class *object;
            Method method;
            _FunctorLocation location;
            static void Delete(void *self,bool from_object){
                std::unique_ptr<_ClassSlot > ptr(
                    static_cast<_ClassSlot*>(self)
                );
                //Call from object
                if(not from_object){
                    //< lock the object
                    std::lock_guard<Class> locker(*(ptr->object));
                    
                    
                    ptr->object->Object::remove_callback(ptr->location);
                }
            }
            static RetT Invoke(const void *self,Args ...args){
                return static_cast<const _ClassSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args) const{
                return (object->*method)(std::forward<Args>(args)...);
            }
            _ClassSlot(Class *o,Method m):
                _Slot<RetT,Args...>(Delete,Invoke),
                object(o),
                method(m){

            }
        template<class T>
        friend class Signal;
        friend class Object;
    };
    class _DeferCallBase{
        protected:
            bool deleted = false;
        friend struct _DeferCallFunctor;
    };
    /**
     * @brief A class for DeferCall
     * 
     * @tparam Class 
     * @tparam Method 
     * @tparam Args 
     */
    template<class Class,class Method,class ...Args>
    class _DeferCallInvoker:public std::tuple<Class*,Args...>,_DeferCallBase{
        Method method;
        /**
         * @brief Construct a new defercallinvoker object
         * 
         * @param object 
         * @param method 
         */
        _DeferCallInvoker(Class *object,Method method,Args ...args):
            std::tuple<Class*,Args...>(object,std::forward<Args>(args)...){

            this->method = method;
        }
        void invoke(){
            if(deleted){
                return;
            }
            if(not object()->try_lock()){
                //< Failed to lock,The object is cleanuping
                return;
            }
            std::lock_guard<Object> locker(*(object()),std::adopt_lock);
            //< Call self
            std::apply(method,static_cast<std::tuple<Class*,Args...>&&>(*this));
        }
        Class *object(){
            return std::get<0>(static_cast<std::tuple<Class*,Args...>&&>(*this));
        }
        /**
         * @brief Main entry for DeferCall
         * 
         * @param self 
         */
        static void Run(void *self){
            std::unique_ptr<_DeferCallInvoker> ptr(
                static_cast<_DeferCallInvoker*>(self)
            );
            ptr->invoke();
        }
        friend class Object;
    };
    /**
     * @brief Helper class for defercall's invoker
     * 
     */
    struct BTKAPI _DeferCallFunctor:public _Functor{
        _DeferCallFunctor(_DeferCallBase *defercall);
    };
    /**
     * @brief Basic signal
     * 
     */
    class BTKAPI SignalBase{
        public:
            SignalBase();
            SignalBase(const SignalBase &) = delete;
            ~SignalBase();
            /**
             * @brief The signal is empty?
             * 
             * @return true 
             * @return false 
             */
            bool empty() const{
                return slots.empty();
            }
            /**
             * @brief The signal is emitting?
             * 
             * @return true 
             * @return false 
             */
            bool emitting() const{
                return spinlock;
            }
            bool operator ==(std::nullptr_t) const{
                return empty();
            }
            bool operator !=(std::nullptr_t) const{
                return not empty();
            }
            operator bool() const{
                return not empty();
            }
        protected:
            struct lock_guard{
                lock_guard(const SignalBase *b):sigbase(b){
                    sigbase->lock();
                }
                lock_guard(const lock_guard &) = delete;
                ~lock_guard(){
                    sigbase->unlock();
                }
                const SignalBase *sigbase;
            };
            void lock() const;
            void unlock() const;

            std::list<_SlotBase*> slots;//< All slots
            mutable int spinlock = 0;//< SDL_spinlock for multithreading
        template<class RetT>
        friend class Signal;
        friend class Connection;
    };
    /**
     * @brief Signal's connection
     * 
     */
    class Connection{
        public:
            Connection() = default;
            Connection(const Connection &) = default;


            SignalBase *signal() const noexcept{
                return current;
            }
            void disconnect(bool from_object = false){
                (*iter)->cleanup(from_object);
                current->slots.erase(iter);
            }
        protected:
            typedef std::list<_SlotBase*>::iterator Iterator;
            SignalBase *current;//< current signal
            Iterator iter;//<The iterator of the slot ptr
            Connection(SignalBase *c,Iterator i):
                current(c),
                iter(i){

            }
        template<class RetT>
        friend class Signal;
        friend class Object;
    };
    /**
     * @brief Generic object provide signals/slots timer etc...
     * 
     */
    class BTKAPI Object{
        public:
            Object();
            Object(const Object &) = delete;
            ~Object();
            using TimerID = _TimerID;
            using Functor = _Functor;
            using FunctorLocation = _FunctorLocation;

            template<class Callable,class ...Args>
            void on_destroy(Callable &&callable,Args &&...args){
                using Invoker = Impl::Invoker<Callable,Args...>;
                add_callback(
                    Invoker::Run,
                    new Invoker{
                        {std::forward<Args>(args)...},
                        std::forward<Callable>(callable)
                    }
                );
            }
            
            /**
             * @brief Disconnect all signal
             * 
             */
            void disconnect_all();
            /**
             * @brief Disconnect all signal and remove all timer
             * 
             */
            void cleanup();

            FunctorLocation add_callback(void(*fn)(void*),void *param);
            FunctorLocation add_functor(const Functor &);
            FunctorLocation remove_callback(FunctorLocation location);
            //Timer
            TimerID add_timer(Uint32 internal);
        public:
            /**
             * @brief Connect signal
             * 
             * @tparam Signal 
             * @tparam Callable 
             * @param signal 
             * @param callable 
             * @return decltype(auto) 
             */
            template<class Signal,class Callable>
            decltype(auto) connect(Signal &&signal,Callable &&callable){
                //FIXME!! It still have sth wrong
                if constexpr(std::is_member_function_pointer<Callable>::value){
                    //NOTE:If we use std::forward<Callable>(callable)
                    //It will have a template argument deduction/substitution failure
                    return signal.connect(
                        callable,
                        static_cast<typename _MemberFunction<Callable>::object_type*>(this)
                    );
                }
                else{
                    return signal.connect(std::forward<Callable>(callable));
                }
            }
            template<class RetT,class Class,class ...Args>
            void defer_call(RetT (Class::*method)(Args...),Args &&...args){
                using Method = std::remove_pointer_t<decltype(method)>;
                using Invoker = _DeferCallInvoker<Class,Method,Args...>;
                Invoker *invoker = new Invoker(
                    static_cast<Class*>(this),
                    method,
                    std::forward<Args>(args)...
                );
                _DeferCallFunctor functor(invoker);
                DeferCall(Invoker::Run,static_cast<void*>(invoker));
            }
        public:
            bool try_lock() const;
            void lock() const;
            void unlock() const;
        protected:
            //
        private:
            std::list<Functor> functors_cb;
            mutable int spinlock = 0;//<SDL_spinlock for multithreading
        
        template<class RetT>
        friend class Signal;
    };
    /**
     * @brief Functor for Connection
     * 
     */
    struct BTKAPI _ConnectionFunctor:public _Functor{
        /**
         * @brief Construct a new connectfunctor object
         * 
         * @param con The connection
         */
        _ConnectionFunctor(Connection con);
    };
    /**
     * @brief Signals
     * 
     * @tparam RetT Return types
     * @tparam Args The args
     */
    template<class RetT,class ...Args>
    class Signal<RetT(Args...)>:public SignalBase{
        public:
            using SignalBase::SignalBase;
            using result_type = RetT;
            /**
             * @brief Emit the signal
             * 
             * @param args The args
             * @return RetT The return type
             */
            RetT emit(Args ...args) const{
                //lock the signalbase
                lock_guard locker(this);
                //why it has complie error on msvc
                //std::is_same<void,RetT>()
                if constexpr(std::is_same<void,RetT>::value){
                    for(auto slot:slots){
                        static_cast<_Slot<RetT,Args...>*>(slot)->invoke(
                            std::forward<Args>(args)...
                        );
                    }
                }
                else{
                    RetT ret{};
                    for(auto slot:slots){
                        ret = static_cast<_Slot<RetT,Args...>*>(slot)->invoke(
                            std::forward<Args>(args)...
                        );
                    }
                    return ret;
                }
            }
            RetT operator ()(Args ...args) const{
                return emit(std::forward<Args>(args)...);
            }
            /**
             * @brief Connect callable
             * 
             * @tparam Callable 
             * @param callable 
             * @return Connection 
             */
            template<class Callable>
            Connection connect(Callable &&callable){
                lock_guard guard(this);
                using Slot = _MemSlot<Callable,RetT,Args...>;
                slots.push_back(
                    new Slot(std::forward<Callable>(callable))
                );
                return Connection{
                    this,
                    --slots.end()
                };
            }
            template<class Method,class TObject>
            void connect(Method &&method,TObject *object){
                static_assert(std::is_base_of<Object,TObject>(),"Object must inherit HasSlots");
                lock_guard guard(this);

                using Slot = _ClassSlot<TObject,Method,RetT,Args...>;
                Slot *slot = new Slot(object,method);
                slots.push_back(
                    slot
                );
                //make connection
                Connection con = {this,--slots.end()};
                
                _ConnectionFunctor functor(con);

                slot->location = object->Object::add_functor(functor);
                
            }
    };
    using HasSlots = Object;
}


#endif // _BTK_OBJECT_HPP_
