#if !defined(_BTK_OBJECT_HPP_)
#define _BTK_OBJECT_HPP_
#include "utils/template.hpp"
#include "utils/traits.hpp"
#include "utils/sync.hpp"
#include "impl/invoker.hpp"
#include "defs.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdio>
#include <chrono>
#include <tuple>
#include <list>
namespace Btk{
    //TODO:The SpinLock is not safe to lock recursively
    //So we are better to write our own recursive spinlock
    BTKAPI void DeferCall(void(* fn)(void*),void *data);

    class SignalBase;
    template<class RetT>
    class Signal;
    class Object;
    class _SlotBase;
    struct _Functor;
    
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
            void disconnect(bool from_object = false);
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
            typedef RetT (*InvokeFn)(void *self,Args ...args);
            InvokeFn invoke_ptr;
            /**
             * @brief Call the slot
             * 
             * @param args 
             * @return RetT 
             */
            RetT invoke(Args ...args){
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
            Callable callable;
            /**
             * @brief Delete self
             * 
             */
            static void Delete(void *self,bool){
                delete static_cast<_MemSlot*>(self);
            }
            static RetT Invoke(void *self,Args ...args){
                return static_cast<_MemSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args){
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
                Btk::unique_ptr<_ClassSlot > ptr(
                    static_cast<_ClassSlot*>(self)
                );
                //Call from object
                if(not from_object){
                    //< lock the object
                    Btk::lock_guard<Class> locker(*(ptr->object));
                    
                    
                    ptr->object->Object::remove_callback(ptr->location);
                }
            }
            static RetT Invoke(void *self,Args ...args){
                return static_cast<_ClassSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args){
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
    class _GenericCallBase{
        protected:
            bool deleted = false;
        friend struct _GenericCallFunctor;
    };
    /**
     * @brief A class for DeferCall or async
     * 
     * @tparam Class 
     * @tparam Method 
     * @tparam Args 
     */
    template<class Class,class Method,class ...Args>
    class _GenericCallInvoker:public std::tuple<Class*,Args...>,_GenericCallBase{
        Method method;
        /**
         * @brief Construct a new defercallinvoker object
         * 
         * @param object 
         * @param method 
         */
        _GenericCallInvoker(Class *object,Method method,Args ...args):
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
            Btk::lock_guard<Object> locker(*(object()),Btk::adopt_lock);
            //< Call self
            std::apply(method,static_cast<std::tuple<Class*,Args...>&&>(*this));
        }
        Class *object(){
            return std::get<0>(static_cast<std::tuple<Class*,Args...>&&>(*this));
        }
        /*
         * @brief Main entry for DeferCall
         * 
         * @param self
         */
        static void Run(void *self){
            Btk::unique_ptr<_GenericCallInvoker> ptr(
                static_cast<_GenericCallInvoker*>(self)
            );
            ptr->invoke();
        }
        friend class Object;
    };
    /**
     * @brief Helper class for defercall or async invoker
     * 
     */
    struct BTKAPI _GenericCallFunctor:public _Functor{
        _GenericCallFunctor(_GenericCallBase *defercall);
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
                return spinlock.is_lock();
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
            /**
             * @brief Debug for dump
             * 
             * @param output 
             */
            void dump_slots(FILE *output = stderr) const;
        public:
            /**
             * @brief Lock the signal
             * @internal use shound not use it
             */
            void lock() const{
                spinlock.lock();
            }
            /**
             * @brief Unlock the signal
             * @internal use shound not use it
             */

            void unlock() const{
                spinlock.unlock();
            }
        protected:
            std::list<_SlotBase*> slots;//< All slots
            mutable SpinLock spinlock;//< SDL_spinlock for multithreading
        template<class RetT>
        friend class Signal;
        friend class Connection;
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
                using Invoker = Impl::OnceInvoker<Callable,Args...>;
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
            /**
             * @brief Remove callback(safe to pass invalid location,but slower)
             * 
             * @param location 
             * @return FunctorLocation 
             */
            FunctorLocation remove_callback_safe(FunctorLocation location);
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
                        std::forward<Callable>(callable),
                        static_cast<typename MemberFunctionTraits<Callable>::object_type*>(this)
                    );
                }
                else{
                    return signal.connect(std::forward<Callable>(callable));
                }
            }
            template<class RetT,class Class,class ...Args>
            void defer_call(RetT (Class::*method)(Args...),Args &&...args){
                using Method = std::remove_pointer_t<decltype(method)>;
                using Invoker = _GenericCallInvoker<Class,Method,Args...>;
                Invoker *invoker = new Invoker(
                    static_cast<Class*>(this),
                    method,
                    std::forward<Args>(args)...
                );
                _GenericCallFunctor functor(invoker);
                DeferCall(Invoker::Run,static_cast<void*>(invoker));
            }
        public:
            bool try_lock() const{
                return spinlock.try_lock();
            }
            void lock() const{
                spinlock.lock();
            }
            void unlock() const{
                spinlock.unlock();
            }
            /**
             * @brief Debug for dump functors
             * 
             * @param output 
             */
            void dump_functors(FILE *output = stderr) const;
        protected:
            //
        private:
            std::list<Functor> functors_cb;
            mutable SpinLock spinlock;//<SDL_spinlock for multithreading
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
                lock_guard<const SignalBase> locker(*this);
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
                lock_guard<const SignalBase> locker(*this);
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
                lock_guard<const SignalBase> locker(*this);

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

    //Timer
    
    struct TimerImpl;
    class  Timer;

    //Impl
    struct _TimerInvokerData{
        Uint32 (*invoke)(Uint32 interval,void *self) = nullptr;
        void   *invoker = nullptr;
        void   (*cleanup)(void *) = nullptr;
    };


    template<bool Val>
    struct _TimerFunctorLocation;

    template<>
    struct _TimerFunctorLocation<true>{
        _FunctorLocation location;
        bool need_remove = true;//< Flags to check if we need remove
    };
    template<>
    struct _TimerFunctorLocation<false>{

    };
    /**
     * @brief Invoker for Timer
     * 
     * @tparam Callable 
     * @tparam Args 
     */
    template<class Callable,class ...Args>
    struct _TimerInvoker:public std::tuple<Args...>,
                         public _TimerFunctorLocation<std::is_member_function_pointer_v<Callable>>{

        
        _TimerInvoker(Callable &&c,Args &&...args):
            std::tuple<Args...>(std::forward<Args>(args)...),
            callable(std::forward<Callable>(c)){

            }

        Callable callable;

        decltype(auto) invoke(Uint32 interval){
            return std::apply(callable,static_cast<std::tuple<Args...>&&>(*this));
        }

        Uint32 run(Uint32 interval){
            if constexpr(std::is_same_v<std::invoke_result_t<Callable,Args...>,void>){
                //No return value
                invoke(interval);
                return interval;
            }
            else{
                return invoke(interval);
            }
        }
        static Uint32 Run(Uint32 interval,void *self){
            return static_cast<_TimerInvoker*>(self)->run(interval);
        }
        static void Delete(void *__self){
            auto self = static_cast<_TimerInvoker*>(__self);
            if constexpr(std::is_member_function_pointer_v<Callable>){
                //Get object and begin remove
                auto object = std::get<0>(static_cast<std::tuple<Args...>&&>(*self));
                if(self->need_remove){
                   object->remove_callback(self->location); 
                }
            }
            delete self;
        }
    };

    //Timer Functor
    struct BTKAPI _TimerFunctor:public _Functor{
        _TimerFunctor(Btk::Timer &timer,bool &need_remove_ref);
    };
    /**
     * @brief Timer
     * @todo Fix Memory leak
     * 
     */
    class BTKAPI Timer{
        public:
            Timer();
            Timer(const Timer &) = delete;
            ~Timer();


            void stop();
            void start();

            bool running() const;
            /**
             * @brief Get Timer's interval
             * 
             * @return Uint32 
             */
            Uint32 interval() const;
            void set_interval(Uint32 interval);

            /**
             * @brief Bind to 
             * 
             * @tparam Callable 
             * @tparam Args 
             * @param callable 
             * @param args 
             */
            template<
                class Callable,
                class ...Args,
                typename _Cond = std::enable_if_t<!std::is_member_function_pointer_v<Callable>>
            >
            void bind(Callable &&callable,Args &&...args){
                using Invoker = _TimerInvoker<Callable,Args...>;
                _TimerInvokerData data;
                auto invoker = new Invoker{
                    std::forward<Callable>(callable),
                    std::forward<Args>(args)...
                };
                data.invoker = invoker;
                data.invoke  = Invoker::Run;
                data.cleanup = Invoker::Delete;
                bind(data);
            }
            /**
             * @brief Bind to Object's member function
             * 
             * @tparam Callable 
             * @tparam Object 
             * @tparam Args 
             * @param callable 
             * @param object 
             * @param args 
             */
            template<
                class Callable,
                class Object,
                class ...Args,
                typename _Cond = std::enable_if_t<std::is_member_function_pointer_v<Callable>>
            >
            void bind(Callable &&callable,Object object,Args &&...args){
                using Invoker = _TimerInvoker<Callable,Object,Args...>;
                _TimerInvokerData data;
                auto invoker = new Invoker{
                    std::forward<Callable>(callable),
                    std::forward<Object>(object),std::forward<Args>(args)...
                };
                data.invoker = invoker;
                data.invoke  = Invoker::Run;
                data.cleanup = Invoker::Delete;

                //Check is based on HasSlots
                constexpr bool val = std::is_base_of<HasSlots,std::remove_pointer_t<Object>>();
                static_assert(val,"Object must inhert HasSlots");

                //TODO Add timer functor
                _TimerFunctor functor(*this,invoker->need_remove);
                auto    loc = static_cast<HasSlots*>(object)->add_functor(functor);

                invoker->location = loc;
            
                bind(data);
            }
            /**
             * @brief Reset the callback
             * 
             */
            void reset(){
                _TimerInvokerData data = {nullptr,nullptr,nullptr};
                bind(data);
            }

            template<class Callable,class ...Args>
            [[depercated("Using Timer::bind instead")]]
            Timer &set_callback(Callable &&callable,Args &&...args){
                bind(std::forward<Callable>(callable),std::forward<Args>(args)...);
                return *this;
            }
        private:
            /**
             * @brief Bind invoker
             * 
             */
            void bind(_TimerInvokerData);

            TimerImpl *timer;
    };
}


#endif // _BTK_OBJECT_HPP_
