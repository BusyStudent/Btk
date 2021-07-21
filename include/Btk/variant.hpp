#if !defined(_BTK_VARIANT_HPP_)
#define _BTK_VARIANT_HPP_
#include "defs.hpp"
#include "rect.hpp"
#include "pixels.hpp"
#include "string.hpp"
#include "exception.hpp"

#include <ostream>

//Use X Macros to generate the code
#define BTK_VARIANT_BODY \
    BTK_VARIANT_FILEDS(bool) \
    BTK_VARIANT_FILEDS(float) \
    BTK_VARIANT_FILEDS(double) \
    BTK_VARIANT_FILEDS(Uint16)\
    BTK_VARIANT_FILEDS(Uint32)\
    BTK_VARIANT_FILEDS(Uint64)\
    BTK_VARIANT_FILEDS(Sint16)\
    BTK_VARIANT_FILEDS(Sint32)\
    BTK_VARIANT_FILEDS(Sint64)\
    BTK_VARIANT_FILEDS(Rect)\
    BTK_VARIANT_FILEDS(FRect)\
    BTK_VARIANT_FILEDS(Point)\
    BTK_VARIANT_FILEDS(FPoint)\
    BTK_VARIANT_FILEDS(Color)\
    BTK_VARIANT_FILEDS(u8string)\
    BTK_VARIANT_FILEDS(u16string)\
    BTK_VARIANT_FILEDS(StringList)\
    BTK_VARIANT_FILEDS(StringRefList)\
    //



namespace Btk{
    class BTKAPI Variant{
        private:
            /**
             * @brief Helper for assert
             * 
             */
            struct _Assert;
        public:
            Variant() = default;
            template<class T>
            Variant(T &&val){
                emplace(std::forward<T>(val));
            }
            Variant(Variant &&v){
                swap(v);
            }
            /**
             * @brief Construct a new Variant object
             * 
             */
            Variant(const Variant &);
            ~Variant(){
                clear();
            }

            void clear();
            /**
             * @brief Swap with another variant
             * 
             */
            void swap(Variant &);
            bool empty() const noexcept{
                return type == Type::Unknown;
            }
            //Get / set

            /**
             * @brief Move assign(just swap)
             * 
             * @param var 
             * @return Variant& 
             */
            Variant &operator =(Variant &&var){
                if(this != &var){
                    swap(var);
                }
                return *this;
            }
            /**
             * @brief Assign
             * 
             * @return Variant& 
             */
            template<class T>
            Variant &operator =(T &&val){
                emplace(std::forward<T>(val));
                return *this;
            }
            Variant &operator =(const Variant &);

            template<class T>
            void emplace(T &&){
                static_assert(std::is_same_v<T,_Assert>,"Invaid type");
            }
            template<class T>
            void emplace(const T &){
                static_assert(std::is_same_v<T,_Assert>,"Invaid type");
            }
            /**
             * @brief Get the value
             * 
             * @tparam T 
             * @return T& 
             */
            template<class T>
            T &get(){
                T *ptr = get_if<T>();
                if(ptr == nullptr){
                    throwRuntimeError("Bad access");
                }
                return *ptr;
            }
            /**
             * @brief Get the value
             * 
             * @tparam T 
             * @return const T& 
             */
            template<class T>
            const T &get() const{
                const T *ptr = get_if<T>();
                if(ptr == nullptr){
                    throwRuntimeError("Bad access");
                }
                return *ptr;
            }
            /**
             * @brief Get the if object
             * 
             * @tparam T The type
             * @return T* Failed on nullptr
             */
            template<class T>
            T *get_if(){
                static_assert(std::is_same_v<T,_Assert>,"Invaid type");
            }

            template<class T>
            const T *get_if() const{
                static_assert(std::is_same_v<T,_Assert>,"Invaid type");
            }

            //Auto cast
            template<class T>
            operator T(){
                return get<T>();
            }

            //For init string
            void emplace(const char *str);

            bool is_same_type(const Variant &v){
                return type == v.type;
            }
            /**
             * @brief Check is the type
             * 
             * @tparam T 
             * @return true 
             * @return false 
             */
            template<class T>
            bool is_type() const{
                return get_if<T>() != nullptr;
            }
        private:
            union Any{
                Any(){}
                ~Any(){}
                //Make the body
                #define BTK_VARIANT_FILEDS(T) T _##T;
                BTK_VARIANT_BODY;
                #undef BTK_VARIANT_FILEDS
                int _var_unused;
            }data;
            enum class Type{
                //Make the type enum
                #define BTK_VARIANT_FILEDS(T) _##T,
                BTK_VARIANT_BODY
                #undef BTK_VARIANT_FILEDS
                //Default
                Unknown
            }type = Type::Unknown;
        friend BTKAPI std::ostream &operator <<(std::ostream &os,const Variant &);
    };
    //Make emplace functions and get
    #define BTK_VARIANT_FILEDS(X) \
        template<> \
        inline void Variant::emplace<X>(X &&v){\
            clear();\
            new(static_cast<void*>(&data)) X(std::forward<X>(v));\
            type = Type::_##X;\
        }\
        template<> \
        inline void Variant::emplace<X>(const X &v){\
            clear();\
            new(static_cast<void*>(&data)) X(v);\
            type = Type::_##X;\
        }\
        template<>\
        inline X * Variant::get_if<X>(){\
            if(type != Type::_##X){\
                return nullptr;\
            }\
            return &(data._##X);\
        }\
        template<>\
        inline const X * Variant::get_if<X>() const{\
            if(type != Type::_##X){\
                return nullptr;\
            }\
            return &(data._##X);\
        }
    BTK_VARIANT_BODY
    #undef BTK_VARIANT_FILEDS

    inline void Variant::emplace(const char* s){
        clear();
        new (static_cast<void*>(&data)) u8string(s);
        type = Type::_u8string;
    }
    //Avoid this
    //os << 1; => os << Variant(1)
    // #define BTK_MAKE_STDOP(T) \
    //     inline std::ostream &operator <<(std::ostream &os,T value){\
    //         os.operator <<(value);\
    //         return os;\
    //     }
    // BTK_MAKE_STDOP(bool);
    // BTK_MAKE_STDOP(float); 
    // BTK_MAKE_STDOP(double); 
    // BTK_MAKE_STDOP(Uint16);
    // BTK_MAKE_STDOP(Uint32);
    // BTK_MAKE_STDOP(Uint64);
    // BTK_MAKE_STDOP(Sint16);
    // BTK_MAKE_STDOP(Sint32);
    // BTK_MAKE_STDOP(Sint64);
    BTKAPI std::ostream &operator <<(std::ostream &os,const Variant &var);
    #undef BTK_MAKE_STDOP
}

#endif // _BTK_VARIANT_HPP_
