#if !defined(_BTK_CURSOR_HPP_)
#define _BTK_CURSOR_HPP_
struct SDL_Cursor;
namespace Btk{
    class PixBuf;
    /**
     * @brief SystemCursor from SDL_mouse.h
     * 
     */
    enum class SystemCursor{
        Arrow,
        Ibeam,
        Wait,
        Crosshair,
        Hand
    };
    /**
     * @brief Mouse's cursor
     * 
     */
    class Cursor{
        public:
            /**
             * @brief Construct a new empty Cursor object
             * 
             */
            Cursor():cursor(nullptr),own(false){};
            /**
             * @brief Construct a new System Cursor object
             * 
             */
            Cursor(SystemCursor);
            /**
             * @brief Construct a new Cursor object from pixbuf
             * 
             * @param pixbuf The pixbuf
             * @param hot_x The cursor's x point
             * @param hot_y The cursor's y point
             */
            Cursor(const PixBuf &pixbuf,int hot_x = 0,int hot_y = 0);
            ~Cursor();
            /**
             * @brief Set the cursor 
             * 
             */
            void set() const;

            /**
             * @brief Reset the cursor to default
             * 
             */
            static void reset();
            /**
             * @brief Get default cursor
             * 
             * @return Cursor 
             */
            static Cursor Default();
            /**
             * @brief Get current cursor
             * 
             * @return Cursor 
             */
            static Cursor Current();
        private:
            Cursor(SDL_Cursor *c,bool w):
                cursor(c),
                own(w){};

            
            SDL_Cursor *cursor;
            bool own;
    };
}


#endif // _BTK_CURSOR_HPP_
