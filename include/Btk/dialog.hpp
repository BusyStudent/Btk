#if !defined(_BTK_DIALOG_HPP_)
#define _BTK_DIALOG_HPP_
#include "defs.hpp"
#include "string.hpp"
#include "object.hpp"
#include "window.hpp"
#include "utils/sync.hpp"
#include "utils/template.hpp"
/**
 * @brief All Dialogs(FileSelectDialog,MessageBox,etc...)
 * 
 */

namespace Btk{
    class BTKAPI Dialog:public HasSlots{
        public:
            enum Option:Uint32{
                Native = 0,
                DonotUseNative = 1 << 0
            };
            enum Status:bool{
                Rejected = 0,
                Accepted = 1,
            };
            Dialog();
            ~Dialog();
            /**
             * @brief Show the dialog(Nonblock)
             * 
             */
            void  show();
            /**
             * @brief Wait for the async thread
             * 
             * @return Status 
             */
            Status wait();
            /**
             * @brief Block the thread 
             * 
             * @return Status 
             */
            Status run();

            Status status() const noexcept{
                return _status;
            }
            Option option() const noexcept{
                return _option;
            }
            Signal<void()> &signal_finished() noexcept{
                return _signal_finished;
            }
            Signal<void()> &signal_accepted() noexcept{
                return _signal_accepted;
            }
            Signal<void()> &signal_rejected() noexcept{
                return _signal_rejected;
            }

        private:
            void default_show();

            Constructable<SyncEvent> _event;
            WindowImpl *_parent = {};
            Option _option = {};
            Status _status = {};
            bool _opened = false;
            bool _asynced = false;

            //Signals
            Signal<void()> _signal_finished;
            Signal<void()> _signal_accepted;
            Signal<void()> _signal_rejected;

        protected:
            //Function table
            VirtualFunction<Status()> do_run;
            VirtualFunction<void  ()> do_show;

            WindowImpl *parent() const noexcept{
                return _parent;
            }
    };
    BTK_FLAGS_OPERATOR(Dialog::Option,Uint32);
    //Boxs ...
    class BTKAPI MessageBox:public Dialog{
        public:
            enum Flag:int{
                Info,
                Warn,
                Error
            };

            MessageBox(u8string_view title = {},u8string_view msg = {},Flag f = Info);
            ~MessageBox();
        private:
            /**
             * @brief Platform MessageBox
             * 
             * @return Status 
             */
            BTKHIDDEN
            Status _do_run();
            BTKHIDDEN
            void   _do_destroy();


            u8string _title;
            u8string _message;
            Flag     _flag;
            //Native impl data
            #ifdef _WIN32

            #else

            #endif
            //Our impl
            Constructable<Window> _window;
        };
    class BTKAPI FileDialog:public Dialog{

    };
}


#endif // _BTK_DIALOG_HPP_
