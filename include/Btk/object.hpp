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
    class SignalBase;
    template<class RetT>
    class Signal;
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
        template<class RetT>
        friend class Signal;
    };
    /**
     * @brief Slot for callable object
     * 
     * @tparam RetT 
     * @tparam Args 
     */
    template<class Callable,class RetT,class ...Args>
    class MemSlot:public SlotBase{
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
                return static_cast<MemSlot*>(self)->invoke(
                    std::forward<Args>(args)...
                );
            }
            RetT invoke(Args ...args) const{
                return callable(std::forward<Args>(args)...);
            }

            MemSlot(Callable &&callable):
                Slot(Delete,Invoke),
                callable(std::forward<Callable>(callable)){

            }
        template<class RetT>
        friend class Signal;
        friend class Object;
    };
    /**
     * @brief Slot for Object's member function 
     * 
     */
    template<class Function,class RetT,class ...Args>
    class ObjectSlot:public SlotBase{


        template<class RetT>
        friend class Signal;
        friend class Object;
    };
    class Connection{
        public:
            Connection() = default;
            Connection(const Connection &) = default;


            SignalBase *signal() const noexcept{
                return current;
            }
        private:
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
                        callable,
                        std::forward<Args>(args)...
                    }
                );
            }
            /**
             * @brief Disconnect all signal
             * 
             */
            void disconnect_all();
        public:
            /**
             * @brief Callbacks on destroy
             * 
             */
            struct CallBack{
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
                void (*run)(Object *object,CallBack *self);
            };
            /**
             * @brief Invoker for callable
             * 
             * @tparam Object 
             * @tparam Args 
             */
            template<class Object,class ...Args>
            struct CallBackInvoker:
                public CallBack,
                       std::tuple<Args...>{

                Object callable;
                CallBackInvoker(Object &&object,Args &&...args):
                    CallBack{CallBack::Unknown,&Run},
                    std::tuple<Args...>{std::forward<Args>(args)...},
                    callable{std::forward<Object>(object)}{

                }
                static void Run(Object*,CallBack *self){
                    std::unique_ptr<CallBackInvoker*> ptr(
                        static_cast<CallBackInvoker*>(self)
                    );
                    std::apply(ptr->callable,static_cast<std::tuple<Args>...>&&>(*ptr));
                }
            };
        private:
            void lock() const;
            void unlock() const;

            std::list<CallBack*> callbacks;
            mutable int spinlock = 0;//<SDL_spinlock for multithreading
        
        template<class RetT>
        friend class Signal;
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
            /**
             * @brief Emit the signal
             * 
             * @param args The args
             * @return RetT The return type
             */
            RetT emit(Args ...args) const{
                //lock the signalbase
                lock_guard locker(this);

                if(std::is_same<void,RetT>()){
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
            template<class Callable,class TObject>
            void connect(Callable &&callable,TObject *object){
                static_assert(std::is_base_of<Object,TObject>(),"Object must inherit HasSlots");

            }
    };
    using HasSlots = Object;
}


#endif // _BTK_OBJECT_HPP_
