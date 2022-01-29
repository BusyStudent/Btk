#include "build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/impl/core.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/string.hpp>
#include <SDL2/SDL_stdinc.h>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <new>

#define BTK_ICONV_FN(FN) decltype(FN) FN = reinterpret_cast<decltype(FN)>(SDL_##FN);
//Iconv
namespace Btk{
    BTK_ICONV_FN(iconv_open);
    BTK_ICONV_FN(iconv_close);
    BTK_ICONV_FN(iconv);

    //I used some sdl iconv codes
    //Code from SDL_iconv
    template<class T>
    void sdl_getlocale(T &buffer){
        const char *lang;
        char *ptr;

        lang = SDL_getenv("LC_ALL");
        if (!lang) {
            lang = SDL_getenv("LC_CTYPE");
        }
        if (!lang) {
            lang = SDL_getenv("LC_MESSAGES");
        }
        if (!lang) {
            lang = SDL_getenv("LANG");
        }
        if (!lang || !*lang || SDL_strcmp(lang, "C") == 0) {
            lang = "ASCII";
        }

        /* We need to trim down strings like "en_US.UTF-8@blah" to "UTF-8" */
        ptr = SDL_strchr(lang, '.');
        if (ptr != NULL) {
            lang = ptr + 1;
        }

        buffer = lang;

        ptr = SDL_strchr(buffer.data(), '@');
        if (ptr != NULL) {
            *ptr = '\0';            /* chop end of string. */
        }
    }
    struct BTKHIDDEN IconvHolder{
        iconv_t icd;
        IconvHolder(const char *tocode,const char *fromcode){
            icd = iconv_open(tocode,fromcode);
            //Err handle
        }
        IconvHolder(const IconvHolder &) = delete;
        ~IconvHolder(){
            iconv_close(icd);
        }
        operator iconv_t() const noexcept{
            return icd;
        }
    };

    template<class T>
    static void iconv_string(iconv_t cd,T &_buf,const char *inbuf,size_t inbytesleft){
        //Reset
        iconv(cd,nullptr,nullptr,nullptr,nullptr);
        
        char *string;
        size_t stringsize;
        char *outbuf;
        size_t outbytesleft;
        size_t retCode = 0;

        stringsize = inbytesleft > 4 ? inbytesleft : 4;
        
        //Restore the end
        //Restore the oldsize
        size_t _oldsize = _buf.size();
        ptrdiff_t diff = _buf.end() - _buf.begin();
        _buf.resize(_buf.size() + stringsize);
        string = _buf.data() + diff;

        outbuf = string;
        outbytesleft = stringsize;
        SDL_memset(outbuf, 0, 4);

        while (inbytesleft > 0) {
            const size_t oldinbytesleft = inbytesleft;
            retCode = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            switch (retCode) {
                case SDL_ICONV_E2BIG:{
                    char *oldstring = string;
                    stringsize *= 2;

                    _buf.resize(_oldsize + stringsize);
                    //string = (char *) SDL_realloc(string, stringsize);
                    string = _buf.data() + diff;

                    outbuf = string + (outbuf - oldstring);
                    outbytesleft = stringsize - (outbuf - string);
                    SDL_memset(outbuf, 0, 4);
                    }
                    break;
                case SDL_ICONV_EILSEQ:
                    /* Try skipping some input data - not perfect, but... */
                    ++inbuf;
                    --inbytesleft;
                    BTK_LOGWARN("Get EILSEQ at %p",inbuf);
                    break;
                case SDL_ICONV_EINVAL:
                case SDL_ICONV_ERROR:
                    /* We can't continue... */
                    inbytesleft = 0;
                    break;
            }
            /* Avoid infinite loops when nothing gets converted */
            if (oldinbytesleft == inbytesleft){
                break;
            }
        }
    }
    void HookIconv(IconvFunctions fns) noexcept{
        iconv_open = fns.iconv_open;
        iconv_close = fns.iconv_close;
        iconv = fns.iconv;
    }
    void GetIconv(IconvFunctions &fns) noexcept{
        fns.iconv_open = iconv_open;
        fns.iconv_close = iconv_close;
        fns.iconv = iconv;
    }
    //Helper template
    template<class Beg,class End>
    auto utf8_index(Beg begin,End end,size_t index){
        auto iter = begin;

        for(size_t i = 0;i < index; i++){
            if(iter == end){
                throw std::out_of_range("At u8 index");
            }
            utf8::unchecked::next(iter);
        }

        return iter;
    }
}

//Uchardet
#if __has_include(<uchardet/uchardet.h>)
    #include <uchardet/uchardet.h>
    #include <Btk/impl/scope.hpp>
    #include <SDL2/SDL_thread.h>
    #include <SDL2/SDL_loadso.h>

    #define FNS \
        PROC(uchardet_new) \
        PROC(uchardet_delete) \
        PROC(uchardet_handle_data) \
        PROC(uchardet_data_end) \
        PROC(uchardet_reset) \
        PROC(uchardet_get_charset) \
        //

    #define LOAD(X) \
        X##_fn = (decltype(::X)*) SDL_LoadFunction(uchardet_lib,#X);
    #define DECL(X) \
        decltype(::X) *X##_fn = nullptr;

    namespace{
        SDL_TLSID uchardet_instance = 0;
        void *uchardet_lib = nullptr;
        #define PROC(X) DECL(X)
        FNS
        #undef PROC
    }
    Btk_CallOnUnload{
        if(uchardet_lib != nullptr){
            uchardet_delete_fn(
                static_cast<uchardet_t>(SDL_TLSGet(uchardet_instance))
            );
            SDL_TLSSet(uchardet_instance,nullptr,nullptr);
            SDL_UnloadObject(uchardet_lib);
        }
    };
    static bool uchardet_init() noexcept{
        uchardet_lib = SDL_LoadObject("libuchardet.so");
        if(uchardet_lib != nullptr){
            #define PROC(X) LOAD(X)
            FNS
            #undef PROC

            uchardet_instance = SDL_TLSCreate();
            return true;
        }
        return false;
    }
    static uchardet_t udet_get() noexcept{
        //NO lib loaded
        if(uchardet_lib == nullptr){
            if(not uchardet_init()){
                //Init failed
                return nullptr;
            }            
        }
        uchardet_t t = (uchardet_t)SDL_TLSGet(uchardet_instance);
        if(t == nullptr){
            //Create a new one
            t = uchardet_new_fn();
            SDL_TLSSet(
                uchardet_instance,
                t,
                reinterpret_cast<void(*)(void*)>(uchardet_delete_fn)
            );
        }
        uchardet_reset_fn(t);
        return t;
    }
    static const char *get_charset_from(const void *mem,size_t n){
        auto t = udet_get();
        if(t == nullptr){
            Btk::throwRuntimeError("Unknown charset");
        }
        uchardet_handle_data_fn(t,static_cast<const char*>(mem),n);
        uchardet_data_end_fn(t);
        const char *ret = uchardet_get_charset_fn(t);
        if(strcmp(ret,"") == 0){
            Btk::throwRuntimeError("Unknown charset");
        }
        return ret;
    }
    #undef LOAD
    #undef DECL
    #undef FNS
    #define GET_CHARSET_FROM(MEM,N) get_charset_from(MEM,N)
#else
    #define GET_CHARSET_FROM(MEM,N) "UTF-8"
#endif


namespace Btk{
    u8string::u8string() = default;
    u8string::~u8string() = default;
    u8string::u8string(const u8string &) = default;
    //Index
    u8string::reference u8string::at(size_type index){
        return {
            this,
            utf8_index(
                impl_begin(),
                impl_end(),
                index
            )
        };
    }
    u8string::const_reference u8string::at(size_type index) const{
        return {
            utf8_index(
                impl_begin(),
                impl_end(),
                index
            )
        };
    }
    //For replace a unicode char
    char * u8string::replace_char(char *where,char32_t ch){
        std::string buffer;
        //Query the end
        const char *end = where;
        Utf8Next(end);
        //Restore
        size_t n = where - impl_begin();
        //Begin replace
        utf8::unchecked::utf32to8(&ch,&ch + 1,std::back_inserter(buffer));

        base().replace(
            _translate_pointer(where),
            _translate_pointer(end),
            buffer
        );
        return impl_begin() + n;
    }
    char * u8string::replace_char(char *where,char16_t ch){
        std::string buffer;
        //Query the end
        const char *end = where;
        Utf8Next(end);
        size_t n = where - impl_begin();
        //Begin replace
        utf8::unchecked::utf16to8(&ch,&ch + 1,std::back_inserter(buffer));

        base().replace(
            _translate_pointer(where),
            _translate_pointer(end),
            buffer
        );
        return impl_begin() + n;
    }
    void u8string::push_back(char32_t ch){
        utf8::unchecked::append(ch,std::back_inserter(base()));
    }
    void u8string::pop_back(){
        if(empty()){
            return;
        }
        size_t size = back().size();

        for(size_t i = 0;i < size;i++){
            base().pop_back();
        }
    }
    void u8string::pop_front(){
        if(empty()){
            return;
        }
        auto iter = begin();
        // base().erase(_translate_pointer(iter.current),_translate_pointer(iter._end));
        erase(iter);
    }
    std::string u8string::encoode(const char *to) const{
        char *i = SDL_iconv_string(to,"UTF-8",c_str(),base().size());
        if(i == nullptr){
            throwSDLError();
        }
        std::string s(i);
        SDL_free(i);
        return s;
    }
    u8string u8string::from(const void *buf,size_t n,const char *encoding){
        if(encoding == nullptr){
            encoding = GET_CHARSET_FROM(buf,n);
        }
        u8string s;
        if(buf == nullptr or n == 0){
            return s;
        }
        IconvHolder iconv("UTF-8",encoding);
        iconv_string<std::string>(iconv,s.base(),static_cast<const char *>(buf),n);
        return s;
    }
    u8string u8string::fromfile(const char *filename){
        FILE *f = fopen(filename,"r");
        if(f == nullptr){
            throwCRuntimeError();
        }
        fseek(f,0,SEEK_END);
        auto n = ftell(f);
        fseek(f,0,SEEK_SET);

        u8string s;
        s.resize(n);
        if(fread(f,n,1,f) != 1){
            fclose(f);
            throwCRuntimeError();
        }
        fclose(f);
        return s;
    }
    void u8string::append_vfmt(const char *fmt,std::va_list varg){
        int strsize;
        //Get the size of the string
        va_list v;
        va_copy(v,varg);

        #ifdef _WIN32
        strsize = _vscprintf(fmt,v);
        #else
        strsize = vsnprintf(nullptr,0,fmt,v);
        #endif
        va_end(v);
        
        //Get the '\0'
        size_t length = base().length();
        base().resize(strsize + base().size());

        char *end = &base()[length];
        //start formatting
        vsprintf(end,fmt,varg);

    }
    void u8string::append_fmt(const char *fmt,...){
        va_list varg;
        va_start(varg,fmt);
        append_vfmt(fmt,varg);
        va_end(varg);
    }
    void u8string::append_from(const void *buf,size_t n,const char *encoding){
        IconvHolder i("UTF-8",GET_CHARSET_FROM(buf,n));
        iconv_string<std::string>(i,base(),static_cast<const char*>(buf),n);
    }
}
namespace Btk{
    u16string u8string_view::to_utf16() const{
        u16string s;
        Utf8To16(s,*this);
        return s;
    }
    u8string_view::const_reference u8string_view::at(size_t index) const{
        return {
            utf8_index(
                impl_begin(),
                impl_end(),
                index
            )
        };
    }
}
namespace Btk{
    u16string::u16string() = default;
    u16string::u16string(const u16string &) = default;
    u16string::~u16string() = default;
    u16string::u16string(const char *s){
        Utf8To16(*this,s);
    }
}
namespace Btk{
    u8string u16string_view::to_utf8() const{
        u8string str;
        Utf16To8(str,*this);
        return str;
    }
}
namespace Btk{
    void __sscanf_chk(int nargs,const char *s,const char *fmt,...){
        std::va_list vargs;
        va_start(vargs,fmt);
        auto ret = ::vsscanf(s,fmt,vargs);
        va_end(vargs);
        if(ret != nargs){
            //scan failed
            throwRuntimeError(u8format("Failed to parse '%s'",fmt));
        }
    }
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
    u8string u8format(const char *fmt,...){
        std::va_list l;
        va_start(l,fmt);
        auto r = u8vformat(fmt,l);
        va_end(l);
        return r;
    }
    //TODO add win32 wsprintf
    u16string u16vformat(const char16_t *fmt,std::va_list varg){
        return u8vformat(u16string_view(fmt).to_utf8().c_str(),varg).to_utf16();
    }
    u16string u16format(const char16_t *fmt,...){
        std::va_list l;
        va_start(l,fmt);
        auto r = u16vformat(fmt,l);
        va_end(l);
        return r;
    }
    char32_t Utf8Next(const char *& ch) noexcept{
        return utf8::unchecked::next(ch);
    }
    char32_t Utf8Prev(const char *& ch) noexcept{
        return utf8::unchecked::prior(ch);
    }
    size_t Utf8Strlen(const char *begin,const char *end) noexcept{
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
    size_t Utf32CharSize(char32_t codepoint) noexcept{
        if ((0xffffff80 & codepoint) == 0){
            return 1;
        } 
        else if((0xfffff800 & codepoint) == 0){
            return 2;
        } 
        else if((0xffff0000 & codepoint) == 0){
            return 3;
        } 
        else {
            return 4;
        }
    }
    size_t Utf8CharSize(const char *s) noexcept{
        #if 0
        return utf8::internal::sequence_length(s);
        #else
        auto n = Utf8GetNext(s);
        return n - s;
        #endif
    }
    bool Utf8IsVaild(const char *begin,const char *end) noexcept{
        if(end == nullptr){
            end = begin + std::strlen(begin);
        }
        return utf8::is_valid(begin,end);
    }
    const char *Utf8Advance(const char *beg,const char *end,const char *cur,long n) noexcept{
        if(cur == nullptr){
            return nullptr;
        }
        if(n > 0){
            //Forward
            if(end == nullptr){
                //No limit
                for(long i = 0;i < n;i++){
                    utf8::unchecked::next(cur);
                }
            }
            else{
                for(long i = 0;i < n;i++){
                    if(cur >= end){
                        return nullptr;
                    }
                    utf8::unchecked::next(cur);
                }
            }
        }
        else if(n < 0){
            //Backward
            if(beg == nullptr){
                //No limit
                for(long i = n;i < 0;i++){
                    utf8::unchecked::prior(cur);
                }
            }
            else{
                for(long i = n;i < 0;i++){
                    if(cur <= beg){
                        //Outof range
                        return nullptr;
                    }
                    utf8::unchecked::prior(cur);
                }
            }
        }
        //n == 0
        return cur;
    }
    const char *Utf8AdvanceChecked(const char *beg,const char *end,const char *cur,long n){
        const char *ptr = Utf8Advance(beg,end,cur,n);
        if(ptr == nullptr){
            throw std::out_of_range("At u8 index");
        }
        return ptr;
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
    size_t u8string_view::rfind(char32_t ch) const{
        //TODO Imporve the speed
        size_t n = length();
        if(n == 0){
            //Empty string
            return {};
        }
        n -= 1;
        auto iter = impl_end() - 1;
        while(iter >= impl_begin()){
            //Stop when we reach be broader
            n --;
            if(utf8::unchecked::prior(iter) == ch){
                return n;
            }
        }
        return npos;
    }
    //TODO Imporve the speed
    size_t u8string_view::find(u8string_view s) const{
        auto pos = base().find(s);
        if(pos == npos){
            return npos;
        }
        size_t n = 0;
        
        auto iter = impl_begin();
        while(iter != impl_end()){
            if(iter - impl_begin() == pos){
                return n;
            }
            utf8::unchecked::next(iter);
            ++n;
        }
        return npos;
    }
    u8string u8string_view::strip() const{
        if(empty()){
            return {};
        }
        auto ed = impl_end() - 1;
        auto bg = impl_begin();
        while(std::isspace(*bg) and bg != ed){
            ++bg;
        }
        while(std::isspace(*ed) and bg != ed){
            --ed;
        }
        return u8string(bg,ed - bg + 1);
    }
    u8string u8string_view::tolower() const{
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
    u8string u8string_view::toupper() const{
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
    //Check
    bool u8string_view::isalpha() const{
        for(char32_t ch:*this){
            if(not std::isalpha(ch)){
                return false;
            }
        }
        return true;
    }
    //Make substr
    u8string_view u8string_view::substr(size_t pos,size_t len) const{
        size_t dis = 0;//< The len disntance
        size_t beg = 0;//The pos point the char begin
        //For to end
        _const_iterator end;
        _const_iterator iter = impl_begin();
        //Find current position
        iter = Utf8Advance(impl_begin(),impl_end(),iter,pos);
        if(iter == nullptr){
            throwRuntimeError("Invalid pos");
        }
        //Seek to the sub string's end
        if(len == npos){
            //Point to end
            end = impl_end();
        }
        else{
            end = Utf8Advance(impl_begin(),impl_end(),iter,len);
            if(end == nullptr){
                throwRuntimeError("Invalid pos");
            }
        }
        beg = iter - impl_begin();
        dis = end - iter;
        return raw_substr(beg,dis);
    }
    /**
     * @brief Split string into buffer
     * 
     * @tparam Elem 
     * @param str 
     * @param delim 
     * @param max 
     * @return std::vector<Elem> 
     */
    template<class Elem>
    static std::vector<Elem> split_impl(
        std::string_view str,
        std::string_view delim,
        size_t max
    ){
        std::vector<Elem> vec;
        size_t pos = 0;
        size_t next;
        if(max == size_t(-1)){
            //No limited
            while(pos < str.length()){
                next = str.find_first_of(delim,pos);
                if(next == str.npos){
                    vec.emplace_back(&(str[pos]),str.length() - pos);
                    break;
                }
                else{
                    vec.emplace_back((&str[pos]),next - pos);
                }
                pos = next + 1;
            }
        }
        else{
            while(pos < str.length() and vec.size() < max){
                next = str.find_first_of(delim,pos);
                if(next == str.npos){
                    vec.emplace_back(&(str[pos]),str.length() - pos);
                    break;
                }
                else{
                    vec.emplace_back((&str[pos]),next - pos);
                }
                pos = next + 1;
            }
        }
        return vec;
    }
    StringList u8string_view::split(u8string_view delim,size_t max) const{
        return split_impl<u8string>(*this,delim,max);
    }
    StringRefList u8string_view::split_ref(u8string_view delim,size_t max) const{
        return split_impl<u8string_view>(*this,delim,max);
    }
    std::string u8string_view::to_locale() const{
        std::string s;
        sdl_getlocale<std::string>(s);
        auto icd = iconv_open(s.c_str(),"UTF-8");
        if(icd == nullptr){
            throwSDLError();
        }
        s.clear();
        iconv_string<std::string>(icd,s,data(),size());
        iconv_close(icd);
        return s;
    }
}