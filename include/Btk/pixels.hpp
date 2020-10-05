#if !defined(_BTK_PIXELS_HPP_)
#define _BTK_PIXELS_HPP_
#include <string_view>
#include <cstdlib>
#include <cstddef>
#include <iosfwd>
struct SDL_Surface;
namespace Btk{
    //Surface 
    class RWops;
    //like sdl color structure
    struct Color{
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    class Surface{
        public:
            
            Surface(SDL_Surface *s = nullptr):surf(s){};//empty surface
            //LoadFromFile
            Surface(std::string_view file);
            //Move construct
            Surface(const Surface &) = delete;
            Surface(Surface && s){
                surf = s.surf;
                s.surf = nullptr;
            };
            ~Surface();
            SDL_Surface *get() const noexcept{
                return surf;
            };
            //Get a copy of this surface
            Surface clone() const;
            //Get a ref of this surface
            Surface ref() const;
            //Some static method to load image
            static Surface FromFile(std::string_view file);
            static Surface FromFile(FILE *f);
            static Surface FromMem(const void *mem,size_t size);
            static Surface FromRWops(RWops &);
            static Surface FromXPMArray(char **);
        private:
            SDL_Surface *surf;
    };
    typedef Surface PixBuf;
};

#endif // _BTK_PIXELS_HPP_
