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
            enum Status:Uint8{
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
            void set_parent(Window &win){
                _parent = &win;
            }
        private:
            Window *_parent = {};
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

            Window *parent() const noexcept{
                return _parent;
            }

            virtual Status do_run() = 0;
            //Default void / wait
            virtual void   do_show() = 0;
            virtual Status do_wait() = 0;
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

            void set_flag(Flag f){
                _flag = f;
            }
            void set_title(u8string_view title){
                _title = title;
            }
            void set_message(u8string_view message){
                _message = message;
            }
        private:
            Status do_run() override;
            Status do_wait() override;
            void   do_show() override;

            //Local fns
            // Status loc_do_run();
            // Status loc_do_wait();
            // void   loc_do_show();
            
            struct _Native;
            struct _Local;

            u8string _title;
            u8string _message;
            Flag     _flag;

            _Native *native_impl = {};
            _Local *local_impl = {};
    };
    class BTKAPI FileDialog:public Dialog{
        public:
            enum Flags:Uint32{
                Open = 0,
                Save = 1 << 0,
                Directory = 1 << 1,
                Multiple = 1 << 2,
            };
            FileDialog(u8string_view title = {},Flags flag = Open);
            ~FileDialog();

            const StringList &result() const noexcept{
                return _result;
            }
            void set_multiple(bool v = true) noexcept;
            void set_title(u8string_view title){
                _title = title;
            }
            void set_flags(Flags f) noexcept{
                _flags = f;
            }
            Flags flags() const noexcept{
                return _flags;
            }
        private:
            Status do_run() override;
            Status do_wait() override;
            void   do_show() override;

            struct _Native;
            struct _Local;

            u8string   _title;
            StringList _result;
            Flags      _flags;

            _Native *native_impl = {};
            _Local *local_impl = {};
    };
    /**
     * @brief Dialog for input text
     * 
     */
    class BTKAPI TextDialog:public Dialog{

    };
    /**
     * @brief Dialog for picking a color
     * 
     */
    class BTKAPI ColorDialog:public Dialog{

    };

    BTK_FLAGS_OPERATOR(FileDialog::Flags,Uint32);
    //Method
    inline void FileDialog::set_multiple(bool v) noexcept{
        if(v){
            _flags |= Multiple;
        }
        else{
            _flags ^= Multiple;
        }
    }
}


#endif // _BTK_DIALOG_HPP_
