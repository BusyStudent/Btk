#include "./build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/thread.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/dialog.hpp>
#include <Btk/button.hpp>
#include <Btk/label.hpp>
#include <Btk/Btk.hpp>

#if BTK_X11
    #include <Btk/platform/popen.hpp>
    #include <sys/wait.h>
    #include "./platform/x11/internal.hpp"
#elif BTK_WIN32
    #include <Btk/platform/winutils.hpp>
    #include <Btk/platform/win32.hpp>
    #include <ShlObj.h>
    #include <ShObjIdl.h>
#endif

#if BTK_X11
#include <csignal>
namespace{
    void main_thrd_wait_and_dispatch(pid_t pid){
        //Dispatch
        bool *has_got = new bool(false);
        auto sig_handle = std::signal(SIGCHLD,[](int){
            //Got a sigchld
            //Interrupt the loop
            Btk::InterruptLoop();
        });

        Btk_defer [pid,has_got,sig_handle](){
            std::signal(SIGCHLD,sig_handle);
            if(not *has_got){
                //Wait until exit
                waitpid(pid,nullptr,0);
            }
            delete has_got;
        };

        while(Btk::WaitEvent()){
            if(::waitpid(pid,nullptr,WNOHANG) == pid){
                //Got this
                *has_got = true;
                return;
            }
        }
    }
}
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
        SyncEvent event;
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
        #elif BTK_WIN32
        UINT type = MB_OK;
        switch(_flag){
            case MessageBox::Info:{
                type |= MB_ICONINFORMATION;
                break;
            }
            case MessageBox::Warn:{
                type |= MB_ICONWARNING;
                break;
            }
            case MessageBox::Error:{
                type |= MB_ICONERROR;
                break;
            }
        }
        HWND owner = nullptr;
        if(parent() != nullptr and not IsMainThread()){
            owner = WinUtils::GetHandleFrom(parent()->sdl_window());
        }
        ::MessageBoxW(
            owner,
            _message.to_utf16().w_str(),
            _title.to_utf16().w_str(),
            type
        );
        return Accepted;
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
        #elif BTK_WIN32
        if(native_impl == nullptr){
            native_impl = new _Native;
        }
        native_impl->event.clear();
        Thread thread([this](){
            do_run();
            native_impl->event.set();
        });
        thread.detach();
        #endif
    }
    auto MessageBox::do_wait() -> Status{
        #if BTK_X11
        if(parent() != nullptr and IsMainThread()){
            //Is main thrd and has parent
            parent()->set_modal(true);
            main_thrd_wait_and_dispatch(native_impl->proc);
            parent()->set_modal(false);
        }
        else{
            ::waitpid(native_impl->proc,nullptr,0);
        }
        return Accepted;
        #elif BTK_WIN32
        native_impl->event.wait();
        native_impl->event.clear();
        return Accepted;
        #endif
    }
    //FileDialog
    struct FileDialog::_Local:public Window{
        
    };
    struct FileDialog::_Native{
        #if BTK_X11
        FILE *pstream;
        #elif BTK_WIN32
        Win32::ComInstance<IFileDialog> dialog;
        Status last_status = {};
        Flags last_flags = {};
        SyncEvent event;
        //Init dialog
        void init(Flags f){
            if(f == last_flags and not dialog.empty()){
                return;
            }
            //DoParse
            Win32::ComInstance<IFileDialog> new_dialog;
            const GUID *clsid;
            const GUID *iid;
            if((f & Open) == Open){
                clsid = &CLSID_FileOpenDialog; 
                iid   = &IID_IFileOpenDialog;
            }
            else if((f & Save) == Save){
                clsid = &CLSID_FileSaveDialog;
                iid   = &IID_IFileSaveDialog;
            }

            //DoCreate
            HRESULT hr;
            hr = CoCreateInstance(
                *clsid,
                nullptr,
                CLSCTX_ALL,
                *iid,
                reinterpret_cast<void**>(&new_dialog)
            );
            if(FAILED(hr)){
                throwWin32Error(hr);
            }
            //DoConfigure
            FILEOPENDIALOGOPTIONS opt;
            new_dialog->GetOptions(&opt);
            if((f & Directory) == Directory){
                opt |= FOS_PICKFOLDERS;
            }
            if((f & Multiple) == Multiple){
                opt |= FOS_ALLOWMULTISELECT;
            }
            new_dialog->SetOptions(opt);

            //Set new val 
            dialog = new_dialog;
            last_flags = f;

        }
        void collect_result(StringList &arr){
            IFileOpenDialog *opendialog;
            if(SUCCEEDED(dialog->QueryInterface(&opendialog))){
                //Multi
                Win32::ComInstance<IFileOpenDialog> guard(opendialog);
                Win32::ComInstance<IShellItemArray> items;
                wchar_t *name;
                DWORD counts;
                opendialog->GetResults(&items);
                //For each and add into it
                items->GetCount(&counts);
                for(DWORD n = 0;n < counts;n++){
                    Win32::ComInstance<IShellItem> item;
                    items->GetItemAt(n,&item);
                    item->GetDisplayName(SIGDN_FILESYSPATH,&name);
                    //Convert
                    arr.emplace_back(u16string_view(name).to_utf8());
                    CoTaskMemFree(name);
                }
            }
            else{
                //No Multi
                Win32::ComInstance<IShellItem> item;
                wchar_t *name;
                dialog->GetResult(&item);
                item->GetDisplayName(SIGDN_FILESYSPATH,&name);
                //Convert
                arr.emplace_back(u16string_view(name).to_utf8());
                CoTaskMemFree(name);
            }
        }
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
        #elif BTK_WIN32
        if(native_impl == nullptr){
            native_impl = new _Native;
        }
        native_impl->init(_flags);
        if(not _title.empty()){
            native_impl->dialog->SetTitle(_title.to_utf16().w_str());
        }
        HWND owner = nullptr;
        if(parent() != nullptr and not IsMainThread()){
            owner = WinUtils::GetHandleFrom(parent()->sdl_window());
        }
        if(FAILED(native_impl->dialog->Show(owner))){
            return Rejected;
        }
        //Parse result
        _result.clear();
        native_impl->collect_result(_result);
        if(_result.empty()){
            return Rejected;
        }
        return Accepted;
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
        #elif BTK_WIN32
        native_impl->event.wait();
        native_impl->event.clear();
        return native_impl->last_status;
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
        #elif BTK_WIN32
        if(native_impl == nullptr){
            native_impl = new _Native;
        }
        native_impl->event.clear();
        Thread thread([this](){
            native_impl->last_status = do_run();
            native_impl->event.set();
        });
        thread.detach();
        #endif
    }
}