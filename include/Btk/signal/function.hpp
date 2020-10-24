#if !defined(_BTKSIGNAL_FUNCTION_HPP_)
#define _BTKSIGNAL_FUNCTION_HPP_
#include <cstddef>
#include <utility>
#include <type_traits>
namespace Btk{
    //Impl
    namespace Impl{
        //Function manager
        struct ManagerBase{
            int refcount;
        };
        template<class T,class RetT,class ...Args>
        struct Manager:public ManagerBase{
            //target fn
            Manager(T f):
                ManagerBase({1}),
                fn(std::forward<T>(f)){}
            T fn;
            //CallFunction
            static RetT Invoke(void *__self,Args ...args){
                Manager *self = static_cast<Manager*>(__self);
                return self->fn(std::forward<Args>(args)...);
            };
            static void Delete(void *__self){
                delete static_cast<Manager*>(__self);
            };
        };
    };
    //throw BadFunctionCall
    [[noreturn]] void throwBadFunctionCall();
    //Function Base
    struct FunctionBase{
        FunctionBase():
            manager_ptr(nullptr),
            delete_ptr(nullptr){};
        //Copy function base
        FunctionBase(const FunctionBase &fn){
            manager_ptr = fn.manager_ptr;
            delete_ptr  = fn.delete_ptr;
            if(delete_ptr != nullptr){
                //has refcount
                auto base = static_cast<Impl::ManagerBase*>(manager_ptr);
                base->refcount += 1;
            }
        }
        FunctionBase(FunctionBase &&fn){
            manager_ptr = fn.manager_ptr;
            delete_ptr  = fn.delete_ptr;
            
            fn.manager_ptr = nullptr;
            fn.delete_ptr  = nullptr;
        }
        //Delete Manager
        ~FunctionBase(){
            reset();
        }
        //reset manager
        void reset(){
            if(delete_ptr != nullptr){
                //is Impl::ManagerBase
                auto base = static_cast<Impl::ManagerBase*>(manager_ptr);
                base->refcount -= 1;
                if(base->refcount == 0){
                    delete_ptr(manager_ptr);
                }
            }
            manager_ptr = nullptr;
            delete_ptr = nullptr;
        }
        //swap manager
        void swap(FunctionBase &fn){
            auto m = fn.manager_ptr;
            auto dptr = fn.delete_ptr;

            fn.manager_ptr = manager_ptr;
            fn.delete_ptr = delete_ptr;

            manager_ptr = m;
            delete_ptr  = dptr;
        }
        //copy manager
        void *manager_ptr;
        void (*delete_ptr)(void *manager);
    };


    template<class RetT>
    class Function;
    //Function
    template<class RetT,class ...Args>
    class Function<RetT(Args...)>:public FunctionBase{
        public:
            //empty function
            Function():
                FunctionBase(),
                invoke_ptr(nullptr)
                {};
            //create function
            template<class Fn>
            Function(Fn fn){
                assign_impl(std::forward<Fn>(fn));
            };
            //Copy function
            Function(const Function &fn):FunctionBase(fn){
                invoke_ptr = fn.invoke_ptr;
            }
            //move function
            Function(Function &&fn):FunctionBase(fn){
                invoke_ptr = fn.invoke_ptr;
                fn.invoke_ptr = nullptr;
            }
            //assign
            template<class Fn>
            void assign_impl(Fn &&fn){
                //typedef typename std::remove_reference<Fn>::type T;
                if constexpr(std::is_pointer<Fn>::value){
                    //C function pointer
                    delete_ptr = nullptr;
                    manager_ptr = reinterpret_cast<void*>(fn);
                    invoke_ptr = InvokeCFunction;
                }
                else{
                    //other any callable
                    typedef Impl::Manager<Fn,RetT,Args...> Manager;
                    manager_ptr = new Manager(std::forward<Fn>(fn));
                    delete_ptr = Manager::Delete;
                    invoke_ptr = Manager::Invoke;
                }
            };
            //assign other callable
            template<class Fn>
            void assign(Fn &&fn){
                //reset function
                reset();
                //assign
                assign_impl(std::forward<Fn>(fn));
            };
            void assign(Function &&fn){
                if(this != &fn){
                    swap(fn);
                }
            };
            void assign(Function &fn){
                if(this != &fn){
                    //clear it
                    reset();
                    manager_ptr = fn.manager_ptr;
                    delete_ptr  = fn.delete_ptr;
                    if(delete_ptr != nullptr){
                        //has refcount
                        auto base = static_cast<Impl::ManagerBase*>(manager_ptr);
                        base->refcount += 1;
                    }
                }
            };
            //reset function
            void reset(){
                FunctionBase::reset();
                invoke_ptr = nullptr;
            };
            //swap function
            void swap(Function &fn){
                FunctionBase::swap(fn);
                auto invp = fn.invoke_ptr;
                fn.invoke_ptr = invoke_ptr;
                invoke_ptr = invp;
            };
            //assign operator
            template<class T>
            Function &operator =(T &&dat){
                assign<T>(std::forward<T>(dat));
                return *this;
            }
            //result_type
            typedef RetT result_type;
            //invoke function pointer
            typedef RetT (*InvokeFn)(void*,Args...);
            result_type call(Args ...args) const{
                if(invoke_ptr == nullptr){
                    //handle err...
                    throwBadFunctionCall();
                }
                else{
                    return invoke_ptr(manager_ptr,std::forward<Args>(args)...);
                }
            };
            //is safe to call
            bool empty() const noexcept{
                return invoke_ptr == nullptr;
            };
            result_type operator()(Args ...args) const{
                return call(std::forward<Args>(args)...);
            };
            bool operator ==(std::nullptr_t) const noexcept{
                return invoke_ptr == nullptr;
            };
            operator bool() const noexcept{
                return empty();
            };
            //Invoke C Function
            static RetT InvokeCFunction(void *fn,Args ...args){
                typedef RetT(*fn_t)(Args...);
                return reinterpret_cast<fn_t>(fn)(
                    std::forward<Args>(args)...
                );
            };
        private:
            InvokeFn invoke_ptr;
    };
};
#endif // _BTKSIGNAL_FUNCTION_HPP_
