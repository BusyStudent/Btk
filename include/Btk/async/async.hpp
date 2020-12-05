#if !defined(BTK_ASYNC_HPP_)
#define BTK_ASYNC_HPP_
#include <tuple>
#include "../signal/signal.hpp"
#include "../defs.hpp"
namespace Btk{
    extern void DeferCall(void(* fn)(void*),void *data);
    namespace Impl{
        template<class RetT>
        struct AsyncSignal{
            using type = Signal<void(RetT)>;
        };
        template<>
        struct AsyncSignal<void>{
            using type = Signal<void()>;
        };
        //A struct to store task value
        template<class RetT>
        struct AsyncResultHolder{
            union{
                //erase the destructor
                RetT data;
                bool __unused = false;
            }data;
            void cleanup(){
                if constexpr(not std::is_pod<RetT>()){
                    data.data.~RetT();
                }
            };
            template<class T>
            void store(T &&t){
                new (&data.data)T(std::forward<T>(t));
            };
        };
        template<>
        struct AsyncResultHolder<void>{
            void cleanup(){

            };
        };
        /**
         * @brief The AsyncInvoker
         * 
         * @tparam Callable The callable type
         * @tparam Args The args types
         */
        template<class Callable,class ...Args>
        struct AsyncInvoker:public AsyncResultHolder<std::invoke_result_t<Callable,Args...>>{

            /**
             * @brief The result type
             * 
             */
            using result_type = std::invoke_result_t<Callable,Args...>;
            /**
             * @brief The signal 
             * 
             * @note It will be emit after exec
             */
            typename AsyncSignal<result_type>::type sig;
            Callable callable;
            std::tuple<Args...> args;
            
            //Create a invoker
            AsyncInvoker(Callable &&_callable,Args &&..._args):
                callable(std::forward<Callable>(_callable)),
                args{std::forward<Args>(args)...}{

            };
            /**
             * @brief The main entry for invoker
             * 
             * @param __self The invoker's ptr
             */
            static void Run(void *__self){
                auto *ptr = static_cast<AsyncInvoker*>(__self);

                if constexpr(std::is_same<result_type,void>()){
                    std::apply(ptr->callable,ptr->args);
                    if(not ptr->sig.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,ptr);
                    }
                    else{
                        //cleanup
                        delete ptr;
                    }
                }
                else{
                    ptr->store(std::apply(ptr->callable,ptr->args));
                    if(not ptr->sig.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,ptr);
                    }
                    else{
                        //Cleanup returned value
                        ptr->leanup();
                        delete ptr;
                    }
                }
            };
            /**
             * @brief Emit Signal in main thread
             * 
             * @param __self The invoker ptr
             */
            static void EmitSignal(void *__self){
                auto *ptr = static_cast<AsyncInvoker*>(__self);

                if constexpr(std::is_same<result_type,void>()){
                    ptr->sig.emit();
                }
                else{
                    ptr->sig.emit(ptr->data.data);
                    ptr->cleanup();
                }
                delete ptr;
            };
        };
        /**
         * @brief Lauch a AsyncInvoker right now
         * 
         * @param invoker the invoker pointer
         * @param run the invoker entry
         */
        BTKAPI void RtLauch(void *invoker,void(*run)(void*invoker));
        BTKAPI void DeferLauch(void *invoker,void(*run)(void*invoker));
    };
    enum class Lauch{
        Async = 0,
        Defered = 1
    };
    /**
     * @brief The AsyncTask
     * 
     * @tparam Callable The callable type
     * @tparam Args The args type
     */
    template<class Callable,class ...Args>
    class AsyncTask{
        public:
            using result_type = typename std::invoke_result_t<Callable,Args...>;
            using signal_type = typename Impl::AsyncSignal<result_type>::type;

            AsyncTask(Callable &&callable,Args&& ...args){
                invoker = new Impl::AsyncInvoker<Callable,Args...>(
                    std::forward<Callable>(callable),
                    std::forward<Args>(args)...
                );
            };
            AsyncTask(AsyncTask &&task){
                invoker = task.invoker;
                task.invoker = nullptr;
            };
            AsyncTask(const AsyncTask &) = delete;
            ~AsyncTask(){
                lauch();
            };
            void lauch(Lauch lauch = Lauch::Async){
                if(invoker != nullptr){
                    if(lauch == Lauch::Async){
                        Impl::RtLauch(invoker,Impl::AsyncInvoker<Callable,Args...>::Run);
                    }
                    else{
                        Impl::DeferLauch(invoker,Impl::AsyncInvoker<Callable,Args...>::Run);
                    }
                    invoker = nullptr;
                }
            };
            signal_type *operator ->() const noexcept{
                return &(invoker->sig);
            }
            signal_type &operator *() const noexcept{
                return (invoker->sig);
            }
        private:
            Impl::AsyncInvoker<Callable,Args...> *invoker;
    };
    template<class T,class ...Args>
    AsyncTask<T,Args...> Async(T &&callable,Args ...args){
        return AsyncTask<T,Args...>(
            std::forward<T>(callable),
            std::forward<Args>(args)...
        );
    };
    void AsyncInit();
    void AsyncQuit();
};


#endif // BTK_ASYNC_HPP_
