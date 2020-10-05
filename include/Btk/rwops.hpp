#if !defined(_BTK_RWOPS_HPP_)
#define _BTK_RWOPS_HPP_
#include <iosfwd>
//A Simple wrapper for SDL_RWOps
struct SDL_RWops;
namespace Btk{
    class RWops{
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
        private:
            SDL_RWops *fptr;
    };
} // namespace Btk

#endif // _BTK_RWOPS_HPP_
