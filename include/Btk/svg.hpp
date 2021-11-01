#if !defined(_BTK_SVG_HPP_)
#define _BTK_SVG_HPP_
#include "defs.hpp"
#include "widget.hpp"
#include "string.hpp"
#include "exception.hpp"
struct SDL_RWops;
namespace Btk{
    class Renderer;
    class PixBuf;


    // struct BTKAPI SVGPath{
    //     std::vector<float> pts;


    //     void   render(Renderer &renderer) const;
    // };
    // struct BTKAPI SVGShape{
    //     std::list<SVGPath> paths;

    //     void   render(Renderer &renderer) const;
    // };
    // struct BTKAPI SVGDocument{
    //     std::list<SVGShape> shapes;
    //     float w,h;

    //     //method
    //     void   render(Renderer &renderer) const;
    //     PixBuf rasterize() const;
    // };
    class BTKAPI SVGImage{
        public:
            SVGImage() = default;
            SVGImage(const SVGImage &) = delete;
            SVGImage(SVGImage &&i){
                image = i.image;
                i.image = nullptr;
            }
            ~SVGImage(){
                _Free(image);
            }

            void render(Renderer &renderer) const{
                return _Render(image,renderer);
            }
            FSize size() const{
                return _Query(image);
            }
            bool empty() const{
                return image == nullptr;
            }

            SVGImage &operator =(SVGImage &&i){
                if(&i == this){
                    return *this;
                }
                _Free(image);
                image = i.image;
                i.image = nullptr;
                return *this;
            }

            static SVGImage Parse(u8string_view str){
                return _Parse(str.data(),str.size());
            }
            static SVGImage ParseFile(const char *file){
                return _ParseFile(file);
            }
            static SVGImage Parse(u8string_view str,const char *unit,float dpi){
                return _Parse(str.data(),str.size(),unit,dpi);
            }
        private:
            SVGImage(void *i):image(i){};
            /**
             * @brief Parse SVG from Stream
             * 
             * @param filename 
             * @param unit 
             * @param dpi 
             * @return void* 
             */
            static void *_ParseStream(SDL_RWops *rw,const char *unit = nullptr,float dpi = 96);
            static void *_ParseFile(const char *f,const char *unit = nullptr,float dpi = 96);
            /**
             * @brief Parse SVG String
             * 
             * @param s 
             * @param n 
             * @param unit 
             * @param dpi 
             * @return void* 
             */
            static void *_Parse(const char *s,size_t n,const char *unit = nullptr,float dpi = 96);
            static void  _Render(const void *image,Renderer &renderer);
            static void  _Free(void *image);
            static FSize _Query(const void *image);

            void *image = nullptr;
    };
    class BTKAPI SVGView:public Widget{
        public:
            SVGView();
            ~SVGView();

            void draw(Renderer &renderer) override;
            void set_image(SVGImage &&s){
                image = std::move(s);
            }
        private:
            float scale_x = 1,scale_y = 1;
            float tr_x = 0,tr_y = 0;

            bool limited_to_bound = true;
            SVGImage image;
    };
}

#endif // _BTK_SVG_HPP_
