#include "build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/string.hpp>
#include <SDL2/SDL_stdinc.h>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <new>
namespace Btk{
    u8string::u8string() = default;
    u8string::~u8string() = default;
    u8string::u8string(const u8string &) = default;
    //Index
    _U8Proxy<u8string> u8string::at(size_type index){
        auto iter = impl_begin();

        for(size_type i = 0;i < index; i++){
            if(iter == impl_end()){
                throw std::out_of_range("At u8 index");
            }
            utf8::unchecked::next(iter);
        }
        auto end = iter;

        if(end == impl_end()){
            throw std::out_of_range("At u8 index");
        }

        utf8::unchecked::next(end);
        return {
            this,
            iter,
            end
        };
    }
    _U8Proxy<const u8string> u8string::at(size_type index) const{
        auto iter = impl_begin();

        for(size_type i = 0;i < index; i++){
            if(iter == impl_end()){
                throw std::out_of_range("At u8 index");
            }
            utf8::unchecked::next(iter);
        }
        auto end = iter;
        
        if(end == impl_end()){
            throw std::out_of_range("At u8 index");
        }
        utf8::unchecked::next(end);
        return {
            this,
            iter,
            end
        };
    }
    void u8string::impl_replace_ch(CharProxy &p,char32_t ch){
        std::string tmp;
        std::u32string_view view(&ch,1);
        utf8::unchecked::utf32to8(view.begin(),view.end(),std::back_insert_iterator(tmp));

        auto diff = p._beg - impl_begin();
        std::string::replace(_translate_pointer(p._beg),_translate_pointer(p._end),tmp);
        //To cur pos
        p._beg = impl_begin() + diff;
        p._end = impl_begin() + diff;
        
        Utf8Next(p._end);
    }
    void u8string::impl_replace_ch(CharProxy &p,char ch){
        auto diff = p._beg - impl_begin();
        std::string::replace(_translate_pointer(p._beg),_translate_pointer(p._end),&ch,1);
        //To cur pos
        p._beg = impl_begin() + diff;
        p._end = impl_begin() + diff;
        
        Utf8Next(p._end);

    }
    void u8string::push_back(char32_t ch){
        std::u32string_view view(&ch,1);
        utf8::unchecked::utf32to8(view.begin(),view.end(),std::back_insert_iterator(base()));
    }
    void u8string::pop_back(){
        if(empty()){
            return;
        }
        auto iter = end();
        --iter;

        for(int i = 0;i < iter->size();i++){
            std::string::pop_back();
        }
    }
    void u8string::pop_front(){
        if(empty()){
            return;
        }
        auto iter = begin();
        base().erase(_translate_pointer(iter._beg),_translate_pointer(iter._end));
    }
    std::string u8string::encoode(const char *to) const{
        char *i = SDL_iconv_string(to,"UTF8",data(),base().size());
        if(i == nullptr){
            throwSDLError();
        }
        std::string s(i);
        SDL_free(i);
        return s;
    }
    u16string u8string::to_utf16() const{
        u16string s;
        Utf8To16(s,*this);
        return s;
    }
    size_t u8string::find(char32_t ch) const{
        size_t n = 0;
        auto iter = impl_begin();
        while(iter != impl_end()){
            if(utf8::unchecked::next(iter) == ch){
                return n;
            }
            ++n;
        }
        return npos;
    }
    u8string u8string::tolower() const{
        u8string c = *this;
        size_t n = 0;
        for(auto ch:base()){
            if(std::isalpha(ch)){
                c.base()[n] = std::tolower(ch);
            }
            ++n;
        }
        return c;
    }
    u8string u8string::toupper() const{
        u8string c = *this;
        size_t n = 0;
        for(auto ch:base()){
            if(std::isalpha(ch)){
                c.base()[n] = std::toupper(ch);
            }
            ++n;
        }
        return c;
    }
}
namespace Btk{
    u16string::u16string() = default;
    u16string::u16string(const u16string &) = default;
    u16string::~u16string() = default;
}
namespace Btk{
    u8string u8vformat(const char *fmt,std::va_list varg){
        BTK_ASSERT(fmt != nullptr);
        //Get bufsize
        std::va_list v1;
        va_copy(v1,varg);
        int s;
        
        #ifdef _WIN32
        s = _vscprintf(fmt,v1);
        #else
        s = vsnprintf(nullptr,0,fmt,v1);
        #endif
        va_end(v1);

        u8string str;
        str.resize(s);

        vsprintf(str.data(),fmt,varg);

        return str;
    }
    char32_t Utf8Next(const char *& ch){
        return utf8::unchecked::next(ch);
    }
    char32_t Utf8Prev(const char *& ch){
        return utf8::unchecked::prior(ch);
    }
    size_t Utf8Strlen(const char *begin,const char *end){
        if(end == nullptr){
            end = begin + std::strlen(begin);
        }
        size_t len = 0;
        while(begin != end){
            utf8::unchecked::next(begin);
            ++ len;
        }
        return len;
    }
    bool Utf8IsVaild(const char *begin,const char *end){
        if(end == nullptr){
            end = begin + std::strlen(begin);
        }
        return utf8::is_valid(begin,end);
    }
}
namespace Btk{
    size_t u8string_view::find(char32_t ch) const{
        size_t n = 0;
        auto iter = impl_begin();
        while(iter != impl_end()){
            if(utf8::unchecked::next(iter) == ch){
                return n;
            }
            ++n;
        }
        return npos;
    }
}