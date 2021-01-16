#if !defined(_BTK_RWOPS_HPP_)
#define _BTK_RWOPS_HPP_
#include <iosfwd>
#include <cstdint>
#include <cstddef>
#include "defs.hpp"
struct SDL_RWops;
namespace Btk{
    //A Simple wrapper for SDL_RWOps
    class BTKAPI RWops{
        public:
            RWops(SDL_RWops *r):fptr(r){};
            RWops(const RWops &) = delete;
            RWops(RWops && rw){
                fptr = rw.fptr;
                rw.fptr = nullptr;
            }
            ~RWops();
            SDL_RWops *get() const noexcept{
                return fptr;
            }
            //something
            bool close();
            
            static RWops FromStdIstream(std::istream &);
            static RWops FromStdOstream(std::ostream &);
            static RWops FromStdFstream(std::fstream &);
            static RWops FromFile(const char *fname,const char *modes);
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
            int64_t tellp() const noexcept;
            //seek
            int64_t seek(int64_t offset,int whence);
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
            uint8_t *buf_base;//begining of buffer
            uint8_t *buf_ptr;//current pos
            uint8_t *buf_end;//end of buffer
    };
} // namespace Btk

#endif // _BTK_RWOPS_HPP_
