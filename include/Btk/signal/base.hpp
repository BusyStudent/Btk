#if !defined(_BTK_SIGNAL_BASE_HPP_)
#define _BTK_SIGNAL_BASE_HPP_

#include <list>

#include "../utils/traits.hpp"
#include "../utils/sync.hpp"
#include "../defs.hpp"

#include "call.hpp"

namespace Btk{
    //Forward decl
    class Object;

    struct _BindWithMemFunction{
        Object *object_ptr;
    };
    // class _SignalBase;
    // class _SlotBase;
    // /**
    //  * @brief Connection for manage it
    //  * 
    //  */
    // class Connection{
    //     public:
    //         void disconnect(){
    //             if(_disconnect == nullptr){
    //                 return;
    //             }
    //             _disconnect(this);
    //         }
    //     private:

    //         union{
    //             //Signal disconnect
    //             struct{
    //                 _SignalBase *ptr;
    //                 std::list<_SlotBase*>::iterator iter;
    //             }sig;
    //             struct{

    //             }hasslots;
    //             struct{
    //                 void *pad1;
    //                 void *pad2;
    //             }pad;
    //         }u;
    //         void (*_disconnect)(Connection *);
    //     template<class RetT>
    //     friend class Signal;
    //     friend class HasSlots;
    // };

    // class _HasSlotsCallBack{
    //     public:
    //         void *uptr;
    //         void (*call)(void *uptr);
    //         void (*remove)(void *uptr);

    //         void do_call(){
    //             if(call != nullptr){
    //                 call(uptr);
    //             }
    //         }
    //         void do_remove(){
    //             if(remove != nullptr){
    //                 remove(uptr);
    //             }
    //         }
    // };
    // class _HasSlotsImpl{
    //     public:
    //         using iterator = std::list<_HasSlotsCallBack>::iterator;
    //         void run_callbacks();
    //         void remove_callback(iterator iter);
    //         auto add_callback(_HasSlotsCallBack cb) -> iterator;
    //     private:
    //         std::list<_HasSlotsCallBack> callbacks;
    //         SpinLock spinlock;
    // };

    // class _SlotBase{
    //     public:
    //         void do_cleanup(){
    //             return cleanup(this);
    //         }
    //     protected:
    //         void (*cleanup)(void *self);
    // };

    // template<class RetT,class ...Args>
    // class _Slot:public _SlotBase{
    //     public:
    //         RetT do_call(Args &&...args){
    //             return call(std::forward<Args>(args)...);
    //         }
    //     protected:
    //         RetT (*call)(void *base,Args ...args);
    // };
    // /**
    //  * @brief Slot for Common resuorce
    //  * 
    //  * @tparam Callable 
    //  * @tparam RetT 
    //  * @tparam Args 
    //  */
    // template<class Callable,class RetT,class ...Args>
    // class _CommonSlot:public _SlotBase<RetT,Args...>{
    //     public:
    //         _CommonSlot(Callable &&c):
    //             callable(c){

    //             this->call    = Call;
    //             this->cleanup = Cleanup;
    //         }
    //     private:
    //         Callable callable;

    //         static void Cleanup(void *self){
    //             delete static_cast<_CommonSlot*>(self);
    //         }
    //         static RetT Call(void *self,Args ...args){
    //             return _Call(
    //                 static_cast<_CommonSlot*>(self)->callable,
    //                 std::forward<Args>(args)...
    //             );
    //         }
    // };
    // /**
    //  * @brief Slot for member function callable
    //  * 
    //  * @tparam Callable 
    //  * @tparam Ret 
    //  * @tparam Args 
    //  */
    // template<class Callable,class Ret,class ...Args>
    // class _MemberFunctionSlot:public _SlotBase<RetT,Args...>{
    //     public:
    //         _MemberFunctionSlot(Callable &&c,_HasSlotsImpl *obj):
    //             callable(c),
    //             object(obj){
                
    //             this->call    = Call;
    //             this->cleanup = Cleanup;
    //         }
    //     private:
    //         void do_disconnect(){
    //             con.disconnect();
    //         }
    //         static RetT Call(void *self,Args ...args){
    //             return _Call(
    //                 static_cast<_CommonSlot*>(self)->callable,
    //                 std::forward<Args>(args)...
    //             );
    //         }
    //         static void Cleanup(void *self){
    //             delete static_cast<_CommonSlot*>(self);
    //         }

    //         //<Callable for Btk::bind / _MemberFunctionBinder
    //         Callable callable;
    //         Connection    con;
    // };
    /**
     * @brief Binder for Wrap method with object to 
     * 
     * @tparam Method 
     */
    template<class Method>
    class _MemberFunctionBinder{
        public:
            using object_type = typename MemberFunctionTraits<Method>::object_type;
            using result_type = typename MemberFunctionTraits<Method>::result_type;
            //Init
            _MemberFunctionBinder(Method m,object_type *c):
                method(m),
                obj(c){

            }
            //Class
            template<class ...Args>
            result_type operator ()(Args &&...args) const{
                return _Call(method,obj,std::forward<Args>(args)...);
            }
        private:
            Method method;
            object_type *obj;
    };

    // //Decl HasSlots / SignalBase
    // class BTKAPI HasSlots{
    //     public:
    //         HasSlots();
    //         HasSlots(const HasSlots &);
    //         HasSlots(HasSlots &&);
    //         ~HasSlots();

    //         auto add_callback(_HasSlotsCallBack callback){
    //             return get()->add_callback(callback);
    //         }
    //         void run_callbacks(){
    //             return get()->run_callbacks();
    //         }
    //     private:
    //         _HasSlotsImpl *get();
    //         _HasSlotsImpl *ptr;
    // };
}
#endif // _BTK_SIGNAL_BASE_HPP_
