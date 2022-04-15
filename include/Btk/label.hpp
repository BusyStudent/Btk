#if !defined(_BTK_LABEL_HPP_)
#define _BTK_LABEL_HPP_
#include <string>
#include <string_view>
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    /**
     * @brief Label widget
     * 
     */
    class BTKAPI Label:public Widget{
        public:
            Label();
            Label(u8string_view text);
            /**
             * @brief Construct a new Label object
             * 
             * @param x Label x
             * @param y Label y
             * @param w Label w
             * @param h Label h
             */
            Label(int x,int y,int w,int h);
            ~Label();
            u8string text() const{
                return text_;
            }
            //Set Label text
            void set_text(u8string_view text);
            void draw(Renderer&,Uint32) override;
        private:
            u8string text_;//text
            
            //PixBuf text_buf;//text pixels buf
            //Texture texture;//text texture
            Color text_color = {0,0,0,255};//text color

            Align align = Align::VCenter | Align::Left;
    };
    class BTKAPI TextView:public Widget{
        
    };
}

#endif // _BTK_LABEL_HPP_
