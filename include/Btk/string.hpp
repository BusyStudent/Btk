#if !defined(_BTK_STRING_HPP_)
#define _BTK_STRING_HPP_
#include <iosfwd>
#include <string>
#include <vector>
#include <string_view>
#include <type_traits>
#include <cstring>
#include <cstdarg>
#include "defs.hpp"

#ifdef _WIN32
    #include <ostream>
#endif


/**
 * @brief Macro to generate operator
 * 
 */
#define BTK_STRING_OPERATOR(TYPE,OP) \
    inline auto operator OP(const TYPE &t1,const TYPE &t2)\
        BTK_NOEXCEPT_IF(t1.base() OP t2.base()){\
        return t1.base() OP t2.base();\
    }

#ifdef BTK_VSCODE_SUPPRESS
    #define BTK_STRING_CONSTANT(NANE,VALUE) extern Btk::u8string_view NAME;
#else
    #define BTK_STRING_CONSTANT(NANE,VALUE) inline constexpr Btk::u8string_view NAME = VALUE;
#endif


namespace Btk{
BTKAPI void __sscanf_chk(int nargs,const char *s,const char *fmt,...);
namespace FmtImpl{
    //Format Helpers
    template<class ...Args>
    void __sscanf(const char *s,const char *fmt,Args &&...args){
        __sscanf_chk(sizeof...(Args),s,fmt,std::forward<Args>(args)...);
    }

    template<class T>
    struct Converter;

    struct IntConverter{
        template<class Type,class String> 
        static void load(
            String &ret,
            std::conditional_t<std::is_unsigned_v<Type>,
                unsigned long,
                long
            > value
        ){
            const char *fmt;
            if constexpr(std::is_unsigned_v<Type>){
                fmt = "%lu";
            }
            else{
                fmt = "%ld";
            }
            ret.append_fmt(fmt,value);
        }
        template<class Type,class View>
        static auto parse(const View&view){
            const char *fmt;
            
            std::conditional_t<std::is_unsigned_v<Type>,
                unsigned long,
                long
            > value {};
            
            if constexpr(std::is_unsigned_v<Type>){
                fmt = "%lu";
            }
            else{
                fmt = "%ld";
            }
            if constexpr(std::is_same_v<View,u8string>){ 
                __sscanf(view.c_str(),fmt,&value);
            }
            else{
                auto tmp = view.to_string();
                __sscanf(tmp.c_str(),fmt,&value); 
            }
            return value;
        }
    };
    struct FloatConverter{
        template<class Type,class String> 
        static void load(String &ret,double value){ 
            ret.append_fmt("%ld",value); 
        }
        template<class Type,class View>
        static double parse(const View&view){ 
            double value{};
            if constexpr(std::is_same_v<View,u8string>){ 
                __sscanf(view.c_str(),"%lf",&value);
            }
            else{
                auto tmp = view.to_string();
                __sscanf(tmp.c_str(),"%lf",&value); 
            }
            return value;
        }
    };


    #define BTK_FMT_CONVERTER(TYPE) \
    template<> \
    struct Converter<TYPE>{ \
        template<class String> \
        static void load(String &ret,TYPE value){ \
            if constexpr(std::is_integral_v<TYPE>){ \
                return IntConverter::load<TYPE>(ret,value);\
            }\
            else{\
                return FloatConverter::load<TYPE>(ret,value);\
            }\
        } \
        template<class View> \
        static TYPE parse(const View&view){ \
            if constexpr(std::is_integral_v<TYPE>){ \
                return IntConverter::parse<TYPE>(view);\
            }\
            else{\
                return FloatConverter::parse<TYPE>(view);\
            }\
        }\
    };
    BTK_FMT_CONVERTER(Uint8);
    BTK_FMT_CONVERTER(Uint16);
    BTK_FMT_CONVERTER(Uint32);
    BTK_FMT_CONVERTER(Uint64);

    BTK_FMT_CONVERTER(Sint64);
    BTK_FMT_CONVERTER(Sint32);
    BTK_FMT_CONVERTER(Sint16);
    BTK_FMT_CONVERTER(Sint8);

    BTK_FMT_CONVERTER(float);
    BTK_FMT_CONVERTER(double);
}
}

namespace Btk{
    #ifdef _WIN32

    #if defined(__GNUC__) && defined(_WIN32)
        #undef strncasecmp
        #undef strcasecmp
    #endif

    inline constexpr auto strncasecmp = ::_strnicmp;
    inline constexpr auto strcasecmp  = ::_stricmp;
    using wstring = u16string;
    #else
    inline constexpr auto strncasecmp = ::strncasecmp;
    inline constexpr auto strcasecmp = ::strcasecmp;
    using wstring = u32string;
    #endif

    inline constexpr auto CaseSensitive = false;
    inline constexpr auto CaseInSensitive = true;

    //Convert 

    /**
     * @brief Get len of a utf8 string
     * 
     * @param beg The string's begin
     * @param end The string's end(could be nullptr)
     * @return The string's size
     */
    BTKAPI size_t Utf8Strlen(const char *beg,const char *end = nullptr) noexcept;
    /**
     * @brief Encode a utf32 codepoint to utf8
     * 
     * @param codepoint The codepoint
     * @param buf The buffer to store the result(must be at least 6 bytes)
     * @return BTKAPI 
     */
    BTKAPI size_t Utf32Encode(char buf[6],char32_t codepoint) noexcept;
    BTKAPI size_t Utf32CharSize(char32_t codepoint) noexcept;
    /**
     * @brief Get size of a utf8 char
     * 
     * @param char 
     * @return size_t 
     */
    BTKAPI size_t Utf8CharSize(const char *s) noexcept;
    /**
     * @brief Move to the next char begin
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return The utf32 encoded codepoint
     */
    BTKAPI char32_t Utf8Next(const char *&) noexcept;
    /**
     * @brief Move to the prev char begin
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return The utf32 encoded codepoint
     */
    BTKAPI char32_t Utf8Prev(const char *&) noexcept;
    /**
     * @brief Get the utf32 codepoint
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return char32_t 
     */
    inline char32_t Utf8Peek(const char *c) noexcept{
        return Utf8Next(c);
    }
    inline char32_t Utf8Next(char *& c) noexcept{
        return Utf8Next(const_cast<const char*&>(c));
    }
    inline char32_t Utf8Prev(char *& c) noexcept{
        return Utf8Prev(const_cast<const char*&>(c));
    }
    template<class T>
    inline T Utf8GetPrev(T v) noexcept{
        Utf8Prev(v);
        return v;
    }
    template<class T>
    inline T Utf8GetNext(T v) noexcept{
        Utf8Next(v);
        return v;
    }
    /**
     * @brief Get distance of two utf8 char
     * 
     * @param p1 
     * @param p2 
     * @return ptrdiff_t 
     */
    inline ptrdiff_t Utf8Distance(const char *p1,const char *p2) noexcept{
        if(p1 > p2){
            return -Utf8Strlen(p2,p1);
        }
        else{
            return Utf8Strlen(p1,p2);
        }
    }
    /**
     * @brief Check the string is invaild utf8 string
     * 
     * @param beg The string's begin
     * @param end The string's end(could be nullptr)
     * @return true on invaild 
     * @return false on vaild 
     */
    BTKAPI bool Utf8IsVaild(const char *beg,const char *end = nullptr) noexcept;
    /**
     * @brief Advance in the string
     * 
     * @param beg The str begin
     * @param end The str end
     * @param cur current position
     * @param n The distance
     * @return pointer to the char (nullptr on out of range)
     */
    BTKAPI 
    const char *Utf8Advance(const char *beg,const char *end,const char *cur,long n) noexcept;
    //If failed,throw outof range
    BTKAPI 
    const char *Utf8AdvanceChecked(const char *beg,const char *end,const char *cur,long n);
    //Utf32
    // BTKAPI 
    // size_t      Utf32SequenceLength(char32_t ch);
    

    template<class Container>
    struct _Utf8RawCodepointRef{
        Container *container;
        char *current;

        //Replace char
        _Utf8RawCodepointRef& operator =(char32_t t){
            current = container->replace_char(current,t);
            return *this;
        }
        _Utf8RawCodepointRef& operator =(char16_t t){
            current = container->replace_char(current,t);
            return *this;
        }
        _Utf8RawCodepointRef& operator =(char t){
            current = container->replace_char(current,char16_t(t));
            return *this;
        }
        char *operator &() const noexcept{
            return current;
        }
        operator char32_t() const noexcept{
            return Utf8Peek(current);
        }
        /**
         * @brief Get the size of the codepoint
         * 
         * @return size_t 
         */
        size_t size() const{
            return Utf8CharSize(current);
        }
    };
    struct _Uft8ConstCodepoinRef{
        const char *current;
        const char *operator &() const noexcept{
            return current;
        }
        operator char32_t() const noexcept{
            return Utf8Peek(current);
        }
        /**
         * @brief Get the size of the codepoint
         * 
         * @return size_t 
         */
        size_t size() const{
            return Utf8CharSize(current);
        }
    };
    //Proxy for replace the char
    template<class Iterator,bool Const>
    struct _Utf8IteratorProxy;
    //No const
    template<class Iterator>
    struct _Utf8IteratorProxy<Iterator,false>{
        Iterator *iterator;
        operator char32_t() const noexcept{
            return Utf8Peek(iterator->current);
        }
        /**
         * @brief Get the size of the codepoint
         * 
         * @return size_t 
         */
        size_t size() const{
            return Utf8CharSize(iterator->current);
        }
        //Has replace
        void operator =(char32_t t){
            iterator->current = iterator->container->replace_char(iterator->current,t);
        }
        void operator =(char16_t t){
            iterator->current = iterator->container->replace_char(iterator->current,t);
        }
        void operator =(char t){
            iterator->current = iterator->container->replace_char(iterator->current,char16_t(t));
        }
    };
    //const
    template<class Iterator>
    struct _Utf8IteratorProxy<Iterator,true>{
        Iterator *iterator;
        operator char32_t() const noexcept{
            return Utf8Peek(iterator->current);
        }
        /**
         * @brief Get the size of the codepoint
         * 
         * @return size_t 
         */
        size_t size() const{
            return Utf8CharSize(iterator->current);
        }
        //No replace
    };

    template<class Container>
    struct _Utf8IteratorBase{
        using self_type = _Utf8IteratorBase<Container>;
        using container_type = Container;
        using reference_type = _Utf8IteratorProxy<self_type,std::is_const_v<Container>>;
        
        const Container *container = nullptr;
        const char *current = nullptr;

        void _move_prev(){
            Utf8Prev(current);
        }
        void _move_next(){
            Utf8Next(current);
        }
        void _advance(long n){
            current = Utf8AdvanceChecked(
                container->impl_begin(),
                container->impl_end(),
                current,
                n
            );
        }
        /**
         * @brief Get the size of the codepoint
         * 
         * @return size_t 
         */
        size_t size() const{
            return Utf8CharSize(current);
        }
        reference_type get_reference(){
            return {this};
        }
        //cmp
        bool operator ==(const _Utf8IteratorBase &i) const noexcept{
            return current == i.current;
        }
        bool operator !=(const _Utf8IteratorBase &i) const noexcept{
            return current != i.current;
        }
        // -
        ptrdiff_t operator -(const _Utf8IteratorBase &i) const{
            return Utf8Distance(i.current,current);
        }
    };
    template<class Container>
    struct _Utf8Iterator:public _Utf8IteratorBase<Container>{
        //Default operators
        _Utf8Iterator() = default;
        _Utf8Iterator(const _Utf8Iterator &) = default;
        ~_Utf8Iterator() = default;

        _Utf8Iterator(Container *c,const char *cur){
            this->container = c;
            this->current = cur;
        }

        _Utf8Iterator &operator =(const _Utf8Iterator &) = default;
        _Utf8Iterator &operator ++(){
            this->_move_next();
            return *this;
        }
        _Utf8Iterator &operator --(){
            this->_move_prev();
            return *this;
        }
        _Utf8Iterator  operator ++(int){
            auto tmp = *this;
            this->_move_next();
            return tmp;
        }
        _Utf8Iterator  operator --(int){
            auto tmp = *this;
            this->_move_prev();
            return tmp;
        }
        //Advance
        _Utf8Iterator &operator +=(long n){
            this->_advance(n);
            return *this;
        }
        _Utf8Iterator &operator -=(long n){
            this->_advance(- n);
            return *this;
        }
        _Utf8Iterator operator +(long n){
            auto tmp = *this;
            tmp._advance(n);
            return tmp;
        }
        _Utf8Iterator operator -(long n){
            auto tmp = *this;
            tmp._advance(- n);
            return tmp;
        }
        ptrdiff_t operator -(const _Utf8Iterator &i) const{
            return Utf8Distance(i.current,this->current);
        }

        auto operator *(){
            return this->get_reference();
        }
    };
    template<class Container>
    struct _Utf8ConstIterator:public _Utf8IteratorBase<const Container>{
        //Default operators
        _Utf8ConstIterator() = default;
        _Utf8ConstIterator(const _Utf8ConstIterator &) = default;
        ~_Utf8ConstIterator() = default;

        _Utf8ConstIterator(const Container *c,const char *cur){
            this->container = c;
            this->current = cur;
        }

        _Utf8ConstIterator &operator =(const _Utf8ConstIterator &) = default;
        _Utf8ConstIterator &operator ++(){
            this->_move_next();
            return *this;
        }
        _Utf8ConstIterator &operator --(){
            this->_move_prev();
            return *this;
        }
        _Utf8ConstIterator  operator ++(int){
            auto tmp = *this;
            this->_move_next();
            return tmp;
        }
        _Utf8ConstIterator  operator --(int){
            auto tmp = *this;
            this->_move_prev();
            return tmp;
        }
        //Advance
        _Utf8ConstIterator &operator +=(long n){
            this->_advance(n);
            return *this;
        }
        _Utf8ConstIterator &operator -=(long n){
            this->_advance(- n);
            return *this;
        }
        _Utf8ConstIterator operator +(long n){
            auto tmp = *this;
            tmp._advance(n);
            return tmp;
        }
        _Utf8ConstIterator operator -(long n){
            auto tmp = *this;
            tmp._advance(- n);
            return tmp;
        }
        ptrdiff_t operator -(const _Utf8ConstIterator &i) const{
            return Utf8Distance(i.current,this->current);
        }
        auto operator *(){
            return this->get_reference();
        }
    };
    //reverse_iterator
    template<class Container>
    struct _Utf8ReverseIterator:public _Utf8IteratorBase<Container>{
        _Utf8ReverseIterator() = default;
        _Utf8ReverseIterator(const _Utf8ReverseIterator &) = default;
        ~_Utf8ReverseIterator() = default;

        _Utf8ReverseIterator(const Container *c,const char *cur){
            this->container = c;
            this->current = cur;
        }
        
        _Utf8ReverseIterator &operator =(const _Utf8ReverseIterator &) = default;
        _Utf8ReverseIterator &operator ++(){
            this->_move_prev();
            return *this;
        }
        _Utf8ReverseIterator &operator --(){
            this->_move_next();
            return *this;
        }
        _Utf8ReverseIterator  operator ++(int){
            auto tmp = *this;
            this->_move_prev();
            return tmp;
        }
        _Utf8ReverseIterator  operator --(int){
            auto tmp = *this;
            this->_move_next();
            return tmp;
        }
        //Advance
        _Utf8ReverseIterator operator +(long n){
            auto tmp = *this;
            tmp._advance(- n);
            return tmp;
        }
        _Utf8ReverseIterator operator -(long n){
            auto tmp = *this;
            tmp._advance(n);
            return tmp;
        }
        ptrdiff_t operator -(const _Utf8ReverseIterator &i) const{
            return Utf8Distance(i.current,this->current);
        }


        auto operator *(){
            return this->get_reference();
        }
    };
    template<class Container>
    struct _Utf8ConstReverseIterator:public _Utf8IteratorBase<const Container>{

    };

    class BTKAPI u8string_view:protected std::string_view{
        public:
            u8string_view() = default;
            u8string_view(std::string_view);
            u8string_view(const std::string &);
            u8string_view(const u8string &);
            u8string_view(const u8string_view &) = default;
            
            using std::string_view::basic_string_view;
            using std::string_view::empty;
            using std::string_view::data;
            using std::string_view::size;
            using std::string_view::npos;

            using Base = std::string_view;
            using List = std::vector<u8string>;
            using RefList = std::vector<u8string_view>;

            using value_type = char32_t;
            using reference = _Uft8ConstCodepoinRef;
            using const_reference = _Uft8ConstCodepoinRef;

            using _iterator = const char *;
            using _const_iterator = const char *;

            using iterator = _Utf8ConstIterator<u8string_view>;
            using const_iterator = _Utf8ConstIterator<u8string_view>;

            u8string_view(const_iterator beg,const_iterator end);
            /**
             * @brief Get the length of the string_view
             * 
             * @return size_t 
             */
            size_t length() const noexcept{
                return Utf8Strlen(impl_begin(),impl_end());
            }
            size_t raw_length() const noexcept{
                return std::string_view::length();
            }
            bool is_vaild() const{
                return Utf8IsVaild(impl_begin(),impl_end());
            }

            std::string_view &base() noexcept{
                return *this;
            }
            const std::string_view &base() const noexcept{
                return *this;
            }
            operator std::string_view() const{
                return *this;
            }
            /**
             * @brief find a first char of
             * 
             * @return size_t 
             */
            size_t find(char32_t) const;
            size_t find(u8string_view) const;
            size_t rfind(char32_t) const;
            size_t rfind(u8string_view) const;

            u16string   to_utf16() const;
            /**
             * @brief convert to locale encoding
             * 
             * @return std::string 
             */
            std::string to_locale() const;
            u8string toupper() const;
            u8string tolower() const;
            u8string tobase64() const;

            u8string lstrip() const;
            u8string rstrip() const;
            /**
             * @brief Strip the space at begin and end
             * 
             * @return u8string 
             */
            u8string strip() const;
            u8string trim() const;
            /**
             * @brief Copy into string
             * 
             * @return u8string 
             */
            u8string to_string() const;

            //Parse
            template<class T>
            T parse() const{
                FmtImpl::Converter<T> cvt;
                return cvt.parse(*this);
            }


            //Check
            bool isalpha() const;
            bool isvalid() const{
                return is_vaild();
            }

            /**
             * @brief Cmp string(ignore the case)
             * 
             * @return true 
             * @return false 
             */
            bool casecmp(u8string_view view) const{
                return compare(view,CaseInSensitive);
            }
            /**
             * @brief Create a sub string_view
             * 
             * @param pos The substring begin
             * @param len The substring len
             * @return u8string_view 
             */
            u8string_view substr(size_t pos = 0,size_t len = npos) const;

            template<class T>
            u8string_view substr(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end) const{
                return u8string_view(
                    _translate_pointer(beg.current),
                    Utf8GetNext(end->current) - beg.current
                );
            }
            /**
             * @brief Split string and copy them into buffer
             * 
             * @param delim 
             * @param max Max substring(default unlimited)
             * @return List 
             */
            List    split(u8string_view delim,size_t max = size_t(-1)) const;
            List    split(char delim,size_t max = size_t(-1)) const{
                return split(u8string_view(&delim,1),max);
            }
            List    split(char32_t ch,size_t max = size_t(-1)) const;
            List    split(char16_t ch,size_t max = size_t(-1)) const{
                return split(char32_t(ch),max);
            }
            /**
             * @brief Split string and push the ref into buffer
             * 
             * @param delim 
             * @param max 
             * @return RefList 
             */
            RefList split_ref(u8string_view delim,size_t max = size_t(-1)) const;
            RefList split_ref(char delim,size_t max = size_t(-1)) const{
                return split_ref(u8string_view(&delim,1),max);
            }
            RefList split_ref(char32_t ch,size_t max = size_t(-1)) const;
            RefList split_ref(char16_t ch,size_t max = size_t(-1)) const{
                return split_ref(char32_t(ch),max);
            }
            /**
             * @brief Check the string is begin with
             * 
             * @param text 
             * @return true 
             * @return false 
             */
            bool begin_with(u8string_view text) const{
                if(text.raw_length() > raw_length()){
                    //Is longger
                    return false;
                }
                return base().substr(0,text.raw_length()) == text.base();
            }
            bool end_with(u8string_view text) const{
                if(text.raw_length() > raw_length()){
                    //Is longger
                    return false;
                }
                return base().substr(raw_length() - text.raw_length(),text.raw_length()) == text.base();
            }
            /**
             * @brief Raw Substr
             * 
             * @tparam Args 
             * @param args 
             * @return u8string_view 
             */
            template<class ...Args>
            u8string_view raw_substr(Args &&...args) const{
                return base().substr(std::forward<Args>(args)...);
            }
            bool compare(u8string_view view,bool casecmp = CaseSensitive) const noexcept{
                //Length is diffent
                if(raw_length() != view.raw_length()){
                    return false;
                }
                if(casecmp == CaseSensitive){
                    return strncmp(data(),view.data(),raw_length());
                }
                else{
                    return strncasecmp(data(),view.data(),raw_length());
                }
            }
            //Index method
            reference       at(size_t index){
                return static_cast<const u8string_view*>(this)->at(index);
            }
            const_reference at(size_t index) const;

            reference       operator [](size_t index){
                return static_cast<const u8string_view*>(this)->at(index);
            }
            const_reference operator [](size_t index) const{
                return static_cast<const u8string_view*>(this)->at(index);
            }
            //Iterator
            iterator begin(){
                auto beg = impl_begin();
                return {
                    this,
                    beg,
                };
            }
            iterator end(){
                auto beg = impl_end();
                return {
                    this,
                    beg,
                };
            }
            const_iterator begin() const{
                auto beg = impl_begin();
                return {
                    this,
                    beg,
                };
            }
            const_iterator end() const{
                auto beg = impl_end();
                return {
                    this,
                    beg,
                };
            }
            //Reference
            reference front(){
                return {
                    impl_begin()
                };
            }
            const_reference front() const{
                return {
                    impl_begin()
                };
            }
            reference back(){
                return {
                    Utf8GetPrev(impl_end())
                };
            }
            const_reference back() const{
                return {
                    Utf8GetPrev(impl_end())
                };
            }
            //Auto cast
            operator std::string() const{
                return std::string(data(),size());
            }

        private:
            _iterator impl_begin(){
                return _translate_iterator(Base::begin());
            }
            _iterator impl_end(){
                return _translate_iterator(Base::end());
            }
            _const_iterator impl_begin() const{
                return _translate_iterator(Base::begin());
            }
            _const_iterator impl_end() const{
                return _translate_iterator(Base::end());
            }
            //Translate
            Base::iterator _translate_pointer(char *ch){
                return Base::begin() + (ch - &*Base::begin());
            }
            Base::const_iterator _translate_pointer(const char *ch) const{
                return Base::begin() + (ch - &*Base::begin());
            }
            const char *_translate_iterator(Base::iterator i){
                return &*i;
            }
            const char *_translate_iterator(Base::const_iterator i) const{
                return &*i;
            }
        template<class T>
        friend struct std::hash;
        friend class u8string;
        friend std::ostream &operator <<(std::ostream &,u8string_view);
        template<class _T>
        friend struct _Utf8IteratorBase;
        template<class _T>
        friend struct _Utf8RawCodepointRef;
    };
    class BTKAPI u16string_view:protected std::u16string_view{
        public:
            u16string_view(std::u16string_view v);
            u16string_view(const u16string &);
            u16string_view(const std::u16string &);
            using std::u16string_view::basic_string_view;
            using std::u16string_view::empty;
            using std::u16string_view::data;

            #ifdef _WIN32
            u16string_view(const std::wstring &);
            u16string_view(std::wstring_view);
            u16string_view(const wchar_t *ws);
            u16string_view(const wchar_t *ws,size_t n);
            #endif

            std::u16string_view &base(){
                return *this;
            }
            const std::u16string_view &base() const{
                return *this;
            }
            u8string to_utf8() const;
        
        template<class T>
        friend struct std::hash;
        friend class u16string;
    };
    /**
     * @brief UTF8 String
     * 
     */
    class BTKAPI u8string{
        public:
            u8string();
            u8string(std::string &&);
            u8string(const std::string &);
            u8string(u8string_view view);
            u8string(std::string_view view);
            u8string(const char *);
            u8string(const Uint8 *);
            u8string(const char *,size_t n);
            u8string(const Uint8 *,size_t n);
            u8string(const u8string &);
            u8string(u8string &&) = default;
            ~u8string();


            //stl
            using iterator = _Utf8Iterator<u8string>;
            using const_iterator = _Utf8ConstIterator<u8string>;
            using reverse_iterator = _Utf8ReverseIterator<u8string>;


            u8string(const_iterator beg,const_iterator end);

            using value_type = char32_t;
            using reference = _Utf8RawCodepointRef<u8string>;
            using const_reference = _Uft8ConstCodepoinRef;
            using pointer = char32_t*;

            //Impl
            using _iterator = char *;
            using _const_iterator = const char *;

            using Base = std::string;
            using List = std::vector<u8string>;
            using RefList = std::vector<u8string_view>;

            using size_type = size_t;
            //Const
            inline static constexpr size_t npos = size_t(-1);

            /**
             * @brief The string doesnnot has invaild char
             * 
             * @return true 
             * @return false 
             */
            bool is_vaild() const{
                return Utf8IsVaild(impl_begin(),impl_end());
            }
            size_t length() const{
                return Utf8Strlen(impl_begin(),impl_end());
            }
            size_t raw_length() const{
                return base().length();
            }
            /**
             * @brief Convert to utf16
             * 
             * @return u16string 
             */
            u16string to_utf16() const;
            /**
             * @brief Pop the last char
             * 
             */
            void pop_back();
            void push_back(char32_t ch);
            void push_back(char16_t ch){
                push_back(char32_t(ch));
            }
            void push_back(char ch){
                base().push_back(ch);
            }
            /**
             * @brief Note it is a slow operation
             * 
             */
            void pop_front();
            /**
             * @brief Erase a ch
             * 
             * @tparam T 
             * @param iter 
             */
            template<class T>
            void erase(const _Utf8IteratorBase<T> iter){
                base().erase(
                    _translate_pointer(iter.current),
                    _translate_pointer(Utf8GetNext(iter.current))
                );
            }
            /**
             * @brief Erase a scope
             * 
             * @tparam T 
             * @param beg 
             * @param end 
             */
            template<class T>
            void erase(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end){
                base().erase(
                    _translate_pointer(beg.current),
                    _translate_pointer(end.current)
                );
            }
            //Index version
            void erase(size_t idx){
                erase(
                    begin() + idx
                );
            }
            void erase(size_t idx,size_t n);
            template<class T>
            void insert(const _Utf8IteratorBase<T> iter,const char *s,size_t n){
                base().insert(
                    _translate_pointer(iter.current),
                    s,
                    n
                );
            }
            template<class T>
            void insert(const _Utf8IteratorBase<T> iter,const char *s){
                base().insert(
                    _translate_pointer(iter.current),
                    s
                );
            }
            template<class T>
            void insert(const _Utf8IteratorBase<T> iter,char32_t ch){
                char buf[6];
                auto n = Utf32Encode(buf,ch);
                base().insert(
                    _translate_pointer(iter.current),
                    buf,
                    n
                );
            }
            //Index version of insert
            void insert(size_t pos,char32_t ch){
                char buf[6];
                auto n = Utf32Encode(buf,ch);
                base().insert(
                    raw_index_of(pos),
                    buf,
                    n
                );
            }
            void insert(size_t pos,const char *s,size_t n){
                base().insert(
                    raw_index_of(pos),
                    s,
                    n
                );
            }
            void insert(size_t pos,const char *s){
                base().insert(
                    raw_index_of(pos),
                    s
                );
            }
            void insert(size_t pos,u8string_view view){
                insert(pos,view.data(),view.size());
            }
            //Replace
            template<class T>
            void replace(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end,char32_t s){
                char buf[6];
                auto n = Utf32Encode(buf,s);
                base().replace(
                    _translate_pointer(beg.current),
                    _translate_pointer(end.current),
                    buf,
                    n
                );
            }
            template<class T>
            void replace(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end,const char *s,size_t n){
                base().replace(
                    _translate_pointer(beg.current),
                    _translate_pointer(end.current),
                    s,
                    n
                );
            }
            template<class T>
            void replace(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end,const char *s){
                base().replace(
                    _translate_pointer(beg.current),
                    _translate_pointer(end.current),
                    s
                );
            }
            template<class T>
            void replace(const _Utf8IteratorBase<T> beg,const _Utf8IteratorBase<T> end,u8string_view view){
                replace(beg,end,view.data(),view.size());
            }
            //Position version of replace
            void replace(size_t pos,size_t n,u8string_view view){
                //Convert pos and n to iterator
                iterator iter = begin();
                iter += pos;
                iterator end = (n == npos ? this->end() : iter + n);
                replace(iter,end,view);
            }
            void replace(size_t pos,size_t n,const char *s,size_t m){
                //Convert pos and n to iterator
                iterator iter = begin();
                iter += pos;
                iterator end = (n == npos ? this->end() : iter + n);
                replace(iter,end,s,m);
            }
            void replace(size_t pos,size_t n,const char *s){
                //Convert pos and n to iterator
                iterator iter = begin();
                iter += pos;
                iterator end = (n == npos ? this->end() : iter + n);
                replace(iter,end,s);
            }
            void replace(size_t pos,size_t n,char32_t ch){
                //Convert pos and n to iterator
                iterator iter = begin();
                iter += pos;
                iterator end = (n == npos ? this->end() : iter + n);
                replace(iter,end,ch);
            }
            void replace(size_t pos,u8string_view v){
                replace(pos,v.size(),v);
            }
            void replace(size_t pos,char32_t ch){
                char buf[6];
                auto n = Utf32Encode(buf,ch);
                base().replace(
                    raw_index_of(pos),
                    raw_index_of(pos) + n,
                    buf,
                    n
                );
            }

            template<class ...Args>
            void assign(Args &&...args){
                base().assign(std::forward<Args>(args)...);
            }

            /**
             * @brief Locate the char 
             * 
             * @param index 
             * @return _U8Proxy 
             */
            reference       at(size_type index);
            const_reference at(size_type index) const;
            /**
             * @brief Encode to
             * 
             * @param to 
             * @return std::string 
             */
            std::string encoode(const char *to) const;
            std::string to_locale() const{
                return u8string_view(*this).to_locale();
            }

            void append_from(const void *buf,size_t n,const char *encoding);
            /**
             * @brief Append string by formatting
             * 
             * @param fmt 
             * @param varg 
             */
            void append_vfmt(const char *fmt,std::va_list varg);
            void append_fmt(const char *fmt,...);

            reference       operator [](size_type index){
                return at(index);
            }
            const_reference operator [](size_type index) const{
                return at(index);
            }

            reference front(){
                return {
                    this,
                    impl_begin()
                };
            }
            const_reference front() const{
                return {
                    impl_begin()
                };
            }

            reference back(){
                return {
                    this,
                    Utf8GetPrev(impl_end())
                };
            }
            const_reference back() const{
                return {
                    Utf8GetPrev(impl_end())
                };
            }

            size_type find(char c) const{
                return find(char32_t(c));
            }
            size_type find(char32_t ch) const{
                return u8string_view(*this).find(ch);
            }
            size_type find(u8string_view v){
                return u8string_view(*this).find(v);
            }
            List    split(u8string_view delim,size_t max = size_t(-1)){
                return u8string_view(*this).split(delim,max);
            }
            RefList split_ref(u8string_view delim,size_t max = size_t(-1)){
                return u8string_view(*this).split_ref(delim,max);
            }

            u8string &operator =(const u8string &) = default;
            u8string &operator =(u8string &&) = default;
            u8string &operator =(u8string_view v){
                base() = v.base();
                return *this;
            }

            template<class T>
            u8string &operator +=(T &&arg){
                base() += std::forward<T>(arg);
                return *this;
            }
            u8string &operator +=(u8string_view view){
                base() += view.base();
                return *this;
            }

            void swap(u8string &us){
                base().swap(us.base());
            }
            //Append
            void append(const u8string &s){
                base().append(s.base());
            }
            void append(u8string_view view){
                base().append(view.base());
            }
            void append(const char *s){
                base().append(s);
            }
            void append(const char *s,size_t n){
                base().append(s,n);
            }
            //Prepend
            void prepend(const u8string &s){
                base().insert(0,s.base());
            }
            void prepend(u8string_view view){
                base().insert(0,view.base());
            }
            void prepend(const char *s){
                base().insert(0,s);
            }
            void prepend(const char *s,size_t n){
                base().insert(0,s,n);
            }
            //CSTR data
            const char *c_str() const noexcept{
                return base().c_str();
            }
            char *data() noexcept{
                return base().data();
            }
            //Empty
            bool empty() const noexcept{
                return base().empty();
            }
            void clear(){
                base().clear();
            }
            //Size
            void resize(size_t n){
                base().resize(n);
            }
            size_t size() const{
                return base().size();
            }
            size_t capacity() const{
                return base().capacity();
            }
            void shrink_to_fit(){
                base().shrink_to_fit();
            }
            u8string_view view() const{
                return u8string_view(*this);
            }
            //Index
            /**
             * @brief Convert a raw index in utf8 index
             * 
             * @param idx 
             * @return size_t 
             */
            size_t index_of(size_t idx){
                return index_of(c_str() + idx);
            }
            size_t index_of(const char *s);
            /**
             * @brief Convert utf8 index into raw index
             * 
             * @param idx 
             * @return size_t 
             */
            size_t raw_index_of(size_t idx) const;
            /**
             * @brief Create string from
             * 
             * @param buf buffer
             * @param n buffer size
             * @param encoding the string encoding(default utf8)
             * @return u8string 
             */
            static u8string from(const void *,size_t n,const char *encoding = nullptr);
            static u8string fromfile(const char *filename);

            static u8string from_utf16(u16string_view view){
                return view.to_utf8();
            }
        public:
            //beg and end
            iterator begin(){
                auto beg = impl_begin();
                return {
                    this,
                    beg,
                };
            }
            iterator end(){
                auto beg = impl_end();
                return {
                    this,
                    beg,
                };
            }
            const_iterator begin() const{
                auto beg = impl_begin();
                return {
                    this,
                    beg,
                };
            }
            const_iterator end() const{
                auto beg = impl_end();
                return {
                    this,
                    beg,
                };
            }
        public:
            operator std::string_view() const noexcept{
                return std::string_view(base().c_str(),base().length());
            }
            operator const std::string &() const noexcept{
                return base();
            }
            //FIXME:C2664 If we donnot ocomit it
            // operator u8string_view() const noexcept{
            //     return u8string_view(base().c_str(),base().length());
            // }
            std::string *operator ->() noexcept{
                return &base();
            }
            const std::string *operator ->() const noexcept{
                return &base();
            }
            const std::string &operator *() const noexcept{
                return base();
            }
            u8string &operator =(const char *s){
                base() = s;
                return *this;
            }

            std::string &get(){
                return base();
            }
            const std::string &get() const{
                return base();
            }
            std::string &base(){
                return _data;
            }
            const std::string &base() const{
                return _data;
            }

            u8string toupper() const{
                return u8string_view(*this).toupper();
            }
            u8string tolower() const{
                return u8string_view(*this).tolower();
            }
            u8string substr(size_t pos = 0,size_t len = npos) const{
                return u8string_view(*this).substr(pos,len);
            }
            bool casecmp(u8string_view v) const{
                return u8string_view(*this).casecmp(v);
            }
        private:
            _iterator impl_begin(){
                return _translate_iterator(base().begin());
            }
            _iterator impl_end(){
                return _translate_iterator(base().end());
            }
            _const_iterator impl_begin() const{
                return _translate_iterator(base().begin());
            }
            _const_iterator impl_end() const{
                return _translate_iterator(base().end());
            }
            //Translate
            Base::iterator _translate_pointer(char *ch){
                return base().begin() + (ch - &*base().begin());
            }
            Base::const_iterator _translate_pointer(const char *ch) const{
                return base().begin() + (ch - &*base().begin());
            }
            char *_translate_iterator(Base::iterator i){
                return &*i;
            }
            const char *_translate_iterator(Base::const_iterator i) const{
                return &*i;
            }
        private:
            char *replace_char(
                char *where,
                char32_t codepoint
            );
            char *replace_char(
                char *where,
                char16_t codepoint
            );

            std::string _data;
        
        template<class T>
        friend struct std::hash;
        template<class _T>
        friend struct _Utf8IteratorBase;
        template<class _T>
        friend struct _Utf8RawCodepointRef;
    };
    class BTKAPI u16string:protected std::u16string{
        public:
            u16string();
            u16string(const char *);
            u16string(const char16_t *);
            u16string(const char16_t *,size_t n);
            u16string(const u16string &);
            u16string(const std::u16string &);
            u16string(std::u16string_view);
            u16string(u16string_view);
            ~u16string();

            #ifdef _WIN32
            u16string(const wchar_t *);
            u16string(const wchar_t *,size_t n);
            u16string(const std::wstring &);
            u16string(std::wstring_view);
            #endif

            using std::u16string::size;
            using std::u16string::empty;
            using std::u16string::data;
            using std::u16string::c_str;
            using std::u16string::length;
            using std::u16string::append;
            using std::u16string::pop_back;

            std::u16string &base() noexcept{
                return *this;
            }
            const std::u16string &base() const noexcept{
                return *this;
            }
            u8string to_utf8() const{
                return u16string_view(*this).to_utf8();
            }

            void swap(u16string &us){
                base().swap(us.base());
            }

        public:
            //Cast
            operator u16string_view() const noexcept{
                return u16string_view(base().data(),base().length());
            }
            operator std::u16string_view() const noexcept{
                return std::u16string_view(base().data(),base().length());
            }
            #ifdef _WIN32
            static_assert(sizeof(wchar_t) == sizeof(char16_t));
            operator std::wstring_view() const noexcept{
                return std::wstring_view((const wchar_t*)base().data(),base().length());
            }
            const wchar_t *w_str() const{
                return reinterpret_cast<const wchar_t*>(base().c_str());
            }
            #endif
        
        template<class T>
        friend struct std::hash;
        friend class u8string;
    };
    //TODO Utf32
    class BTKAPI u32string:public std::u32string{
        public:
            using std::u32string::u32string;
    };


    //u8string inline begin
    inline u8string::u8string(std::string_view s):
        _data(s){
    }
    inline u8string::u8string(u8string_view s):
        _data(std::string_view(s)){
    }
    inline u8string::u8string(const std::string &s):
        _data(s){
    }
    inline u8string::u8string(std::string &&s):
        _data(std::move(s)){
    }
    inline u8string::u8string(const_iterator beg,const_iterator end):
        _data(beg.current,Utf8GetNext(end.current)){
    }
    inline u8string::u8string(const char *s):
        _data(s){
    }
    inline u8string::u8string(const char *s,size_t n):
        _data(s,n){
    }
    inline u8string::u8string(const Uint8 *s):
        _data(reinterpret_cast<const char*>(s)){
    }
    inline u8string::u8string(const Uint8 *s,size_t n):
        _data(reinterpret_cast<const char*>(s),n){
    }
    inline u16string u8string::to_utf16() const{
        return u8string_view(*this).to_utf16();   
    }
    //u8string_view
    inline u8string_view::u8string_view(const std::string &s):
        std::string_view(s){
    }
    inline u8string_view::u8string_view(const u8string &s):
        std::string_view(s){
    }
    inline u8string_view::u8string_view(std::string_view s):
        std::string_view(s){
    }
    inline u8string_view::u8string_view(const_iterator beg,const_iterator end):
        std::string_view(beg.current,Utf8GetNext(end.current) - beg.current){
    }
    inline auto u8string_view::trim() const -> u8string{
        return strip();
    }
    inline auto u8string_view::to_string() const -> u8string{
        return u8string(data(),size());
    }
    inline auto u8string_view::split(char32_t ch,size_t max) const -> List{
        u8string tmp;
        tmp.push_back(ch);
        return split(tmp,max);
    }
    inline auto u8string_view::split_ref(char32_t ch,size_t max) const -> RefList{
        u8string tmp;
        tmp.push_back(ch);
        return split_ref(tmp,max);
    }
    //u16string_view
    inline u16string_view::u16string_view(std::u16string_view v)
        :std::u16string_view(v){
    }
    inline u16string_view::u16string_view(const u16string &s):
        std::u16string_view(s.base()){
    }
    inline u16string_view::u16string_view(const std::u16string &s):
        std::u16string_view(s){
    }
    //u16string_view windows
    #ifdef _WIN32
    inline u16string_view::u16string_view(const wchar_t *ws):
        u16string_view(reinterpret_cast<const char16_t*>(ws)){
    }
    inline u16string_view::u16string_view(const wchar_t *ws,size_t n):
        u16string_view(reinterpret_cast<const char16_t*>(ws),n){
    }
    inline u16string_view::u16string_view(const std::wstring &ws):
        u16string_view(ws.c_str(),ws.length()){
    }
    inline u16string_view::u16string_view(std::wstring_view ws):
        u16string_view(ws.data(),ws.length()){
    }
    #endif
    //u16string
    inline u16string::u16string(const std::u16string &us):
        std::u16string(us){
    }
    inline u16string::u16string(const char16_t *us):
        std::u16string(us){
    }
    inline u16string::u16string(const char16_t *us,size_t n):
        std::u16string(us,n){
    }
    inline u16string::u16string(std::u16string_view us):
        std::u16string(us){
    }
    inline u16string::u16string(u16string_view us):
        std::u16string(std::u16string_view(us)){
    }
    //u16string windows
    #ifdef _WIN32
    inline u16string::u16string(const wchar_t *ws):
        u16string(reinterpret_cast<const char16_t*>(ws)){
    }
    inline u16string::u16string(const wchar_t *ws,size_t n):
        u16string(reinterpret_cast<const char16_t*>(ws),n){
    }
    inline u16string::u16string(std::wstring_view ws):
        u16string(ws.data(),ws.length()){
    }
    inline u16string::u16string(const std::wstring &ws):
        u16string(ws.c_str(),ws.length()){
    }
    #endif

    // /**
    //  * @brief output helper
    //  * 
    //  * @tparam T 
    //  * @param o 
    //  * @param p 
    //  * @return std::ostream& 
    //  */
    // template<class T>
    // inline std::ostream &operator <<(std::ostream &o,const _U8Proxy<T> &p){
    //     o << std::string_view(p);
    //     return o;
    // }
    //Output codepint reference
    template<class T>
    inline std::ostream &operator <<(std::ostream &o,const _Utf8RawCodepointRef<T> &p){
        const char *cur = &p;
        const char *end = &p;
        Utf8Next(end);
        o << std::string_view(cur,end - cur);
        return o;
    }
    inline std::ostream &operator <<(std::ostream &o,const _Uft8ConstCodepoinRef &p){
        const char *cur = &p;
        const char *end = &p;
        Utf8Next(end);
        o << std::string_view(cur,end - cur);
        return o;
    }
    template<class C,bool V>
    inline std::ostream &operator <<(std::ostream &o,const _Utf8IteratorProxy<C,V> &p){
        const char *cur = p.iterator->current;
        const char *end = p.iterator->current;
        Utf8Next(end);
        o << std::string_view(cur,end - cur);
        return o;
    }
    
    inline std::ostream &operator <<(std::ostream &o,const u8string &p){
        #if defined(_WIN32) && !defined(BTK_WIN32_NOLOCALE)
        //We need to covert to locale
        o << p.to_locale();
        #else
        o << std::string_view(p);
        #endif
        return o;
    }
    inline std::ostream &operator <<(std::ostream &o,u8string_view p){
        #if defined(_WIN32) && !defined(BTK_WIN32_NOLOCALE)
        o << p.to_locale();
        #else
        o << std::string_view(p);
        #endif
        return o;
    }
    inline std::ostream &operator <<(std::ostream &o,const u16string & p){
        o << p.to_utf8();
        return o;
    }
    inline std::ostream &operator <<(std::ostream &o,u16string_view p){
        o << p.to_utf8();
        return o;
    }
    BTK_STRING_OPERATOR(u8string,<);
    BTK_STRING_OPERATOR(u8string,>);
    BTK_STRING_OPERATOR(u8string,==);
    BTK_STRING_OPERATOR(u8string,!=);
    BTK_STRING_OPERATOR(u8string,>=);
    BTK_STRING_OPERATOR(u8string,<=);
    inline bool operator ==(const u8string &s1,u8string_view s2){
        return s1.base() == s2.base();
    }
    inline bool operator ==(const u8string &s1,const char *s2){
        return s1.base() == s2;
    }
    inline u8string operator +(const u8string &u1,const u8string &u2){
        u8string ret(u1);
        ret.append(u2);
        return ret;
    }
    inline u8string operator +(const u8string &u1,const char *u2){
        u8string ret(u1);
        ret.append(u2);
        return ret;
    }
    inline u8string operator ""_btku8(const char *str,size_t n){
        return u8string(str,n);
    }
    //u8string inline end
    //u8string_view begin
    BTK_STRING_OPERATOR(u8string_view,>);
    BTK_STRING_OPERATOR(u8string_view,<);
    BTK_STRING_OPERATOR(u8string_view,==);
    BTK_STRING_OPERATOR(u8string_view,!=);
    BTK_STRING_OPERATOR(u8string_view,>=);
    BTK_STRING_OPERATOR(u8string_view,<=);
    inline bool operator ==(const u8string_view &s1,const char *s2){
        return s1.base().compare(s2);
    }
    inline u8string_view operator ""_btku8v(const char *s,size_t n){
        return u8string_view(s,n);
    }
    //u8string_view end
    
    //u16string inline begin
    BTK_STRING_OPERATOR(u16string,>);
    BTK_STRING_OPERATOR(u16string,<);
    BTK_STRING_OPERATOR(u16string,==);
    BTK_STRING_OPERATOR(u16string,!=);
    BTK_STRING_OPERATOR(u16string,>=);
    BTK_STRING_OPERATOR(u16string,<=);
    inline u16string operator ""_btku16(const char16_t *str,size_t n){
        return u16string(str,n);
    }
    //u16string end
    
    //u16strinf_view inline begin
    BTK_STRING_OPERATOR(u16string_view,>);
    BTK_STRING_OPERATOR(u16string_view,<);
    BTK_STRING_OPERATOR(u16string_view,==);
    BTK_STRING_OPERATOR(u16string_view,!=);
    BTK_STRING_OPERATOR(u16string_view,>=);
    BTK_STRING_OPERATOR(u16string_view,<=);
    inline u16string_view operator ""_btku16v(const char16_t *str,size_t n){
        return u16string_view(str,n);
    }    //u16strinf_view end
    /**
     * @brief Utf8 format string
     * 
     * @param fmt 
     * @return BTKAPI 
     */
    BTKAPI u8string  u8vformat(const char *fmt,std::va_list);
    BTKAPI u16string u16vformat(const char16_t *fmt,std::va_list);
    /**
     * @brief Utf8 format string
     * 
     * @param fmt 
     * @param ... 
     * @return u8string 
     */
    BTKAPI  u8string u8format(const char *fmt,...);
    BTKAPI u16string u16format(const char16_t *fmt,...);
    /**
     * @brief Convert a number to string
     * 
     * @tparam T 
     * @tparam _Cond 
     * @param value 
     * @return u8string 
     */
    template<
        class T,
        typename _Cond = std::enable_if_t<std::is_arithmetic_v<T>>
    >
    u8string ToString(const T &value){
        return std::to_string(value);
    }
    // /**
    //  * @brief UTF8 String List
    //  * 
    //  */
    // class StringList_:public std::vector<u8string>{
    //     public:
    //         using std::vector<u8string>::vector;
            
    //         u8string &operator [](size_type idx){
    //             return at(idx);
    //         }
    //         const u8string &operator [](size_type idx) const{
    //             return at(idx);
    //         }
    // };
    // class StringRefList_:public std::vector<u8string_view>{
        
    // };

    using StringList = u8string_view::List;
    using StringRefList = u8string_view::RefList;

    using String = u8string;

    //Hook iconv functions
    class _iconv;
    using iconv_t = _iconv *;
    struct IconvFunctions{
        iconv_t (*iconv_open)(const char *tocode,const char *fromcode);
        int     (*iconv_close)(iconv_t);
        size_t  (*iconv)(iconv_t,const char **,size_t*,char **,size_t*);
    };
    BTKAPI void HookIconv(IconvFunctions) noexcept;
    BTKAPI void GetIconv(IconvFunctions&) noexcept;
    //Std
    inline std::ostream &operator <<(std::ostream &os,const char *s){
        os << u8string_view(s);
        return os;
    }
    // inline std::ostream &operator <<(std::ostream &os,const StringList &strlist){
    //     for(auto &str:strlist){

    //     }
    // }

    //Triats for string class
    template<class T,class = void>
    struct IsStringClass:std::false_type{};
    template<class T>
    struct IsStringClass<T,
        std::void_t<
            decltype(std::declval<T>().data()),
            decltype(std::declval<T>().size()),
            decltype(std::declval<T>().length())>
        >
        :std::true_type{

    };
    
    //Strlen for different cstring
    inline size_t _strlen_ptr(const char *s) noexcept{
        return std::strlen(s);
    }

    //For pointer
    template<class T>
    size_t _strlen_ptr(const T *str) noexcept{
        size_t n = 0;
        while(*str != T('\0')){
            ++str;
            ++n;
        }
        return n;
    }
    // For static string
    template<class T,size_t N>
    constexpr size_t _strlen_array(const T (&)[N]) noexcept{
        //Remove '\0'
        return N - 1;
    }

    template<class T>
    constexpr size_t strlen(T &&p) noexcept{
        if constexpr(std::is_array_v<std::remove_reference_t<T>>){
            return _strlen_array(std::forward<T>(p));
        }
        else if constexpr(std::is_pointer_v<std::decay_t<T>>){
            return _strlen_ptr(std::forward<T>(p));
        }
        else if constexpr(IsStringClass<std::decay_t<T>>()){
            if constexpr(std::is_same_v<char,std::decay_t<decltype(*std::declval<T>().data())>>){
                //use size
                return p.size();
            }
            else{
                //use length
                return p.length();
            }
        }
        else{
            static_assert(std::is_same_v<void,T>());
        }
    }
}
#endif // _BTK_STRING_HPP_
