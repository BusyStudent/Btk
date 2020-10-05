#include <SDL2/SDL.h>
#include <Btk/rwops.hpp>
#include <istream>
#include <ostream>
#include <fstream>
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
            return current - last / num;
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
    //close rwops
    bool RWops::close(){
        if(fptr != nullptr){
            bool ret = (SDL_RWclose(fptr) == 0);
            fptr = nullptr;
            return ret;
        }
        
        return false;
    }
};