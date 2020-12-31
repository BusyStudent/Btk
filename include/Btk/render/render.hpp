#if !defined(_BTK_RENDERER_HPP_)
#define _BTK_RENDERER_HPP_
#include "../pixels.hpp"
#include "../rect.hpp"
namespace Btk{
    /**
     * @brief Abstruct Renderer
     * 
     */
    class Renderer{
        public:
            Renderer() = default;
            Renderer(const Renderer &) = delete;
        public:
            //Some virtual draw methods
            virtual int lines(int x1,int y1,int x2,int y2) = 0;
            virtual int lines(int x1,int y1,int x2,int y2) = 0;
            virtual int rects(const Rect *rect,int n) = 0;
            virtual int rect(const Rect &rect) = 0;
            virtual int box(const Rect &rect) = 0;
        public:
            /**
             * @brief Draw AAline
             * 
             * @param x1 The line's x1
             * @param y1 The line's y1
             * @param x2 The line's x2
             * @param y2 The line's y2
             * @param c The line's color
             * @return int 
             */
            int aaline(int x1,int y1,int x2,int y2,Color c);

            virtual int line();
            
            virtual int rect(const Rect &rect);
            int rect(const Rect &rect,Color c);

            virtual int box(const Rect &rect,Color c);
            //virtual int control(int code,...);
            virtual ~Renderer();
        private:
            void query_texture();
            void delete_texture(void *texture);
    };
    
};


#endif // _BTK_RENDERER_HPP_
