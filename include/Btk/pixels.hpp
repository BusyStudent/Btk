#if !defined(_BTK_PIXELS_HPP_)
#define _BTK_PIXELS_HPP_
#include <string_view>
#include <cstdlib>
#include <cstddef>
#include <iosfwd>
#include <SDL2/SDL_pixels.h>
struct SDL_Surface;
struct SDL_Texture;
namespace Btk{
    //Surface 
    class RWops;
    //like sdl color structure
    typedef SDL_Color Color;
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
            //operators

            //save surface
            void save_png(RWops &,int quality);
            void save_jpg(RWops &,int quality);
            void save_bmp(RWops &);

            void save_png(std::string_view fname,int quality);
            void save_jpg(std::string_view fname,int quality);
            void save_bmp(std::string_view fname);
            
            Surface &operator =(SDL_Surface *);//assign
            Surface &operator =(Surface &&);//assign
            //check empty
            bool empty() const noexcept{
                return surf == nullptr;
            }
            SDL_Surface *operator ->() const noexcept{
                return surf;
            }
            //Some static method to load image
            static Surface FromFile(std::string_view file);
            static Surface FromFile(FILE *f);
            static Surface FromMem(const void *mem,size_t size);
            static Surface FromRWops(RWops &);
            static Surface FromXPMArray(char **);
        private:
            SDL_Surface *surf;
    };
    //RendererTexture
    class Texture{
        public:
            Texture(SDL_Texture *t = nullptr):texture(t){};
            Texture(const Texture &) = delete;
            Texture(Texture &&t){
                texture = t.texture;
                t.texture = nullptr;
            }
            ~Texture();
            int w() const;
            int h() const;
            //check is empty
            bool empty() const noexcept{
                return texture == nullptr;
            }
            //assign
            Texture &operator =(SDL_Texture*);
            Texture &operator =(Texture &&);
            //Get pointer
            SDL_Texture *get() const noexcept{
                return texture;
            }
        private:
            SDL_Texture *texture;
        friend struct Renderer;
    };
    typedef Surface PixBuf;
};

#endif // _BTK_PIXELS_HPP_
