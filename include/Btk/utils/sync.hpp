#if !defined(_BTK_UTILS_SYNC_HPP_)
#define _BTK_UTILS_SYNC_HPP_
#include "../defs.hpp"
#include <atomic>
struct SDL_semaphore;
struct SDL_mutex;
struct SDL_cond;
namespace Btk{
    /**
     * @brief SpinLock
     * 
     */
    class BTKAPI SpinLock{
        public:
            SpinLock() = default;
            SpinLock(const SpinLock &) = default;
            ~SpinLock() = default;
            /**
             * @brief Is the SpinLock locked?
             * 
             * @return true 
             * @return false 
             */
            bool is_lock() const noexcept{
                return slock != 0;
            }
            void lock() noexcept;
            void unlock() noexcept;
            bool try_lock() noexcept;
        private:
            int slock = 0;
    };
    /**
     * @brief recursive spinlock
     * 
     */
    class BTKAPI RSpinLock{
        public:
            RSpinLock() = default;
            RSpinLock(const RSpinLock &) = default;
            ~RSpinLock() = default;

            void lock() noexcept;
            void unlock() noexcept;
            bool try_lock() noexcept;
        private:
            std::atomic<unsigned long> owner;
            std::atomic<bool> locked;
            std::atomic<int>  counts;
    };
    class BTKAPI Semaphore{
        public:
            /**
             * @brief Construct a new Semaphore object
             * 
             * @param value The initial value(default to 0)
             */
            Semaphore(Uint32 value = 0);
            Semaphore(const Semaphore &) = delete;
            Semaphore(Semaphore &&s){
                sem = s.sem;
                s.sem = nullptr;
            }
            ~Semaphore();
            /**
             * @brief Get current value
             * 
             * @return Uint32 
             */
            Uint32 value() const;
            
            void post();
            void wait();
        private:
            SDL_semaphore *sem;
    };
    /**
     * @brief Python like threading.event()
     * 
     */
    class BTKAPI SyncEvent{
        public:
            //TODO : we could use Win32 Event in windows platform
            SyncEvent();
            SyncEvent(const SyncEvent &) = delete;
            ~SyncEvent();

            bool is_set() const noexcept;
            //
            void set();
            void clear();
            //Wait false on timeout
            void wait();
            bool wait(Uint32 ms);
        private:
            //Protect isset
            mutable SpinLock it_lock;
            SDL_cond *cond = nullptr;
            SDL_mutex *mtx = nullptr;
            bool     isset = false;

    };
    using Sem = Semaphore;

}

#endif // _BTK_UTILS_SYNC_HPP_
