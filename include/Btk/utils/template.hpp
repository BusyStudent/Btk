#if !defined(_BTK_UTILS_TEMPLATE_HPP_)
#define _BTK_UTILS_TEMPLATE_HPP_
#include <memory>
#include <utility>
#include <cstddef>
#include "../defs.hpp"
/**
 * @brief It contains some useful template
 * 
 */
namespace Btk{
    struct adopt_lock_t{};
    inline constexpr auto adopt_lock = adopt_lock_t{};
    /**
     * @brief Lock Guard
     * 
     * @tparam T 
     */
    template<class T>
    class LockGuard{
        public:
            LockGuard(T &p):ptr(p){
                p.lock();
            }
            LockGuard(T &p,adopt_lock_t):ptr(p){}
            LockGuard(const LockGuard &) = delete;
            ~LockGuard(){
                ptr.unlock();
            }
        private:
            T &ptr;
    };
    template<class T>
    using lock_guard = LockGuard<T>;

    template<class T,class Deleter = std::default_delete<T>>
    using unique_ptr = std::unique_ptr<T,Deleter>;
    /**
     * @brief Base for RefPtr
     * 
     */
    class RefPtrBase{
        protected:

        bool has_refcount() const noexcept{
            return refcount != nullptr;
        }
        /**
         * @brief Get the refcount object
         * 
         * @return int 
         */
        int  get_refcount() const noexcept{
            if(has_refcount()){
                return *refcount;
            }
            return -1;
        }
        /**
         * @brief add ref
         * 
         */
        void ref() const noexcept{
            if(not has_refcount()){
                refcount = new int(1);
            }
            else{
                ++(*refcount);
            }
        }
        /**
         * @brief Ref the base
         * 
         * @param base 
         */
        void ref(const RefPtrBase &base){
            if(base.has_refcount()){
                refcount = base.refcount;
                ++(*refcount);
            }
            else{
                refcount = nullptr;
            }
        }
        /**
         * @brief de ref
         * 
         * @return true on We should delete the object
         * @return false 
         */
        bool unref() const noexcept{
            if(has_refcount()){
                --(*refcount);
                //We should cleanup
                if(*refcount == 0){
                    delete refcount;
                    refcount = nullptr;
                    return true;
                }
                refcount = nullptr;
            }
            return false;
        }
        mutable int *refcount = nullptr;
    };
    /**
     * @brief Ref Pointer
     * 
     * @tparam T 
     */
    template<class T>
    class RefPtr:protected RefPtrBase{
        public:
            explicit RefPtr(T *);
            RefPtr() = default;
            RefPtr(const RefPtr &);
            ~RefPtr(){
                reset();
            }
            /**
             * @brief Get the data
             * 
             * @return T* 
             */
            T *get() const noexcept{
                return data;
            }
            /**
             * @brief Get refcount
             * 
             * @return int 
             */
            int refcount() const noexcept{
                return this->get_refcount();
            }
            void reset(){
                //True on cleanup
                if(this->unref()){
                    delete data;
                }
            }
            T *operator ->() const noexcept{
                return get();
            }
            T &operator *() const noexcept{
                return *get();
            }

            RefPtr &operator =(const RefPtr &);
            RefPtr &operator =(std::nullptr_t);
            RefPtr &operator =(T *);
        private:
            T *data = nullptr;
    };
    template<class T>
    inline RefPtr<T>::RefPtr(const RefPtr &p){
        this->ref(p);
        data = p.data;
    }
    template<class T>
    inline RefPtr<T>::RefPtr(T *p){
        reset();
        data = p;
        if(p != nullptr){
            this->ref();
        }
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(T *ptr){
        reset();
        data = ptr;
        if(data != nullptr){
            //Create a new refcount when it is not nullptr
            this->ref();
        }
        return *this;
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(std::nullptr_t){
        reset();
        return *this;
    }
    template<class T>
    inline RefPtr<T>& RefPtr<T>::operator =(const RefPtr<T> &ptr){
        if(this == &ptr){
            return *this;
            
        }
        reset();
        this->ref(ptr);
        data = ptr.data;
        return *this;
    }
    /**
     * @brief Weak Pointer
     * 
     * @tparam T 
     */
    template<class T>
    class WeakPtr{

    };
    /**
     * @brief Holder for Object to erase the destructor
     * 
     * @tparam T 
     */
    template<class T>
    class ObjectHolder{
        public:
            ObjectHolder() = default;
            ~ObjectHolder() = default;
            /**
             * @brief Construct the object
             * 
             * @tparam Args 
             * @param args 
             */
            template<class ...Args>
            void construct(Args &&...args){
                new(reinterpret_cast<void*>(buffer)) T(
                    std::forward<Args>(args)...
                );
            }
            /**
             * @brief Destruct the object
             * 
             */
            void destroy(){
                if constexpr(std::is_destructible_v<T>){
                    get()->~T();
                }
            }
            /**
             * @brief Make the memory in 0
             * 
             */
            void zero(){
                for(size_t n = 0;n < sizeof(T);++n){
                    buffer[n] = 0;
                }
            }
            //Get 
            T *get() noexcept{
                return reinterpret_cast<T*>(buffer);
            }
            const T *get() const noexcept{
                return reinterpret_cast<const T*>(buffer);
            }
            T *operator ->() noexcept{
                return get();
            }
            T &operator *() noexcept{
                return *get();
            }
            const T *operator ->() const noexcept{
                return get();
            }
            const T &operator *() const noexcept{
                return *get();
            }
        private:
            alignas(T) Uint8 buffer[sizeof(T)];
    };

    struct nullopt_t{};
    inline constexpr auto nullopt = nullopt_t{};
    /**
     * @brief Optional value
     * 
     * @tparam T 
     */
    template<class T>
    class Optional{
        public:
            Optional() = default;
            Optional(const T &);
            Optional(T &&);
            Optional(nullopt_t):Optional(){}
            Optional(const Optional &);
            ~Optional(){
                reset();
            }

            bool has_value() const noexcept{
                return _has_value;
            }

            void reset(){
                if(has_value()){
                    buffer.destroy();
                    _has_value = false;
                }
            }
            template<class ...Args>
            void emplace(Args &&...args){
                reset();
                buffer.construct(std::forward<Args>(args)...);
                _has_value = true;
            }
            
            operator bool() const noexcept{
                return _has_value;
            }
            T *operator ->() noexcept{
                return buffer.get();
            }
            T &operator &() noexcept{
                return *buffer.get();
            }
            const T *operator ->() const noexcept{
                return buffer.get();
            }
            const T &operator &() const noexcept{
                return *buffer.get();
            }
        private:
            //< buffer contain the value
            ObjectHolder<T> buffer;
            bool _has_value = false;
    };
    template<class T>
    inline Optional<T>::Optional(const T &v){
        buffer.construct(std::forward<T>(v));
        _has_value = true;
    }
    template<class T>
    inline Optional<T>::Optional(T &&v){
        buffer.construct(std::forward<T>(v));
        _has_value = true;
    }

    template<class T>
    using shared_ptr = RefPtr<T>;

    template<class T>
    using optional = Optional<T>;
}

#endif // _BTK_UTILS_TEMPLATE_HPP_
