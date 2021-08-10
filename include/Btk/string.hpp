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
    inline auto operator OP(const TYPE &t1,const TYPE &t2){\
        return t1.base() OP t2.base();\
    }

namespace Btk{
    #ifdef _WIN32
    inline constexpr auto strncasecmp = _strnicmp;
    #else
    inline constexpr auto strncasecmp = ::strncasecmp;
    #endif

    inline constexpr auto CaseSensitive = false;
    inline constexpr auto CaseInSensitive = true;

    //Get iterator type
    template<class T>
    struct _U8StringImplIterator{
        using Iterator = char *;
    };
    template<class T>
    struct _U8StringImplIterator<const T>{
        using Iterator = const char *;
    };
    /**
     * @brief Get len of a utf8 string
     * 
     * @param beg The string's begin
     * @param end The string's end(could be nullptr)
     * @return The string's size
     */
    BTKAPI size_t Utf8Strlen(const char *beg,const char *end = nullptr);
    /**
     * @brief Move to the next char begin
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return The utf32 encoded codepoint
     */
    BTKAPI char32_t Utf8Next(const char *&);
    /**
     * @brief Move to the prev char begin
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return The utf32 encoded codepoint
     */
    BTKAPI char32_t Utf8Prev(const char *&);
    /**
     * @brief Get the utf32 codepoint
     * 
     * @param ch The pointer to the char(could not be nullptr)
     * @return char32_t 
     */
    inline char32_t Utf8Peek(const char *c){
        return Utf8Next(c);
    }
    inline char32_t Utf8Next(char *& c){
        return Utf8Next(const_cast<const char*&>(c));
    }
    inline char32_t Utf8Prev(char *& c){
        return Utf8Prev(const_cast<const char*&>(c));
    }
    /**
     * @brief Get size of a utf8 char
     * 
     * @param char 
     * @return size_t 
     */
    inline size_t Utf8CharSize(const char *s){
        auto n = s;
        Utf8Next(n);

        return n - s;
    }
    /**
     * @brief Get distance of two utf8 char
     * 
     * @param p1 
     * @param p2 
     * @return ptrdiff_t 
     */
    inline ptrdiff_t Utf8Distance(const char *p1,const char *p2){
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
    BTKAPI bool Utf8IsVaild(const char *beg,const char *end = nullptr);
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
    const char *Utf8Advance(const char *beg,const char *end,const char *cur,long n);
    /**
     * @brief Proxy for u8string
     * 
     */
    template<class T>
    struct _U8Proxy{
        using Iterator = typename _U8StringImplIterator<T>::Iterator;

        T *str = nullptr;
        Iterator _beg;
        Iterator _end;

        _U8Proxy() = default;
        _U8Proxy(T *s,Iterator i1,Iterator i2)
            :str(s),_beg(i1),_end(i2){}
        _U8Proxy(const _U8Proxy &) = default;
        _U8Proxy(_U8Proxy &&) = default;

        /**
         * @brief Cast to UTF32
         * 
         * @return char32_t 
         */
        operator char32_t() const{
            return Utf8Peek(_beg);
        }
        /**
         * @brief Cast to string_view
         * 
         * @return std::string_view 
         */
        operator std::string_view () const{
            const char *beg = &*_beg;
            const char *end = &*_end;
            return std::string_view(beg,end - beg);
        }
        /**
         * @brief Get the size of the char
         * 
         * @return size_t 
         */
        size_t size(){
            const char *beg = &*_beg;
            const char *end = &*_end;
            return end - beg;
        }
        char32_t get() const{
            return Utf8Peek(_beg);
        }
        /**
         * @brief Assign value
         * 
         * @tparam Ch 
         * @param ch 
         * @return _U8Proxy& 
         */
        template<class Ch>
        _U8Proxy &operator =(const Ch &ch){
            static_assert(not std::is_const_v<T>,"Const value cannot be assigned");
            static_assert(not std::is_integral_v<T>,"Must assigned a char,char16_t,char32_t");
            str->impl_replace_ch(*this,ch);
            return *this;
        }
        char *operator &() noexcept{
            return _beg;
        }
        _U8Proxy &operator =(const _U8Proxy &) = default;
    };
    template<class T>
    struct _U8Iterator:protected _U8Proxy<T>{
        using _U8Proxy<T>::_U8Proxy;
        _U8Iterator() = default;
        _U8Iterator(const _U8Iterator &) = default;
        _U8Iterator &operator ++(){
            Utf8Next(this->_beg);
            Utf8Next(this->_end);
            return *this;
        }
        _U8Iterator operator ++(int){
            Utf8Next(this->_beg);
            Utf8Next(this->_end);
            return *this;
        }
        _U8Iterator &operator --(){
            //string.end()
            if(this->_beg == this->_end){
                Utf8Prev(this->_beg);
            }
            else{
                Utf8Prev(this->_beg);
                Utf8Prev(this->_end);
            }
            return *this;
        }
        _U8Iterator operator --(int){
            if(this->_beg == this->_end){
                Utf8Prev(this->_beg);
            }
            else{
                Utf8Prev(this->_beg);
                Utf8Prev(this->_end);
            }
            return *this;
        }
        bool operator ==(const _U8Iterator &i) const{
            return this->_beg == i._beg;
        }
        bool operator !=(const _U8Iterator &i) const{
            return this->_beg != i._beg;
        }
        //STL
        _U8Proxy<T> &operator *(){
            return *this;
        }
        _U8Proxy<T> *operator ->(){
            return this;
        }
        /**
         * @brief Calc the distance
         * 
         * @tparam T 
         * @param i 
         * @return ptrdiff_t 
         */
        template<class T1>
        ptrdiff_t operator -(const _U8Iterator<T1> i) const{
            return Utf8Distance(this->_beg,i._beg);
        }
        friend class u8string;
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

            using _iterator = const char *;
            using _const_iterator = const char *;
            /**
             * @brief Get the length of the string_view
             * 
             * @return size_t 
             */
            size_t length() const{
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
            /**
             * @brief Strip the space at begin and end
             * 
             * @return u8string 
             */
            u8string strip() const;
            u8string trim() const;
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
    class BTKAPI u8string:protected std::string{
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

            using std::string::basic_string;

            using CharProxy = _U8Proxy<u8string>;
            using ConstCharProxy = _U8Proxy<const u8string>;
            using Iterator = _U8Iterator<u8string>;
            using ConstIterator = _U8Iterator<const u8string>;

            //stl
            using const_iterator = ConstIterator;
            using iterator = Iterator;
            u8string(const_iterator beg,const_iterator end);

            using value_type = char32_t;
            using reference = char32_t&;
            using pointer = char32_t*;

            using std::string::assign;
            using std::string::c_str;
            using std::string::empty;
            using std::string::data;
            using std::string::swap;
            using std::string::size;
            using std::string::clear;
            using std::string::resize;
            using std::string::append;
            using std::string::compare;
            using std::string::max_size;
            using std::string::shrink_to_fit;
            using std::string::size_type;
            using std::string::npos;
            //Impl
            using _iterator = char *;
            using _const_iterator = const char *;

            using Base = std::string;
            using List = std::vector<u8string>;
            using RefList = std::vector<u8string_view>;

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
                Base::push_back(ch);
            }
            /**
             * @brief Note it is a slow operation
             * 
             */
            void pop_front();
            /**
             * @brief Erase
             * 
             * @tparam T 
             * @param iter 
             */
            template<class T>
            void erase(_U8Iterator<T> iter){
                Base::erase(iter._beg,iter._end);
            }
            /**
             * @brief Locate the char 
             * 
             * @param index 
             * @return _U8Proxy 
             */
            CharProxy      at(size_type index);
            ConstCharProxy at(size_type index) const;
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

            CharProxy      operator [](size_type index){
                return at(index);
            }
            ConstCharProxy operator [](size_type index) const{
                return at(index);
            }

            CharProxy front(){
                return begin();
            }
            ConstCharProxy front() const{
                return begin();
            }

            CharProxy back(){
                return --end();
            }
            ConstCharProxy back() const{
                return --end();
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
            /**
             * @brief Create string from
             * 
             * @param buf buffer
             * @param n buffer size
             * @param encoding the string encoding(default utf8)
             * @return u8string 
             */
            static u8string from(const void *,size_t n,const char *encoding = nullptr);
        public:
            //beg and end
            Iterator begin(){
                auto beg = impl_begin();
                auto end = beg;
                Utf8Next(end);
                return {
                    this,
                    beg,
                    end
                };
            }
            Iterator end(){
                auto beg = impl_end();
                auto end = beg;
                return {
                    this,
                    beg,
                    end
                };
            }
            ConstIterator begin() const{
                auto beg = impl_begin();
                auto end = beg;
                Utf8Next(end);
                return {
                    this,
                    beg,
                    end
                };
            }
            ConstIterator end() const{
                auto beg = impl_end();
                auto end = beg;
                return {
                    this,
                    beg,
                    end
                };
            }
        public:
            operator std::string_view() const noexcept{
                return std::string_view(base().c_str(),base().length());
            }
            //FIXME:C2664 If we donnot ocomit it
            // operator u8string_view() const noexcept{
            //     return u8string_view(base().c_str(),base().length());
            // }
            std::string *operator ->() noexcept{
                return this;
            }
            const std::string *operator ->() const noexcept{
                return this;
            }
            const std::string &operator *() const noexcept{
                return *this;
            }

            std::string &get(){
                return *this;
            }
            const std::string &get() const{
                return *this;
            }
            std::string &base(){
                return *this;
            }
            const std::string &base() const{
                return *this;
            }

            u8string toupper() const{
                return u8string_view(*this).toupper();
            }
            u8string tolower() const{
                return u8string_view(*this).tolower();
            }
            u8string substr(size_t pos = 0,size_t len = npos) const{
                return u8string_view(*this).substr(0,len);
            }
            bool casecmp(u8string_view v) const{
                return u8string_view(*this).casecmp(v);
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
            char *_translate_iterator(Base::iterator i){
                return &*i;
            }
            const char *_translate_iterator(Base::const_iterator i) const{
                return &*i;
            }
        private:
            void impl_replace_ch(CharProxy &p,char32_t ch);
            void impl_replace_ch(CharProxy &p,char16_t ch){
                impl_replace_ch(p,char32_t(ch));
            }
            void impl_replace_ch(CharProxy &p,char ch);
        
        template<class T>
        friend struct std::hash;
        template<class _T>
        friend struct _U8Proxy;
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

            using std::u16string::data;
            using std::u16string::c_str;

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
            #endif
        
        template<class T>
        friend struct std::hash;
        friend class u8string;
    };
    //u8string inline begin
    inline u8string::u8string(std::string_view s):
        std::string(s){
    }
    inline u8string::u8string(u8string_view s):
        std::string(std::string_view(s)){
    }
    inline u8string::u8string(const std::string &s):
        std::string(s){
    }
    inline u8string::u8string(std::string &&s):
        std::string(std::move(s)){
    }
    inline u8string::u8string(const_iterator beg,const_iterator end):
        std::string(beg._beg,end._end){

    }
    inline u8string::u8string(const char *s):
        std::string(s){
    }
    inline u8string::u8string(const char *s,size_t n):
        std::string(s,n){
    }
    inline u8string::u8string(const Uint8 *s):
        std::string(reinterpret_cast<const char*>(s)){
    }
    inline u8string::u8string(const Uint8 *s,size_t n):
        std::string(reinterpret_cast<const char*>(s),n){
    }
    inline u16string u8string::to_utf16() const{
        return u8string_view(*this).to_utf16();   
    }
    inline void u8string::append_fmt(const char *fmt,...){
        va_list varg;
        va_start(varg,fmt);
        append_vfmt(fmt,varg);
        va_end(varg);
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

    /**
     * @brief output helper
     * 
     * @tparam T 
     * @param o 
     * @param p 
     * @return std::ostream& 
     */
    template<class T>
    inline std::ostream &operator <<(std::ostream &o,const _U8Proxy<T> &p){
        o << std::string_view(p);
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
    inline u8string u8format(const char *fmt,...){
        std::va_list l;
        va_start(l,fmt);
        auto r = u8vformat(fmt,l);
        va_end(l);
        return r;
    }
    inline u16string u16format(const char16_t *fmt,...){
        std::va_list l;
        va_start(l,fmt);
        auto r = u16vformat(fmt,l);
        va_end(l);
        return r;
    }
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
    BTKAPI void HookIconv(IconvFunctions);
    BTKAPI void GetIconv(IconvFunctions&);
    //Std
    inline std::ostream &operator <<(std::ostream &os,const char *s){
        os << u8string_view(s);
        return os;
    }
    // inline std::ostream &operator <<(std::ostream &os,const StringList &strlist){
    //     for(auto &str:strlist){

    //     }
    // }
}
#endif // _BTK_STRING_HPP_
