#if !defined(_BTK_CURSOR_HPP_)
#define _BTK_CURSOR_HPP_
struct SDL_Cursor;
namespace Btk{
    class PixBuf;
    class PixBufRef;
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
            Cursor() noexcept :cursor(nullptr){};
            /**
             * @brief Construct a new Cursor object
             * 
             * @param c 
             */
            Cursor(const Cursor &c) noexcept{
                cursor = c.cursor;
                _Ref(cursor);
            }
            Cursor(Cursor &&c) noexcept{
                cursor = c.cursor;
                c.cursor = nullptr;
            }
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
            Cursor(PixBufRef pixbuf,int hot_x = 0,int hot_y = 0);
            ~Cursor(){
                _Unref(cursor);
            }
            /**
             * @brief Set the cursor 
             * 
             */
            void set() const{
                _Set(cursor);
            }
            bool empty() const noexcept{
                return cursor == nullptr;
            }

            Cursor &operator =(const Cursor &c) noexcept{
                if(&c != this){
                    _Unref(cursor);
                    cursor = c.cursor;
                    _Ref(cursor);
                }
                return *this;
            }

            Cursor &operator =(Cursor &&c) noexcept{
                if(&c != this){
                    _Unref(cursor);
                    cursor = c.cursor;
                    c.cursor = nullptr;
                }
                return *this;
            }

            /**
             * @brief Reset the cursor to prev
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

            struct Impl;
        private:
            Cursor(Impl *c):cursor(c){}

            static void _Unref(void *cursor) noexcept;
            static void _Ref(void *cursor) noexcept;
            static void _Set(void *cursor);
            
            Impl *cursor;
    };
    /**
     * @brief Set the Cursor object
     * 
     * @return BTKAPI 
     */
    BTKAPI void SetCursor(SystemCursor);
    /**
     * @brief Reset cursor to prev
     * 
     * @return BTKAPI 
     */
    BTKAPI void ResetCursor() noexcept;

    inline void Cursor::reset(){
        ResetCursor();
    }
}


#endif // _BTK_CURSOR_HPP_
