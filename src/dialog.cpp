#include "./build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/thread.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/dialog.hpp>
#include <Btk/button.hpp>
#include <Btk/label.hpp>

#if BTK_X11
    #include <Btk/platform/popen.hpp>
    #include <sys/wait.h>
    #include "./platform/x11/internal.hpp"
#endif


namespace Btk{
    Dialog::Dialog() = default;
    Dialog::~Dialog() = default;
    auto Dialog::run() -> Status{
        if(_opened){
            //Double call
            return _status;
        }


        _opened = true;
        
        _status = do_run();
        //Emit signal
        _signal_finished();
        if(_status == Accepted){
            _signal_accepted();
        }
        else{
            _signal_rejected();
        }

        _opened = false;
        return _status;
    }
    auto Dialog::wait() -> Status{
        if(not _opened){
            return _status;
        }
        
        _status = do_wait();
        //Emit signal
        _signal_finished();
        if(_status == Accepted){
            _signal_accepted();
        }
        else{
            _signal_rejected();
        }

        _opened = false;
        return _status;
    }
    void Dialog::show(){
        _opened = true;
        return do_show();
    }
    //MessageBox
    // MessageBox::~MessageBox(){
    //     // _do_destroy();
    // }
    struct MessageBox::_Native{
        #if BTK_WIN32
        
        #elif BTK_X11
        process_t proc;
        #endif
    };
    //Local impl
    struct MessageBox::_Local:public Window{
        ImageView *icon;
        Button *btn_yes;
        Label  *lab_msg;
    };
    MessageBox::MessageBox(u8string_view title,u8string_view message,Flag flag):
        Dialog(),
        _title(title),
        _message(message),
        _flag(flag){

    }
    MessageBox::~MessageBox(){
        delete native_impl;
        delete local_impl;
    }
    auto MessageBox::do_run() -> Status{
        #if BTK_X11
        do_show();
        return do_wait();
        #endif
    }
    void MessageBox::do_show(){
        #if BTK_X11
        if(native_impl == nullptr){
            native_impl = new _Native;
        }
        if(X11::has_kdialog){
            //Has kdialog
            u8string_view cmd;
            switch(_flag){
                case MessageBox::Info:
                    cmd = "--msgbox";
                    break;
                case MessageBox::Warn:
                    cmd = "--warningyesno";
                    break;
                case MessageBox::Error:
                    cmd = "--error";
                    break;

            }
            if(parent() != nullptr){
                native_impl->proc = spawn("kdialog","--title",_title,cmd,_message);
            }
            else{
                //has parent
                native_impl->proc = spawn(
                    "kdialog",
                    "--title",_title,
                    cmd,_message,
                    "--embed",std::to_string(parent()->x_window)
                );
            }
        }
        else if(X11::has_zenity){
            //Has zenity
            u8string_view cmd;
            switch(_flag){
                case MessageBox::Info:
                    cmd = "--info";
                    break;
                case MessageBox::Warn:
                    cmd = "--warning";
                    break;
                case MessageBox::Error:
                    cmd = "--error";
                    break;
            }
            native_impl->proc = spawn("zenity","--text",_message,cmd,"--title",_title);
        }
        else{
            throwRuntimeError("No native useable");
        }
        #endif
    }
    auto MessageBox::do_wait() -> Status{
        #if BTK_X11
        ::waitpid(native_impl->proc,nullptr,0);
        return Accepted;
        #endif
    }
    //FileDialog
    struct FileDialog::_Local:public Window{
        
    };
    struct FileDialog::_Native{
        #if BTK_X11
        FILE *pstream;
        #endif
    };
    FileDialog::FileDialog(u8string_view title,Flags flags):
        Dialog(),
        _title(title),
        _flags(flags){

    }
    FileDialog::~FileDialog(){
        delete local_impl;
        delete native_impl;
    }
    auto FileDialog::do_run() -> Status{
        #if BTK_X11
        do_show();
        return do_wait();
        #endif
    }
    auto FileDialog::do_wait() -> Status{
        #if BTK_X11
        u8string buf;
        int ch;
        while(not std::feof(native_impl->pstream)){
            ch = std::fgetc(native_impl->pstream);
            if(ch == EOF){
                break;
            }
            else if(ch == '\n'){
                break;
            }
            buf += char(ch);
        }
        fclose(native_impl->pstream);
        native_impl->pstream = nullptr;
        //Check
        if(buf.empty()){
            return Rejected;
        }
        //TODO Will it has error when each path has sapce in there?
        _result = buf.split(" ");
        return Accepted;
        #endif
    }
    void FileDialog::do_show(){
        #if BTK_X11
        if(native_impl == nullptr){
            native_impl = new _Native;
        }
        if(X11::has_kdialog){
            //Parse args
            u8string_view kind;
            if((_flags & Open) == Open){
                kind = "--getopenfilename";
            }
            else if((_flags & Save) == Save){
                kind = "--getsavefilename";
            }
            else if((_flags & Directory) == Directory){
                kind = "--getexistingdirectory";
            }

            //If multi
            if((_flags & Multiple) == Multiple){
                native_impl->pstream = popen("kdialog",kind,"--title",_title,"--multiple");
            }
            else{
                native_impl->pstream = popen("kdialog",kind,"--title",_title);
            }
        }
        else if(X11::has_zenity){
            //Parse args
            StringRefList reflist = {"zenity","--file-selection"};
            if((_flags & Multiple) == Multiple){
                reflist.push_back("--multiple");
            }
            if((_flags & Save) == Save){
                reflist.push_back("--save");
            }
            reflist.push_back("--title");
            reflist.push_back(_title);
            native_impl->pstream = vpopen(reflist);
        }
        if(native_impl->pstream == nullptr){
            //Handle err?
        }
        #endif
    }
}