#include "./build.hpp"

#include <Btk/impl/thread.hpp>
#include <Btk/impl/atomic.hpp>
#include <Btk/async.hpp>
#include <Btk/Btk.hpp>

#include <condition_variable>
#include <exception>
#include <thread>
#include <queue>
#include <mutex>
#include <list>


namespace Btk{
    struct ThreadPool;
    struct Task;
    struct BTKHIDDEN Worker{
        //Create a worker
        Worker(ThreadPool *p):
            pool(p),
            thrd(&Btk::Worker::run,this){
            
        };
        //status for worker
        Atomic idle = true;//is idle
        Atomic running = true;
        //mutex for std::condvar
        std::mutex cond_mtx;

        ThreadPool *pool;
        Thread      thrd;
        

        void run();
        //It will get a task from queue
        bool wait_task(Task &task);
    };
    struct BTKHIDDEN Task{
        void *userdata;
        void(*entry)(void *userdata);
        void run(){
            entry(userdata);
        };
    };
    struct BTKHIDDEN ThreadPool{
        static ThreadPool *instance;

        std::list<Worker> workers_list;
        std::queue<Task>  tasks_queue;
        std::condition_variable condvar;
        SpinLock tasks_mtx;//SpinLock for tasks_queue
        Atomic idle_workers = 0;//IDLE workers count
        Atomic cur_workers = 1;//Current workers
        Atomic max_workers = 4;// Max Worker in the pool

        std::mutex workers_mtx;
        ThreadPool();
        ~ThreadPool();
        void run();
        void add_worker();

        //It will add a task
        void add_task(Task);
        //Add a task if the queue is empty
        bool tryadd_task(Task);
    };
    ThreadPool *ThreadPool::instance = nullptr;

    void AsyncInit(){
        if(ThreadPool::instance == nullptr){
            ThreadPool::instance = new ThreadPool();
        }
    };
    void AsyncQuit(){
        delete ThreadPool::instance;
        ThreadPool::instance = nullptr;
    };
};
namespace Btk{
    //Worker
    void Worker::run(){
        Task task;
        while(running){
            if(not wait_task(task)){
                //We cannot get a task
                idle = true;
                continue;
            }
            idle = false;
            pool->idle_workers -= 1;

            try{
                task.run();
            }
            catch(...){
                Btk::DeferCall(std::rethrow_exception,std::current_exception());
            }
            
            idle = true;
            pool->idle_workers += 1;
        }
    };
    bool Worker::wait_task(Task &task){
        {
            std::lock_guard locker(pool->tasks_mtx);
            if(not pool->tasks_queue.empty()){
                
                task = pool->tasks_queue.front();
                pool->tasks_queue.pop();
                return true;
            }
        }
        std::unique_lock<std::mutex> uniq_lock(cond_mtx);
        pool->condvar.wait(uniq_lock);
        //wait for task
        {
            //Task is getted
            std::lock_guard locker(pool->tasks_mtx);
            if(not pool->tasks_queue.empty()){
                
                task = pool->tasks_queue.front();
                pool->tasks_queue.pop();
                return true;
            }
            else{
                return false;
            }
        }
    }
};
namespace Btk{
    //ThreadPool
    ThreadPool::ThreadPool(){
        //Default to start a workers
        workers_list.emplace_back(this);
    }
    ThreadPool::~ThreadPool(){
        while(true){
            //sleep for 10 ms to check tasks_queue
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard locker(tasks_mtx);
            if(tasks_queue.empty()){
                break;
            }
        }
        //stop running workers
        for(auto &worker:workers_list){
            worker.running = false;
        }
        condvar.notify_all();
        //cleanup workers
        for(auto iter = workers_list.begin();iter != workers_list.end();){
            iter->thrd.join();
            iter = workers_list.erase(iter);
        }
    }
    //Push a task into queue
    void ThreadPool::add_task(Task t){
        //Try add worker
        add_worker();
        {
            std::lock_guard locker(tasks_mtx);
            tasks_queue.push(t);
        }
        condvar.notify_one();
    }
    //Push a task if the queue is empty
    bool ThreadPool::tryadd_task(Task t){
        //Try add worker
        add_worker();
        {
            std::lock_guard locker(tasks_mtx);
            if(not tasks_queue.empty()){
                return false;
            }
            tasks_queue.push(t);
        }
        condvar.notify_one();
        return true;
    }
    //Create a worker
    void ThreadPool::add_worker(){
        if(cur_workers >= max_workers){
            return;
        }
        std::lock_guard locker(workers_mtx);
        workers_list.emplace_back(this);
        cur_workers += 1;
    }
};
namespace Btk{
namespace Impl{
    //AsyncImpl
    void RtLauch(void *invoker,void(*invoker_main)(void*)){
        if(ThreadPool::instance != nullptr){
            //ThreadPool was init
            if(ThreadPool::instance->tryadd_task({
                invoker,
                invoker_main
            })){
                //ThreadPool is idle
                //So we use the thread in threadpool
                return ;
            }
        }
        Thread(invoker_main,invoker).detach();
    }
    void DeferLauch(void *invoker,void(*invoker_main)(void*)){
        BTK_ASSERT(ThreadPool::instance != nullptr);
        ThreadPool::instance->add_task({
            invoker,
            invoker_main
        });
    }
};
};