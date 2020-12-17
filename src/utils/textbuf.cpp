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

    template<class T>
    T *do_realloc(T *ptr,size_t new_size){
        T *new_ptr = static_cast<T*>(realloc(ptr,new_size));
        if(new_ptr == nullptr){
            throw std::bad_alloc();
        }
        return new_ptr;
    }
    template<class T>
    T *do_malloc(size_t new_size){
        T *ptr = static_cast<T*>(malloc(new_size));
        if(ptr == nullptr){
            throw std::bad_alloc();
        }
        return ptr;
    }
    void do_free(void *ptr){
        if(ptr != nullptr){
            free(ptr);
        }
    }
    
    char16_t *alloc_string(size_t len){
        return do_malloc<char16_t>((len + 1) * sizeof(char16_t));
    }
    char16_t *realloc_string(char16_t *ptr,size_t newlen){
        return do_realloc<char16_t>(ptr,(newlen + 1) * sizeof(char16_t));
    }
};
namespace Btk{
    TextBuffer::~TextBuffer(){
        do_free(mem);
    }
    TextBuffer::TextBuffer(const TextBuffer &buf){
        if(buf.mem != nullptr){
            //The text's size
            size_t bufsize = (buf.len + 1)* sizeof(char16_t);

            mem = do_malloc<char16_t>(bufsize);
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
            mem = do_realloc(mem,(new_max_len+ 1) * sizeof(char16_t));
            max_len = new_max_len;
        }
        memcpy(mem + len,text,length * sizeof(char16_t));
        len += length;
        mem[len] = u'\0';
    }
    void TextBuffer::append(const char *text,size_t length){
        //FIXME I donnot why iconv will skip 2 byte a the buffer begin
        //Is this bug?
        BTK_ASSERT(text != nullptr);

        if(mem == nullptr){
            //We papre some spaces
            mem = alloc_string(length + 2);
            max_len = length + 2;
        }

        size_t old_len = len;//restore the len
        size_t ret;//iconv ret val

        const char *inbuf  = text;//input buffer
        size_t tsize = length * sizeof(char);//<text size

        char16_t *cur = mem + len;//<current ptr
        size_t cur_size = (max_len - len) * sizeof(char16_t);//buffer size
        
        
        while(tsize > 0){

            ret = u16converter.iconv(&inbuf,&tsize,
                reinterpret_cast<char**>(&cur),&cur_size);
            
            if(ret == SDL_ICONV_E2BIG){
                //buffer is too small
                size_t cur_len = cur - mem;//restore current length

                mem = realloc_string(mem,max_len + tsize);

                max_len += tsize;
                cur_size = (max_len - len) * sizeof(char16_t);
                cur = mem + cur_len;

                len = cur_len;
            }
            else if(ret == SDL_ICONV_EILSEQ){
                //got error terminate the string
                err:
                mem[old_len] = u'\0';
                u16converter.reset();
                throwSDLError();
            }
            else if(ret == SDL_ICONV_EINVAL){
                //Some inval chars
                //Skip it
                ++inbuf;
                --tsize;
            }
            else if(ret == SDL_ICONV_ERROR){
                goto err;
            }
        }
        //terminate the string
        u16converter.reset();
        len = cur - mem;
        mem[len] = u'\0';
    }
    void TextBuffer::resize(size_t new_len){
        if(new_len == 0){
            free(mem);
            len = 0;
            max_len = 0;
            return;
        }
        if(new_len < len){
            //be smaller
            mem = do_realloc(mem,(new_len + 1) * sizeof(char16_t));
            mem[new_len] = u'\0';

            max_len = new_len;
            len = new_len;
        }
        else if(new_len == len or new_len == max_len){
            //do no op
        }
        else{
            //expand or be smaller
            //It is safe to not add zero terminater
            mem = do_realloc(mem,(new_len + 1) * sizeof(char16_t));
            max_len = new_len;
        }
    }
    std::string TextBuffer::to_string() const{
        if(len == 0){
            return std::string();
        }
        std::string str;
        str.resize(len * 2);

        const char16_t *inbuf  = mem;//input buffer
        size_t tsize = len * sizeof(char16_t);//<text size

        char *cur = str.data();//<current ptr
        size_t cur_size = str.size();//buffer size
        
        size_t length = 0;
        size_t ret;
        
        while(tsize > 0){

            ret = u8converter.iconv(
                reinterpret_cast<const char**>(&inbuf),
                &tsize,
                &cur,
                &cur_size);
            
            if(ret == SDL_ICONV_E2BIG){
                //buffer is too small
                size_t cur_len = cur - str.data();//restore current length

                str.resize(str.length() + tsize);
                cur_size = (max_len - len) * sizeof(char16_t);
                cur = str.data() + cur_len;

                length += cur_len;
            }
            else if(ret == SDL_ICONV_EILSEQ){
                //got error terminate the string
                err:
                u8converter.reset();
                throwSDLError();
            }
            else if(ret == SDL_ICONV_EINVAL){
                //Some inval chars
                //Skip it
                ++inbuf;
                --tsize;
            }
            else if(ret == SDL_ICONV_ERROR){
                goto err;
            }
        }
        //terminate the string
        u8converter.reset();
        str.shrink_to_fit();
        return str;
    }
    void TextBuffer::clear(){
        if(mem != nullptr){
            mem[0] = u'\0';
        }
        len = 0;
        max_len = 0;
    }
    void TextBuffer::assign(const TextBuffer &buffer){
        if(&buffer == this){
            return;
        }
        if(buffer.len == 0){
            //the buffer is empty
            len = 0;
            if(mem != nullptr){
                mem[0] = u'\0';
            }
            return;
        }
        char16_t *new_ptr = alloc_string(buffer.len);
        len = buffer.len;
        max_len = buffer.len;
        do_free(mem);
        mem = new_ptr;
    }
    void TextBuffer::assign(TextBuffer &&buffer){
        if(&buffer == this){
            return;
        }
        if(buffer.len == 0){
            //the buffer is empty
            len = 0;
            if(mem != nullptr){
                mem[0] = u'\0';
            }
            return;
        }
        if(len == 0){
            //We can move data from the buffer
            do_free(mem);
            mem = buffer.mem;
            len = buffer.len;
            max_len = buffer.max_len;
        }
        else{
            //copy the data
            char16_t *new_ptr = alloc_string(buffer.len);
            len = buffer.len;
            max_len = buffer.len;
            do_free(mem);
            mem = new_ptr;
            memcpy(mem,buffer.mem,(buffer.len + 1) * sizeof(char16_t));
        }
    }
    void TextBuffer::extend(size_t new_len){
        if(new_len > max_len){
            mem = realloc_string(mem,new_len);
            max_len = new_len;
        }
    }
    void TextBuffer::shrink_to_fit(){
        if(max_len != len){
            mem = realloc_string(mem,len);
            max_len = len;
        }
    }
};