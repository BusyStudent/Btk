#if !defined(BTK_ASYNC_HPP_)
#define BTK_ASYNC_HPP_
#include <tuple>
#include <memory>
#include "../signal/signal.hpp"
#include "../defs.hpp"
namespace Btk{
    extern void DeferCall(void(* fn)(void*),void *data);
    namespace Impl{
        template<bool has_signal,class RetT>
        struct AsyncSignal;

        template<class RetT>
        struct AsyncSignal<true,RetT>{
            using type = Signal<void(RetT)>;
            type signal;//< The Signals
        };
        template<>
        struct AsyncSignal<true,void>{
            using type = Signal<void()>;
            type signal;
        };
        //Empty signal
        template<class RetT>
        struct AsyncSignal<false,RetT>{
            using type = void;
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
                if constexpr(not std::is_destructible<RetT>()){
                    data.data.~RetT();
                }
            };
            template<class T>
            void store(T &&t){
                new (&data.data)T(std::forward<T>(t));
            };
        };
        //A specialization for ref
        template<class RetT>
        struct AsyncResultHolder<RetT&>{
            union{
                //erase the destructor
                std::remove_reference_t<RetT> data;
                bool __unused = false;
            }data;
            void cleanup(){};
            
            template<class T>
            void store(T &&t){
                data.data = &t;
            };
        };
        template<>
        struct AsyncResultHolder<void>{
            void cleanup(){

            };
        };
        /**
         * @brief The async base,It store the return value and the signal
         * 
         * @tparam HasSignal The value should we enable signal
         * @tparam Callable 
         * @tparam Args 
         */
        template<bool HasSignal,class Callable,class ...Args>
        struct AsyncBase:
            public AsyncSignal<HasSignal,std::invoke_result_t<Callable,Args...>>,
                   AsyncResultHolder<std::invoke_result_t<Callable,Args...>>,
                   std::tuple<Args...>{
            public:
                //< The async task has signal
                static constexpr int has_signal  = HasSignal;
                /**
                 * @brief Construct a new Async Base object
                 * 
                 * @param args The args
                 */
                AsyncBase(Args &&...args):
                    std::tuple<Args...>(std::forward<Args>(args)...){};
        };
        /**
         * @brief The AsyncInvoker
         * 
         * @tparam Callable The callable type
         * @tparam Args The args types
         */
        template<bool HasSignal,class Callable,class ...Args>
        struct AsyncInvoker:public AsyncBase<HasSignal,Callable,Args...>{
            /**
             * @brief The signal 
             * 
             * @note It will be emit after exec
             */
            Callable callable;
            
            //Create a invoker
            AsyncInvoker(Callable &&_callable,Args &&..._args):
                AsyncBase<HasSignal,Callable,Args...>(std::forward<Args>(_args)...),
                callable(std::forward<Callable>(_callable)){

            };
            /**
             * @brief The result type
             * 
             */
            using result_type = std::invoke_result_t<Callable,Args...>;
            /**
             * @brief Invoke the callable
             * 
             * @return The callable's result
             */
            result_type invoke(){
                return std::apply(callable,
                                  static_cast<std::tuple<Args...>&&>(*this));
            }
            /**
             * @brief Run the invoker
             * 
             */
            void run(){
                if constexpr(not HasSignal){
                    //It doesnnot have signal ,just Invoke it self
                    std::unique_ptr<AsyncInvoker> ptr(this);
                    invoke();
                }
                else if constexpr(std::is_same<result_type,void>()){
                    invoke();
                    if(not this->signal.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,this);
                    }
                    else{
                        //cleanup
                        delete this;
                    }
                }
                else{
                    this->store(invoke());
                    if(not this->signal.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,this);
                    }
                    else{
                        //Cleanup returned value
                        this->cleanup();
                        delete this;
                    }
                }
            };
            /**
             * @brief Emit the signal impl
             * 
             */
            template<class T = std::enable_if<HasSignal>>
            void emit(){
                if constexpr(std::is_same<result_type,void>()){
                    this->signal.emit();
                }
                else{
                    if constexpr(std::is_reference<result_type>()){
                        //Is ref
                        //So the Holder's value is a point
                        this->signal.emit(*(this->data.data));
                    }
                    else{
                        this->signal.emit(this->data.data);
                    }
                    this->cleanup();
                }
                delete this;
            };
            /**
             * @brief The main entry for invoker
             * 
             * @param __self The invoker's ptr
             */
            static void Run(void *__self){
                static_cast<AsyncInvoker*>(__self)->run();
            };
            /**
             * @brief Emit Signal in main thread
             * 
             * @param __self The invoker ptr
             */
            template<class T = std::enable_if<HasSignal>>
            static void EmitSignal(void *__self){
                static_cast<AsyncInvoker*>(__self)->emit();
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
     * @brief A type to tell the async 
     *        don't emit the signal in main thread
     * 
     */
    struct NoSignal{

    };
    /**
     * @brief The AsyncTask
     * 
     * @tparam HasSignal If the task has signal
     * @tparam Callable The callable type
     * @tparam Args The args type
     */
    template<bool HasSignal,class Callable,class ...Args>
    class AsyncTask{
        public:
            using result_type = typename std::invoke_result_t<Callable,Args...>;
            using signal_type = typename Impl::AsyncSignal<HasSignal,result_type>::type;
            using invoker_type = typename Impl::AsyncInvoker<HasSignal,Callable,Args...>;
            //< The type of invoker
            AsyncTask(Callable &&callable,Args&& ...args){
                invoker = new invoker_type(
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
            /**
             * @brief Lauch the asyn task
             * 
             * @param lauch Lauch flag
             */
            void lauch(Lauch lauch = Lauch::Async){
                if(invoker != nullptr){
                    if(lauch == Lauch::Async){
                        Impl::RtLauch(invoker,invoker_type::Run);
                    }
                    else{
                        Impl::DeferLauch(invoker,invoker_type::Run);
                    }
                    invoker = nullptr;
                }
            };

            template<class T = std::enable_if<HasSignal,signal_type>>
            T *operator ->() const noexcept{
                return &(invoker->sig);
            }
            
            template<class T = std::enable_if<HasSignal,signal_type>>
            T &operator *() const noexcept{
                return (invoker->sig);
            }
        private:
            invoker_type *invoker;
    };
    /**
     * @brief Create a Async Task
     * 
     * @tparam T The callable type
     * @tparam Args The args type
     * @param callable The callable
     * @param args The args
     * @return AsyncTask object
     */
    template<class T,class ...Args>
    AsyncTask<true,T,Args...> Async(T &&callable,Args ...args){
        return AsyncTask<true,T,Args...>(
            std::forward<T>(callable),
            std::forward<Args>(args)...
        );
    };
    /**
     * @brief Create a Async Task(No signal)
     * 
     * @tparam T The callable type
     * @tparam Args The args type
     * @param nosignal A flag of nosignal
     * @param callable The callable
     * @param args The args
     * @return AsyncTask object(No signal)
     */
    template<class T,class ...Args>
    AsyncTask<false,T,Args...> Async(NoSignal,T &&callable,Args ...args){
        return AsyncTask<false,T,Args...>(
            std::forward<T>(callable),
            std::forward<Args>(args)...
        );
    };
    void AsyncInit();
    void AsyncQuit();
};


#endif // BTK_ASYNC_HPP_
