#if !defined(_BTK_OBJECT_HPP_)
#define _BTK_OBJECT_HPP_
#include "defs.hpp"
#include <type_traits>
#include <cstddef>
#include <memory>
#include <chrono>
#include <tuple>
#include <list>
namespace Btk{
    //TODO:The SpinLock is not safe to lock recursively
    //So we are better to write our own recursive spinlock
    class SignalBase;
    template<class RetT>
    class Signal;
    class Object;
    /**
     * @brief Callbacks on destroy
     * 
     */
    struct ObjectCallBack{
        enum {
            Unknown,//< Unkonw callback
            Timer,//< Callback for removeing timer
            Signal//< Callback for removeing signal
        }type;
        /**
         * @brief Call the callback
         * 
         * @note Donot forget to delete self in function run
         */
        void (*run)(Object *object,ObjectCallBack *self);
    };
    /**
     * @brief Lockguard for SlotBase and Object
     * 
     * @tparam T 
     */
    template<class T>
    struct _LockGuard_{
        _LockGuard_(T *b):base(b){
            base->lock();
        }
        _LockGuard_(const _LockGuard_ &) = delete;
        ~_LockGuard_(){
            base->unlock();
        }
        T *base;
    };
    /**
     * @brief Basic slot
     * 
     */
    class SlotBase{
        protected:
            //< pointer for delete the slot
            typedef void (*CleanupFn)(void *self,bool from_object);
            CleanupFn cleanup_ptr;
            
            SlotBase(CleanupFn f):cleanup_ptr(f){};
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
    class Slot:public SlotBase{
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
            Slot(CleanupFn c,InvokeFn i):SlotBase(c){
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
    class MemSlot:public Slot<RetT,Args...>{
        protected:
            mutable Callable callable;
            /**
             * @brief Delete self
             * 
             */
            static void Delete(void *self,bool){
                delete static_cast<MemSlot*>(self);
            }
            static RetT Invoke(const void *self,Args ...args){
                return static_cast<const MemSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args) const{
                return callable(std::forward<Args>(args)...);
            }

            MemSlot(Callable &&callable):
                Slot<RetT,Args...>(Delete,Invoke),
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
    class ClassSlot:public Slot<RetT,Args...>{
        protected:
            Class *object;
            Method method;
            std::list<ObjectCallBack*>::iterator iter;//< iter from 
            static void Delete(void *self,bool from_object){
                std::unique_ptr<ClassSlot > ptr(
                    static_cast<ClassSlot*>(self)
                );
                //Call from object
                if(not from_object){
                    //< delete it
                    auto p = (*ptr->iter);
                    //< lock the object
                    _LockGuard_<Class> locker(ptr->object);
                    p->run(nullptr,p);
                    
                    ptr->object->Object::callbacks.erase(ptr->iter);
                }
            }
            static RetT Invoke(const void *self,Args ...args){
                return static_cast<const ClassSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args) const{
                return (object->*method)(std::forward<Args>(args)...);
            }
            ClassSlot(Class *o,Method m):
                Slot<RetT,Args...>(Delete,Invoke),
                object(o),
                method(m){

            }
        template<class T>
        friend class Signal;
        friend class Object;
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

            std::list<SlotBase*> slots;//< All slots
            mutable int spinlock = 0;//< SDL_spinlock for multithreading
        template<class RetT>
        friend class Signal;
        friend class Connection;

        template<class T>
        friend struct _LockGuard_;
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
            typedef std::list<SlotBase*>::iterator Iterator;
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
     * @brief Remove connection in Object callback
     * 
     */
    struct BTKAPI ConnectionWrapper:public ObjectCallBack{
        ConnectionWrapper(Connection c):
            con(c){
            
            type = ObjectCallBack::Signal;
            run = Run;

        }
        Connection con;
        /**
         * @brief pass nullptr is just delete self
         * 
         */
        static void Run(Object *,ObjectCallBack*);
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

            template<class Callable,class ...Args>
            void on_destroy(Callable &&callable,Args &&...args){
                using Invoker = CallBackInvoker<Callable,Args...>;
                callbacks.push_back(
                    new Invoker{
                        std::forward<Callable>(callable),
                        std::forward<Args>(args)...
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
        public:
            using CallBack = ObjectCallBack;
            /**
             * @brief Invoker for callable
             * 
             * @tparam Callable 
             * @tparam Args 
             */
            template<class Callable,class ...Args>
            struct CallBackInvoker:
                public ObjectCallBack,
                       std::tuple<Args...>{

                Callable callable;
                CallBackInvoker(Callable &&object,Args &&...args):
                    CallBack{CallBack::Unknown,&Run},
                    std::tuple<Args...>{std::forward<Args>(args)...},
                    callable{std::forward<Callable>(object)}{

                }
                static void Run(Object*,CallBack *self){
                    std::unique_ptr<CallBackInvoker > ptr(
                        static_cast<CallBackInvoker*>(self)
                    );
                    std::apply(ptr->callable,static_cast<std::tuple<Args...>&&>(*ptr));
                }
            };
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
                if constexpr(std::is_member_function_pointer<Callable>::value){
                    return signal.connect(std::forward<Callable>,this);
                }
                else{
                    return signal.connect(std::forward<Callable>(callable));
                }
            }
        private:
            struct lock_guard{
                lock_guard(const Object *o){
                    object = o;
                    o->lock();
                }
                lock_guard(const lock_guard &) = delete;
                ~lock_guard(){
                    object->unlock();
                }
                const Object *object;
            };
            void lock() const;
            void unlock() const;

            std::list<ObjectCallBack*> callbacks;
            mutable int spinlock = 0;//<SDL_spinlock for multithreading
        
        template<class RetT>
        friend class Signal;
        template<class Class,class Method,class RetT,class ...Args>
        friend class ClassSlot;
        friend class ConnectionWrapper;

        template<class T>
        friend struct _LockGuard_;
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
                        static_cast<Slot<RetT,Args...>*>(slot)->invoke(
                            std::forward<Args>(args)...
                        );
                    }
                }
                else{
                    RetT ret{};
                    for(auto slot:slots){
                        ret = static_cast<Slot<RetT,Args...>*>(slot)->invoke(
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
                using Slot = MemSlot<Callable,RetT,Args...>;
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

                using Slot = ClassSlot<TObject,Method,RetT,Args...>;
                Slot *slot = new Slot(object,method);
                slots.push_back(
                    slot
                );
                //make connection
                Connection con = {this,--slots.end()};
                object->Object::callbacks.push_back(
                    new ConnectionWrapper(con)
                );

                slot->iter = --object->Object::callbacks.end();
            }
    };
    using HasSlots = Object;
}


#endif // _BTK_OBJECT_HPP_
