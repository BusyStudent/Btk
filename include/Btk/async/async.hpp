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
            using type = Signal<void(RetT &)>;
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
            union Data{
                //erase the destructor
                RetT data;
                bool data__unused;
                Data(){
                    data__unused = false;
                }
                ~Data(){}
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
            union Data{
                //erase the destructor
                std::remove_reference_t<RetT> data;
                bool data__unused;

                Data(){
                    data__unused = false;
                }
                ~Data(){}
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
         * @brief A RAII class for impl Invoker
         * 
         * @tparam HasSiganlArg Is The signal arg is not empty?
         * @tparam T The invoker type
         */
        template<bool HasSiganlArg,class T>
        struct AsyncGuard;

        template<class T>
        struct AsyncGuard<true,T>{
            //The invoker
            T *invoker;
            bool need_cleanup = true;
            bool need_delete = true;
            //< need We cleanup for the invoker
            AsyncGuard(T *i){
                invoker = i;
            }
            AsyncGuard(const AsyncGuard &) = delete;
            ~AsyncGuard(){
                if(need_cleanup){
                    invoker->cleanup();
                }
                if(need_delete){
                    delete invoker;
                }
            }
            /**
             * @brief Relase both
             * 
             */
            void release(){
                need_cleanup = false;
                need_delete = false;
            };
        };
        /**
         * @brief The specialization of no signal arg
         * 
         * @tparam T The invoker type
         */
        template<class T>
        struct AsyncGuard<false,T>{
            T *invoker;
            bool need_delete = true;
            //< need We cleanup for the invoker
            AsyncGuard(T *i){
                invoker = i;
            }
            AsyncGuard(const AsyncGuard &) = delete;
            ~AsyncGuard(){
                if(need_delete){
                    delete invoker;
                }
            }
            void release(){
                need_delete = false;
            };
        };
        template<class T>
        struct AsyncScopePtr{
            T *invoker;
            AsyncScopePtr(T *ptr){
                invoker = ptr;
            };
            AsyncScopePtr(const AsyncScopePtr &) = delete;
            ~AsyncScopePtr(){
                invoker->cleanup();
                delete invoker;
            };
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
             * @brief Is the signal has args
             * 
             */
            static constexpr bool signal_has_args = not std::is_same<result_type,void>();
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
                    AsyncGuard<signal_has_args,AsyncInvoker> guard(this);
                    invoke();
                    if(not this->signal.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,this);
                        //relase the guard
                        guard.release();
                    }
                }
                else{
                    AsyncGuard<signal_has_args,AsyncInvoker> guard(this);
                    //< We didnot have the value
                    //< So we set the flags to false
                    guard.need_cleanup = false;
                    this->store(invoke());
                    //< Ok,we store the value
                    guard.need_cleanup = true;
                    if(not this->signal.empty()){
                        //Emit this signal on main thread
                        DeferCall(EmitSignal,this);
                        guard.release();
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
                    std::unique_ptr<AsyncInvoker> ptr(this);
                    this->signal.emit();
                }
                else{
                    //Add Guard
                    AsyncScopePtr<AsyncInvoker> guard(this);

                    if constexpr(std::is_reference<result_type>()){
                        //Is ref
                        //So the Holder's value is a point
                        this->signal.emit(*(this->data.data));
                    }
                    else{
                        this->signal.emit(this->data.data);
                    }
                }
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
            /**
             * @brief a use full operator to connect signal
             * @note It only useable only it has signal
             * @return The signal or void*(NoSignal)
             */
            signal_type *operator ->() const noexcept{
                if constexpr(HasSignal){
                    return &(invoker->signal);
                }
                else{
                    return nullptr;
                }
            };
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
