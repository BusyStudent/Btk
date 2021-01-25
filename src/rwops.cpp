#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/exception.hpp>
#include <Btk/rwops.hpp>
#include <cstring>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <fstream>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <cerrno>
    #define BTK_FDOPEN _fdopen
#else
    #include <cerrno>
    #include <unistd.h>
    #define BTK_FDOPEN fdopen
#endif


namespace{
    //some functions
    int close_rwops(SDL_RWops *context){
        SDL_FreeRW(context);
        return 0;
    }
    size_t unsupported_write(SDL_RWops *context,const void *buf,size_t num,size_t n){
        #ifndef NDEBUG
        //for debugging
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::RWops]Context %p unsupport write(%p,%zu,%zu)",context,buf,num,n);
        #endif
        SDL_SetError("Context %p unsupport write(%p,%zu,%zu)",context,buf,num,n);
        return 0;
    }
    size_t unsupported_read(SDL_RWops *context,void *buf,size_t num,size_t n){
        #ifndef NDEBUG
        //for debugging
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::RWops]Context %p unsupport read(%p,%zu,%zu)",context,buf,num,n);
        #endif
        SDL_SetError("Context %p unsupport read(%p,%zu,%zu)",context,buf,num,n);
        return 0;
    }
    Sint64 unknown_size(SDL_RWops *){
        return 0;
    }
    //some templates for std stream
    template<class T>
    size_t stream_read(SDL_RWops *context,void *buf,size_t num,size_t n){
        T* str = static_cast<T*>(context->hidden.unknown.data1);
        str->read(static_cast<char*>(buf),num * n);
        if(str->bad()){
            return 0;
        }
        else{
            return str->gcount() / num;
        }
    }
    template<class T>
    size_t stream_write(SDL_RWops *context,const void *buf,size_t num,size_t n){
        T* str = static_cast<T*>(context->hidden.unknown.data1);

        auto last = str->tellp();

        str->write(static_cast<const char*>(buf),num * n);
        if(str->bad()){
            return 0;
        }
        else{
            auto current = str->tellp();
            return (current - last) / num;
        }
    }

    Sint64 istream_seek(SDL_RWops *context,Sint64 offset,int whence){
        std::istream* str = static_cast<std::istream*>(context->hidden.unknown.data1);
            switch(whence){
                case RW_SEEK_SET:
                    str->seekg(offset,std::ios::beg);
                    break;
                case RW_SEEK_CUR:
                    str->seekg(offset,std::ios::cur);
                    break;
                case RW_SEEK_END:
                    str->seekg(offset,std::ios::end);
                    break;
            }
            if(str->bad()){
                return -1;
            }
            else{
                return str->tellg();
            }
    }
    Sint64 ostream_seek(SDL_RWops *context,Sint64 offset,int whence){
        std::ostream* str = static_cast<std::ostream*>(context->hidden.unknown.data1);
            switch(whence){
                case RW_SEEK_SET:
                    str->seekp(offset,std::ios::beg);
                    break;
                case RW_SEEK_CUR:
                    str->seekp(offset,std::ios::cur);
                    break;
                case RW_SEEK_END:
                    str->seekp(offset,std::ios::end);
                    break;
            }
            if(str->bad()){
                return -1;
            }
            else{
                return str->tellp();
            }
    }
    Sint64 fstream_seek(SDL_RWops *context,Sint64 offset,int whence){
        std::fstream* str = static_cast<std::fstream*>(context->hidden.unknown.data1);
            switch(whence){
                case RW_SEEK_SET:
                    str->seekg(offset,std::ios::beg);
                    str->seekp(offset,std::ios::beg);
                    break;
                case RW_SEEK_CUR:
                    str->seekg(offset,std::ios::cur);
                    str->seekp(offset,std::ios::cur);
                    break;
                case RW_SEEK_END:
                    str->seekg(offset,std::ios::end);
                    str->seekp(offset,std::ios::end);
                    break;
            }
            if(str->bad() or str->tellg() != str->tellp()){
                return -1;
            }
            else{
                return str->tellp();
            }
    }
};
namespace Btk{
    RWops::~RWops(){
        if(fptr != nullptr){
            SDL_RWclose(fptr);
        }
    }
    //open std istream
    RWops RWops::FromStdIstream(std::istream &istr){
        SDL_RWops *fptr = SDL_AllocRW();
        fptr->type = SDL_RWOPS_UNKNOWN;

        fptr->hidden.unknown.data1 = &istr;
        fptr->close = close_rwops;
        fptr->size = unknown_size;
        fptr->read = stream_read<std::istream>;
        fptr->write = unsupported_write;
        fptr->seek = istream_seek;
        return fptr;
    }
    //Opem ostream
    RWops RWops::FromStdOstream(std::ostream &ostr){
        SDL_RWops *fptr = SDL_AllocRW();
        fptr->type = SDL_RWOPS_UNKNOWN;

        fptr->hidden.unknown.data1 = &ostr;
        fptr->close = close_rwops;
        fptr->size = unknown_size;
        fptr->read = unsupported_read;
        fptr->write = stream_write<std::ostream>;
        fptr->seek = ostream_seek;
        
        return fptr;
    }
    //OpenFstream
    RWops RWops::FromStdFstream(std::fstream &fstr){
        SDL_RWops *fptr = SDL_AllocRW();
        fptr->type = SDL_RWOPS_UNKNOWN;

        fptr->hidden.unknown.data1 = &fstr;
        fptr->close = close_rwops;
        fptr->size = unknown_size;
        fptr->read = stream_read<std::fstream>;
        fptr->write = stream_write<std::fstream>;
        fptr->seek = fstream_seek;
        
        return fptr;
    }
    RWops RWops::FromFile(const char *fname,const char *modes){
        SDL_RWops *rw = SDL_RWFromFile(fname,modes);
        if(rw == nullptr){
            throwSDLError();
        }
        return rw;
    }
    RWops RWops::FromFD(int fd,const char *modes){
        FILE *fp = BTK_FDOPEN(fd,modes);
        if(fp == nullptr){
            throwRuntimeError(cformat("fdopen %d:%s",fd,strerror(errno)).c_str());
        }
        SDL_RWops *rw = SDL_RWFromFP(fp,SDL_TRUE);
        if(rw == nullptr){
            throwSDLError();
        }
        return rw;
    }
    //close rwops
    bool RWops::close(){
        if(fptr != nullptr){
            bool ret = (SDL_RWclose(fptr) == 0);
            fptr = nullptr;
            return ret;
        }
        
        return false;
    }
    RWops &RWops::operator =(RWops &&rwops){
        if(&rwops != this){
            close();
            fptr = rwops.fptr;
            rwops.fptr = nullptr;
        }
        return *this;
    }
    //Function to cast 
    inline MemBuffer &GetMemBuffer(SDL_RWops *ctxt){
        return *static_cast<MemBuffer*>(
            ctxt->hidden.unknown.data1
        );
    }
    //MemBuffer
    MemBuffer::MemBuffer():
        RWops(SDL_AllocRW()){
        
        buf_base = nullptr;
        buf_ptr = nullptr;
        buf_end = nullptr;
        //set current
        //use it to set object pointer
        fptr->hidden.unknown.data1 = this;

        fptr->type = SDL_RWOPS_UNKNOWN;
        fptr->size = [](SDL_RWops *ctxt) -> Sint64{
            //return buffer capcitity
            return GetMemBuffer(ctxt).capcitity();
        };
        fptr->seek = [](SDL_RWops *ctxt,Sint64 offset,int whence) -> Sint64{
            return GetMemBuffer(ctxt).seek(offset,whence);
        };
        fptr->write = [](SDL_RWops *ctxt,const void *buf,size_t size,size_t n) -> size_t{
            return GetMemBuffer(ctxt).write(buf,size,n);
        };
        fptr->read  = [](SDL_RWops *ctxt,void *buf,size_t size,size_t n) -> size_t{
            return GetMemBuffer(ctxt).read(buf,size,n);
        };
        fptr->close = [](SDL_RWops *ctxt) -> int{
            MemBuffer &buf = GetMemBuffer(ctxt);
            if(buf.buf_base != nullptr){
                //Free memory
                std::free(buf.buf_base);
            }
            buf.buf_base = nullptr;
            buf.buf_ptr = nullptr;
            buf.buf_end = nullptr;
            return true;
        };
    }
    MemBuffer::MemBuffer(MemBuffer &&buf):RWops(static_cast<RWops&&>(buf)){
        
        fptr->hidden.unknown.data1 = this;
    }
    MemBuffer::~MemBuffer(){

    }
    //methods
    Sint64 MemBuffer::tellp() const noexcept{
        return buf_ptr - buf_base;
    }
    size_t MemBuffer::write(const void *buf,size_t num,size_t n){
        size_t size = num * n;
        if(buf_ptr + size > buf_end){
            //overflow
            //begin realloc
            
            size_t cur = buf_ptr - buf_base;//restore current pos
            size_t end = buf_ptr - buf_base;//restore end pos

            uint8_t *new_ptr = (uint8_t*) realloc(buf_base,size + capcitity());
            if(new_ptr == nullptr){
                //failed
                SDL_OutOfMemory();
                return 0;
            }
            else{
                buf_base = new_ptr;

                buf_ptr = buf_base + cur;
                buf_end = buf_base + end;
            }
        }
        //memcpy
        memcpy(buf_ptr,buf,size);

        buf_ptr += size;

        return n;
    }
    size_t MemBuffer::read(void *buf,size_t size,size_t n){
        size_t rest = buf_end - buf_ptr;
        //rest data blocks
        if(rest == 0){
            //doesnnot have data
            SDL_Error(SDL_EFREAD);
            return 0;
        }
        size_t blcoks = rest % size;//useable blocks
        memcpy(buf,buf_ptr,blcoks * size);

        buf_ptr += (blcoks * size);

        return blcoks;
    }

    Sint64 MemBuffer::seek(Sint64 offset,int whence){
        switch(whence){
            case RW_SEEK_CUR:{
                //from current pos
                if(buf_ptr + offset <= buf_end){
                    buf_ptr += offset;
                    //return current pos
                    return buf_ptr - buf_base;
                }
                else{
                    return -1;
                }
            };
            case RW_SEEK_SET:{
                //from buf begin
                if(buf_base + offset <= buf_end){
                    buf_ptr = buf_base + offset;
                    //return current pos
                    return offset;
                }
                else{
                    return -1;
                }
            };
            case RW_SEEK_END:{
                //from buf end
                if(buf_end + offset <= buf_end){
                    buf_ptr += offset;
                    //return current pos
                    return buf_ptr - buf_base;
                }
                else{
                    return -1;
                }
                break;
            };
            default:
                SDL_Unsupported();
                return -1;
        }
    }
    void CreatePipe(RWops &r,RWops &w){
        int fds[2];
        int ret;
        #ifdef _WIN32
        ret = _pipe(fds,1024,_O_BINARY);
        #else
        ret = pipe(fds);
        #endif
        if(ret == -1){
            throwRuntimeError(strerror(errno));
        }
        r = RWops::FromFD(fds[0],"rb");
        w = RWops::FromFD(fds[1],"wb");
    }
    std::ostream &operator <<(std::ostream &str,const MemBuffer &buf){
        if(buf.buf_base != nullptr){
            str.write((char*)buf.buf_base,buf.size());
        }
        return str;
    }
    
};