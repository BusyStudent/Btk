#if !defined(_BTK_MSGBOX_IMPL)
#define _BTK_MSGBOX_IMPL
#include "../impl/atomic.hpp"
#include "fselect.hpp"
#include "msgbox.hpp"
#include <string>
#include <memory>
namespace Btk{
    namespace Impl{
        /**
         * @brief A helper class to delete MessageBox impl
         * 
         * @tparam T 
         */
        template<class T>
        struct RefDeleter{
            RefDeleter(T *ptr){
                this->ptr = ptr;
            }
            RefDeleter(const RefDeleter &) = delete;
            ~RefDeleter(){
                ptr->unref();
            }
            T *ptr;
        };
    };
    struct FSelectBoxImpl{
        using SignalAsync = FSelectBox::SignalAsync;
        
        //< The return value of the box
        u8string value;
        u8string title;
        //< The title
        Atomic refcount = 1;
        
        bool multiple = false;
        bool save = false;

        SignalAsync signal;
        //< The async signal
        void Run();
        void unref();
    };
    struct MessageBoxImpl{
        using SignalAsync = MessageBox::SignalAsync;
                
        u8string title;
        u8string message;
        SignalAsync signal;
        //messagebox flag
        int    flag;
        Atomic refcount = 1;
        void Run();
        void unref();
    };
}



#endif // _BTK_MSGBOX_IMPL
