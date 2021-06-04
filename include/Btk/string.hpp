#if !defined(_BTK_STRING_HPP_)
#define _BTK_STRING_HPP_
#include <iosfwd>
#include <string>
#include <vector>
#include <string_view>
#include <type_traits>
#include <cstdarg>
#include "defs.hpp"
/**
 * @brief Macro to generate operator
 * 
 */
#define BTK_STRING_OPERATOR(TYPE,OP) \
    inline auto operator OP(const TYPE &t1,const TYPE &t2){\
        return t1.base() OP t2.base();\
    }

namespace Btk{
    //Get iterator type
    template<class T>
    struct _U8StringImplIterator{
        using Iterator = char *;
    };
    template<class T>
    struct _U8StringImplIterator<const T>{
        using Iterator = const char *;
    };
    BTKAPI size_t Utf8Strlen(const char *beg,const char *end = nullptr);
    BTKAPI char32_t Utf8Next(const char *&);
    BTKAPI char32_t Utf8Prev(const char *&);
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
    BTKAPI bool Utf8IsVaild(const char *beg,const char *end);
    /**
     * @brief Advance in the string
     * 
     * @param beg The str begin
     * @param end The str end
     * @param cur current position
     * @param n The distance
     * @return BTKAPI const* (nullptr on out of range)
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
             * @brief find a char of
             * 
             * @return size_t 
             */
            size_t find(char32_t) const;
            size_t find(u8string_view) const;

            u16string to_utf16() const;
            u8string toupper() const;
            u8string tolower() const;
            /**
             * @brief Strip the space at begin and end
             * 
             * @return u8string 
             */
            u8string strip() const;
            /**
             * @brief Cmp string(ignore the case)
             * 
             * @return true 
             * @return false 
             */
            bool casecmp(u8string_view) const;
            /**
             * @brief Split string and copy them into buffer
             * 
             * @param delim 
             * @param max Max substring(default unlimited)
             * @return List 
             */
            List    split(u8string_view delim,size_t max = size_t(-1));
            List    split(char delim,size_t max = size_t(-1)){
                return split(u8string_view(&delim,1),max);
            }
            RefList split_ref(u8string_view delim,size_t max = size_t(-1));
            RefList split_ref(char delim,size_t max = size_t(-1)){
                return split_ref(u8string_view(&delim,1),max);
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

            std::u16string_view &base(){
                return *this;
            }
            const std::u16string_view &base() const{
                return *this;
            }
            u8string to_utf8() const;
        
        template<class T>
        friend struct std::hash;
    };
    /**
     * @brief UTF8 String
     * 
     */
    class BTKAPI u8string:protected std::string{
        public:
            u8string();
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

            template<class T>
            u8string &operator +=(T &&arg){
                base() += std::forward<T>(arg);
                return *this;
            }
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
            operator u8string_view() const noexcept{
                return u8string_view(base().c_str(),base().length());
            }
            std::string *operator ->() noexcept{
                return this;
            }
            std::string &operator &() noexcept{
                return *this;
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
            ~u16string();

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
    //u8string_view
    inline u8string_view::u8string_view(const std::string &s):
        std::string_view(s){
    }
    inline u8string_view::u8string_view(std::string_view s):
        std::string_view(s){
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
        o << std::string_view(p);
        return o;
    }
    inline std::ostream &operator <<(std::ostream &o,u8string_view p){
        o << std::string_view(p);
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
    BTKAPI u8string u8vformat(const char *fmt,std::va_list);
    /**
     * @brief Utf8 format string
     * 
     * @param fmt 
     * @param ... 
     * @return u8string 
     */
    inline u8string u8vformat(const char *fmt,...){
        std::va_list l;
        va_start(l,fmt);
        auto r = u8vformat(fmt,l);
        va_end(l);
        return r;
    }
    using StringList = u8string_view::List;
    using StringRefList = u8string_view::RefList;

    using String = u8string;
}
#endif // _BTK_STRING_HPP_
