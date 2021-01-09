#if !defined(_BTK_CURSOR_HPP_)
#define _BTK_CURSOR_HPP_
struct SDL_Cursor;
namespace Btk{
    class Cursor{
        private:
            SDL_Cursor *cursor;
            bool own;
    };
}


#endif // _BTK_CURSOR_HPP_
