#if !defined(_BOX_FONT_HPP_)
#define _BOX_FONT_HPP_
#include <cstddef>
#include <string_view>
#include "pixels.hpp"
namespace Btk{
    struct FontImpl;
    class Font{
        public:
            //empty font
            Font():pimpl(nullptr){};
            Font(const Font &font);
            Font(Font &&f){
                pimpl = f.pimpl;
                f.pimpl = nullptr;
            };
            ~Font();
            //render text
            //fastest
            Surface render_solid(std::string_view text,Color color);
            //mid
            Surface render_shaded(std::string_view text,Color fg,Color bg);
            //best
            Surface render_blended(std::string_view text,Color color);
            //Get font information
            int ptsize() const noexcept;
            
            FontImpl *impl() const noexcept{
                return pimpl;
            }
            //Open font from file
            static Font FromFile(std::string_view filename,int ptsize);
        private:
            Font(FontImpl *i):pimpl(i){};
            FontImpl *pimpl;
    };
    extern int  InitFontSystem();
};


#endif // _BOX_FONT_HPP_
