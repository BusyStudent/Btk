#if !defined(_BOX_FONT_HPP_)
#define _BOX_FONT_HPP_
#include <cstddef>
#include <string>
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
            /**
             * @brief Construct a new Font object
             * 
             * @param fontname Font name
             * @param ptsize Font ptsize
             */
            Font(std::string_view fontname,int ptsize);
            ~Font();
            /**
             * @brief Render solid text
             * 
             * @note This one is fastest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_solid(std::string_view text,Color color);
            /**
             * @brief Render shaded text
             * 
             * @note This one is middle speed
             * @param text Your text
             * @param fg Text color
             * @param bg BackGround color
             * @return PixBuf 
             */
            PixBuf render_shaded(std::string_view text,Color fg,Color bg);
            /**
             * @brief Render blended text
             * 
             * @note This one is slowest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_blended(std::string_view text,Color color);
            /**
             * @brief Get font ptsize
             * 
             * @return int 
             */
            int  ptsize() const noexcept;
            /**
             * @brief Get font refcount
             * 
             * @return int 
             */
            int  refcount() const noexcept;
            /**
             * @brief Set the font ptsize
             * 
             * @param new_ptsize New ptsize
             */
            void set_ptsize(int new_ptsize);
            /**
             * @brief Close font
             * 
             */
            void close();
            /**
             * @brief Open font by its name
             * 
             * @param fontname Font name
             * @param ptsize Font ptsize
             */
            void open(std::string_view fontname,int ptsize);
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             */
            void openfile(std::string_view filename,int ptsize);
            /**
             * @brief Clone a font
             * 
             * @return Font 
             */
            Font clone() const;
            FontImpl *impl() const noexcept{
                return pimpl;
            }

            /**
             * @brief Assign font
             * 
             * @return *this 
             */
            Font &operator =(const Font &);
            Font &operator =(Font &&);
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             * @return Font 
             */
            static Font FromFile(std::string_view filename,int ptsize);
        private:
            Font(FontImpl *i):pimpl(i){};
            FontImpl *pimpl;
    };
    class FontSet{

    };
    /**
     * @brief Some useful function about font
     * 
     */
    namespace FontUtils{
        /**
         * @brief Get font file by its name
         * 
         * @param name font name
         * @return std::string 
         */
        std::string GetFileByName(std::string_view name);
        /**
         * @brief Init font utils
         * 
         */
        void Init();
        /**
         * @brief Quit font utils
         * 
         */
        void Quit();
    };
};


#endif // _BOX_FONT_HPP_
