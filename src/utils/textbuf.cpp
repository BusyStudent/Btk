#include "../build.hpp"

#include <SDL2/SDL_stdinc.h>
#include <Btk/utils/textbuf.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/exception.hpp>
#include <new>
namespace{
    using Btk::throwSDLError;
    struct Converter{
        Converter(const char *to,const char *from){
            ptr = SDL_iconv_open(to,from);
            if(ptr == reinterpret_cast<SDL_iconv_t>(-1)){
                throwSDLError();
            }
        };
        Converter(const Converter &) = delete;
        ~Converter(){
            SDL_iconv_close(ptr);
        };
        //start convert
        size_t iconv(const char **inbuf,size_t *inbyte,char **outbuf,size_t *outbyte){
            return SDL_iconv(ptr,inbuf,inbyte,outbuf,outbyte);
        };
        void reset(){
            SDL_iconv(ptr,nullptr,nullptr,nullptr,nullptr);
        };
        SDL_iconv_t ptr;
    };
    /**
     * @brief Convert to utf16
     * 
     */
    static thread_local Converter u16converter("UTF16","UTF8");
    /**
     * @brief Convert to utf8
     * 
     */
    static thread_local Converter u8converter("UTF8","UTF16");
};
namespace Btk{
    TextBuffer::~TextBuffer(){
        if(mem != nullptr){
            free(mem);
        }
    }
    TextBuffer::TextBuffer(const TextBuffer &buf){
        if(buf.mem != nullptr){
            //The text's size
            size_t bufsize = (buf.len + 1)* sizeof(char16_t);

            mem = static_cast<char16_t*>(malloc(bufsize));
            if(mem == nullptr){
                throw std::bad_alloc();
            }
            memcpy(mem,buf.mem,bufsize);

            len = buf.len;
            max_len = buf.len;
        }
        else{
            mem = nullptr;
            len = 0;
            max_len = 0;
        }
    }
    void TextBuffer::append(const char16_t *text,size_t length){
        if(text == nullptr or length == 0){
            return;
        }
        if(len + length > max_len){
            //We did not place to append the text
            //begin realloc
            size_t new_max_len = (len + length);
            char16_t *new_mem = static_cast<char16_t*>(
                realloc(mem,(new_max_len+ 1) * sizeof(char16_t))
            );
            
            if(new_mem == nullptr){
                throw std::bad_alloc();
            }

            max_len = new_max_len;
            mem = new_mem;
        }
        memcpy(mem + len,text,length * sizeof(char16_t));
        len += length;
        mem[len] = '\0';
    }
    void TextBuffer::append(const char *text,size_t length){
        //TODO: using iconv directly
        char * s = SDL_iconv_string("UTF16","UTF8",text,length);
        if(s == nullptr){
            throwSDLError();
        }
        SDLScopePtr ptr(s);
        char16_t *utf16 = reinterpret_cast<char16_t*>(s);
        size_t len = U16Strlen(utf16);
        append(utf16,len);
    }
    std::string TextBuffer::to_string() const{
        if(len == 0){
            return std::string();
        }
        std::string ret;
        ret.resize(len * 2);

        size_t inlen = len;
        //It should be ok to prepare for the double length
        size_t outlen = len * 2;
        
        const char *inbuf = reinterpret_cast<char*>(mem);
        char *outbuf = ret.data();

        size_t byte = u8converter.iconv(&inbuf,&inlen,&outbuf,&outlen);
        switch(byte){
            case SDL_ICONV_ERROR:
            case SDL_ICONV_EINVAL:
            case SDL_ICONV_EILSEQ:
            case SDL_ICONV_E2BIG:
                //Error happended
                u8converter.reset();
                throwSDLError();
        }
        //Add zero terminater
        ret[byte + 1] = '\0';
        ret.resize(byte);
        return ret;
    }
};