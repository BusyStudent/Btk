#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include "rect.hpp"
#include "widget.hpp"
#include "container.hpp"

//< Key name for Layout item
#define BTK_LAYOUT_ITEMPTR "_item_"

namespace Btk{
    /**
     * @brief Layout interface
     * 
     */
    class BTKAPI Layout:public Group{
        public:
            Layout();
            ~Layout();

            bool handle(Event &) override;
            void draw(Renderer &,Uint32) override;
            void set_rect(const Rect &) override;


            /**
             * @brief Item for Layout
             * 
             */
            struct Item{
                Widget *widget = nullptr;//< The widget associated with this item

                float stretch = 1.0f;//< The stretch factor of this item

                //< Resulotion of the widget
                float w = -1,h = -1;
                float x = -1,y = -1;

                //TODO: Need support for min/max size
                //TODO: Need support for fixed size

                //< The fixed_size
                FSize fixed_size = {
                    -1.0f,//< -1 means no fixed size on horizontal
                    -1.0f//< -1 means no fixed size on vertical
                };
            };
            //override method we need
            using Group::add;

            bool add(Widget *) override;
            bool detach(Widget *) override;
            void clear() override;

            //For lower / raise
            void lower_widget(Widget *w) override;
            void raise_widget(Widget *w) override;

            /**
             * @brief Ask the layout the data is dirty
             * 
             */
            void invalidate();
            /**
             * @brief Set the content margins object
             * 
             * @param left 
             * @param top 
             * @param right 
             * @param bottom 
             */
            void set_content_margins(float left,float top,float right,float bottom){
                margin.left = left;
                margin.top = top;
                margin.right = right;
                margin.bottom = bottom;
                invalidate();
            }
            void set_content_margins(const FMargin &m){
                margin = m;
                invalidate();
            }
        protected:
            /**
             * @brief Update the layout
             * 
             */
            virtual void update() = 0;
            /**
             * @brief Alloc a new item for the widget
             * 
             * @param w 
             * @return Item* 
             */
            virtual Item *alloc_item(Widget *w) = 0;
            /**
             * @brief Get item by index
             * 
             * @param index (negative index is not allowed)
             * @return Item* 
             */
            Item *index_item(Uint32 index);
            /**
             * @brief Get item by widget
             * 
             * @param w The widget ptr
             * @return Item* 
             */
            Item *index_item(Widget *w){
                if(w == nullptr){
                    return nullptr;
                }
                return static_cast<Item*>(w->userdata(BTK_LAYOUT_ITEMPTR));
            }
            void  clear_items();

            std::list<Item*> items;//< The items of the layout
            FMargin margin = {0.0f,0.0f,0.0f,0.0f};//< The content margins
        private:
            bool is_dirty = true;
    };
    /**
     * @brief A Box Container
     * 
     */
    class BTKAPI BoxLayout:public Layout{
        public:
            enum Direction:Uint8{
                LeftToRight,//Horizontal
                RightToLeft,
                TopToBottom,//Vertical
                BottomToTop
            };
        public:
            BoxLayout(Direction d);
            ~BoxLayout();

            // bool add(Widget *w) override;
            /**
             * @brief Add a stretch
             * 
             * @param stretch 
             */
            void add_stretch(float stretch = 1.0f);
            
            void set_direction(Direction direction);
            void set_spacing(float spacing);
            
            Direction direction() const noexcept{
                return _direction;
            }
            float spacing() const noexcept{
                return _spacing;
            }
            bool is_horizontal() const noexcept{
                return _direction == LeftToRight or _direction == RightToLeft;
            }
            bool is_vertical() const noexcept{
                return _direction == TopToBottom or _direction == BottomToTop;
            }
            /**
             * @brief Set the stretch object of widget
             * 
             */
            void set_stretch(Widget *w,float stretch);
            void set_stretch(Widget &w,float stretch){
                set_stretch(&w,stretch);
            }
            /**
             * @brief Set the fixed size object of widget
             * 
             * @param w The widget ptr
             * @param fixed_width width(-1 on auto)
             * @param fixed_height width(-1 on auto)
             */
            void set_fixed_size(Widget *w,float fixed_width,float fixed_height);
            void set_fixed_size(Widget &w,float fixed_width,float fixed_height){
                set_fixed_size(&w,fixed_width,fixed_height);
            }
            /**
             * @brief Reset the fixed size object of widget
             * 
             * @param w 
             */
            void reset_fixed_size(Widget *w){
                set_fixed_size(w,-1,-1);
            }
            void reset_fixed_size(Widget &w){
                set_fixed_size(&w,-1,-1);
            }
        private:
            void  update() override;
            Item *alloc_item(Widget *w) override;

            bool lazy = true;

            Direction _direction;
            float _spacing = 0;
    };
    /**
     * @brief Vertical Box Layout(default direction is TopToBottom)
     * 
     */
    class BTKAPI VBoxLayout:public BoxLayout{
        public:
            VBoxLayout();
            ~VBoxLayout();
    };
    /**
     * @brief Horizontal Box Layout(default direction is LeftToRight)
     * 
     */
    class BTKAPI HBoxLayout:public BoxLayout{
        public:
            HBoxLayout();
            ~HBoxLayout();
    };
    /**
     * @brief Grid Layout
     */
    class BTKAPI GridLayout:public Layout{
        public:
            GridLayout();
            GridLayout(const GridLayout &) = delete;
            ~GridLayout();

            void update();
            bool add_row(Widget *widget);
            void add();
        private:
            
    };
};


#endif // _BTK_LAYOUT_HPP_
