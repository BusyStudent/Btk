#if !defined(_BTK_RWOPS_HPP_)
#define _BTK_RWOPS_HPP_
#include <iosfwd>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include "defs.hpp"
#include <SDL2/SDL_rwops.h>
namespace Btk{
    //A Simple wrapper for SDL_RWops
    class MemBuffer;
    class BTKAPI RWops{
        public:
            RWops(SDL_RWops *r):fptr(r){};
            RWops(const RWops &) = delete;
            RWops(MemBuffer &&) = delete;
            RWops(RWops && rw){
                fptr = rw.fptr;
                rw.fptr = nullptr;
            }
            ~RWops();
            SDL_RWops *get() const noexcept{
                return fptr;
            }
            /**
             * @brief Close the rwops
             * 
             * @return true 
             * @return false 
             */
            bool close();
            size_t write(const void *buf,size_t size,size_t n){
                return SDL_RWwrite(fptr,buf,size,n);
            }
            size_t read(void *buf,size_t size,size_t n){
                return SDL_RWread(fptr,buf,size,n);
            }
            size_t tell() const{
                return SDL_RWtell(fptr);
            }
            size_t size() const{
                return SDL_RWsize(fptr);
            }
            size_t seek(Sint64 offset,int whence){
                return SDL_RWseek(fptr,offset,whence);
            }
            RWops &operator =(RWops &&);
            RWops &operator =(MemBuffer &&) = delete;

            static RWops FromStdIstream(std::istream &);
            static RWops FromStdOstream(std::ostream &);
            static RWops FromStdFstream(std::fstream &);
            static RWops FromFile(const char *fname,const char *modes);
            static RWops FromMem(const void *mem,size_t n);
            static RWops FromFP(FILE *fp,bool autoclose = true);
            static RWops FromFD(int fd,const char *modes);
        private:
            SDL_RWops *fptr;
        friend class MemBuffer;
    };
    //Memory buffer
    class BTKAPI MemBuffer:public RWops{
        public:
            MemBuffer();
            //MemBuffer(const MemBuffer &);
            MemBuffer(MemBuffer &&);
            ~MemBuffer();
            //tell current pos
            Sint64 tellp() const noexcept;
            //seek
            Sint64 seek(int64_t offset,int whence);
            //RW
            size_t  write(const void *buf,size_t num,size_t n);
            size_t  read(void *buf,size_t num,size_t n);
            //Get size
            size_t  size() const noexcept{
                return buf_ptr - buf_base;
            };
            size_t  capcitity() const noexcept{
                return buf_end - buf_base;
            };
            //write all data out
            friend std::ostream &operator <<(std::ostream&,const MemBuffer&);
        private:
            Uint8 *buf_base;//begining of buffer
            Uint8 *buf_ptr;//current pos
            Uint8 *buf_end;//end of buffer
    };
    /**
     * @brief Create a two binary pipes
     * 
     * @param r
     * @param w 
     */
    BTKAPI void CreatePipe(RWops &r,RWops &w);
} // namespace Btk

#endif // _BTK_RWOPS_HPP_
