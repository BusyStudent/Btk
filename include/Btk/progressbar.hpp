#if !defined(_BTK_PROGRESSBAR_HPP_)
#define _BTK_PROGRESSBAR_HPP_
#include "string.hpp"
#include "widget.hpp"
namespace Btk{
    /**
     * @brief ProgressBar
     * 
     */
    class BTKAPI ProgressBar:public Widget{
        public:
            ProgressBar();
            ~ProgressBar();

            void set_value(float value);
            void draw(Renderer &) override;



            Signal<void(float)> &signal_value_changed(){
                return _signal_value_changed;
            }
        private:
            Signal<void(float)> _signal_value_changed;
            u8string value_text = {};
            u8string fmt_string = "%.2f%%";
            float value = 0;
            bool display_value = true;
            bool draw_boarder = true;
            bool draw_background = true;

            Orientation orientation;
            Color background;
            Color boarder;
    };
}

#endif // _BTK_PROGRESSBAR_HPP_
