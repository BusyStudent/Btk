#if !defined(_BTKSIGNAL_HPP_)
#define _BTKSIGNAL_HPP_
#include <type_traits>
#include <utility>
#include <cstddef>
#include <list>
namespace Btk{
    //TODO: improve thread safety

    [[noreturn]] void throwEmptySignal();
    namespace Impl{
        struct SlotBase{
            //pointer to delete
            typedef void(*DeleteFn)(void *self,bool from_hasslots);
            SlotBase(DeleteFn fn):delete_ptr(fn){};
            //cleanup self
            //from has_slots = true if it is called by ~HasSlots()
            void cleanup(bool from_hasslots = false){
                delete_ptr(this,from_hasslots);
            };
            DeleteFn delete_ptr;
        };
        //Slot
        template<class RetT,class ...Args>
        struct Slot:public SlotBase{
            //Invoke Ptr
            typedef RetT(*InvokeFn)(const void *self,Args ...args);
            //construct Slot
            Slot(DeleteFn df,InvokeFn fn):
                SlotBase(df),
                invoke_ptr(fn){};
            //invoke self
            RetT invoke(Args ...args) const{
                invoke_ptr(this,std::forward<Args>(args)...);
            }
            InvokeFn invoke_ptr;
        };
        //SlotDetail
        template<class T,class RetT,class ...Args>
        struct SlotDetail:public Slot<RetT,Args...>{
            SlotDetail(T &&f):
                Slot<RetT,Args...>(Delete,Invoke),
                fn(std::forward<T>(f)){};

            T fn;
            //DeleteSelf
            static void Delete(void *self,bool){
                delete static_cast<SlotDetail*>(self);
            }
            //InvokeSelf
            static RetT Invoke(const void *__self,Args ...args){
                const SlotDetail *slot = static_cast<const SlotDetail*>(__self);
                return slot->fn(std::forward<Args>(args)...);
            }
        };
    };
    //Signal
    struct SignalBase{
        SignalBase();
        SignalBase(const SignalBase &) = delete;
        ~SignalBase();
        //disconnect all slots
        void disconnect_all(){
            for(auto s:slots){
                s->cleanup();
            }
        }
        bool empty() const noexcept{
            return slots.empty();
        };
        //check is empty
        bool operator ==(std::nullptr_t) const noexcept{
            return empty();
        };
        bool operator !=(std::nullptr_t) const noexcept{
            return not empty();
        };
        std::list<Impl::SlotBase*> slots;
    };

    struct Connection{
        SignalBase *current;
        std::list<Impl::SlotBase*>::iterator iter;
        //disconnect this connection
        void disconnect(bool from_hasslots = false){
            (*iter)->cleanup(from_hasslots);
            current->slots.erase(iter);
        }
        //disconnect after checking
        //return false if failed
        bool try_disconnect(bool from_hasslots = false){
            std::list<Impl::SlotBase*>::iterator i;
            for(i = current->slots.begin();i != current->slots.end(); ++i){
                if(i == iter){
                    //get it
                    disconnect(from_hasslots);
                    return true;
                }
            }
            return false;
        }
    };

    template<class RetT>
    struct Signal;

    template<class RetT,class ...Args>
    struct Signal<RetT(Args...)>;

    //a class to auto track connections
    struct HasSlots{
        HasSlots();
        HasSlots(const HasSlots &) = delete;
        ~HasSlots();

        //disconnect all signal
        void disconnect_all();

        std::list<Connection> _connections;
    };

    namespace Impl{
        //ClassSlot to hold member pointer
        template<class Class,class Method,class RetT,class ...Args>
        struct ClassSlot:public Slot<RetT,Args...>{
            ClassSlot(Class *ptr,Method m)
                :Slot<RetT,Args...>(Delete,Invoke),
                class_ptr(ptr),
                method(m){

            }
            Class *class_ptr;
            Method method;
            std::list<Connection>::iterator iter;
            //track HasSlots connection
            static RetT Invoke(const void *__self,Args ...args){
                const ClassSlot *slot = static_cast<const ClassSlot*>(__self);
                return ((slot->class_ptr)->*(slot->method))(
                    std::forward<Args>(args)...
                );
            }
            static void Delete(void *self,bool from_hasslots){
                ClassSlot *slot = static_cast<ClassSlot*>(self);
                if(not from_hasslots){
                    //not called by ~HasSlots()
                    //unregister it
                    slot->class_ptr->HasSlots::_connections.erase(
                        slot->iter
                    );
                }
                delete slot;
            }
        };
    };

    template<class RetT,class ...Args>
    struct Signal<RetT(Args...)>:public SignalBase{
        Signal(){}
        Signal(const Signal &) = delete;
        
        RetT emit(Args ...args) const{
            if(empty()){
                throwEmptySignal();
            }
            if constexpr(std::is_same<RetT,void>::value){
                //no return value
                for(auto s:slots){
                    static_cast<Impl::Slot<RetT,Args...>*>(s)->invoke(
                        std::forward<Args>(args)...
                    );
                }
            }
            else{
                //Get return value
                RetT ret;
                for(auto s:slots){
                    ret = static_cast<Impl::Slot<RetT,Args...>*>(s)->invoke(
                        std::forward<Args>(args)...
                    );
                }
                return ret;
            }
        }
        //connect anything callable
        template<class Callable>
        Connection connect(Callable &&callable){
            slots.push_back(
                new Impl::SlotDetail<Callable,RetT,Args...>(
                    std::forward<Callable>(callable)
                )
            );
            return Connection{
                this,
                --slots.end()
            };
        }
        //connect object inherit HasSlots
        template<class Callable,class Object>
        void connect(Callable &&callable,Object *object){
            static_assert(std::is_base_of<HasSlots,Object>::value,"Object must inherit HasSlots");

            auto *solt = new Impl::ClassSlot<Object,Callable,RetT,Args...>(
                object,callable
            );
            slots.push_back(solt);
            
            object->HasSlots::_connections.push_back({
                this,
                --slots.end()
            });
            //set hasslots iterator

            solt->iter = --(object->HasSlots::_connections.end());
        }
        RetT operator ()(Args ...args) const{
            return Signal::emit(std::forward<Args>(args)...);
        }

        typedef RetT result_type;
    };
};


#endif // _BTKSIGNAL_HPP_
