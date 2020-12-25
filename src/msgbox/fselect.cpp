#include "../build.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/async/async.hpp>
#include <Btk/impl/atomic.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/window.hpp>
#include <Btk/button.hpp>
#include <string>
namespace Btk{
    struct FSelectBoxImpl{
        using SignalAsync = FSelectBox::SignalAsync;
        //< The return value of the box
        std::string value;
        std::string title;
        //< The title
        Atomic refcount = 1;
        
        bool multiple = false;
        bool save = false;

        SignalAsync signal;
        //< The async signal
        void Run();
        void unref(){
            --refcount;
            if(refcount <= 0){
                delete this;
            }
        }
    };
    //The deleter
    struct FSelectDeleter{
        void operator()(FSelectBoxImpl *ptr){
            ptr->unref();
        };
    };
    void FSelectBoxImpl::Run(){
        std::unique_ptr<FSelectBoxImpl,FSelectDeleter> ptr(this);
        #ifdef __gnu_linux__
        //We use zenity or kdialog to impl it
        PStream pfd;
        //gen command line
        std::string cmd("zenity --file-selection");
        if(not title.empty()){
            cmd += cformat(" --title '%s'",title.c_str());
        }
        if(multiple){
            cmd += " --multiple";
        }
        if(save){
            cmd += " --save";
        }
        BTK_LOGINFO("Exec %s",cmd.c_str());
        if(pfd.try_open(
            cmd,
            "r")){
            pfd >> value;
            BTK_LOGINFO("Zenity return value %s",value.c_str());
            if(not signal.empty()){
                signal(value);
            }
        }
        else{
            BTK_LOGINFO("Failed to exec %s %d:%s",cmd.c_str(),errno,strerror(errno));
        }
        #else
        throwRuntimeError("Doesn't Impl yet");
        #endif
    }
}
namespace Btk{
    FSelectBox::~FSelectBox(){
        if(pimpl != nullptr){
            pimpl->unref();
        }
    }
    FSelectBox::FSelectBox(std::string_view title){
        pimpl = new FSelectBoxImpl;
        pimpl->title = title;
    }
    void FSelectBox::show(){
        ++(pimpl->refcount);
        Async(NoSignal(),&FSelectBoxImpl::Run,pimpl).lauch();
    }
    void FSelectBox::set_multi(bool val){
        pimpl->multiple = val;
        if(val){
            pimpl->save = false;
        }
    }
    void FSelectBox::set_save(bool val){
        pimpl->save = val;
        if(val){
            //The multipie flag was conflict with it
            pimpl->multiple = false;
        }
    }
    FSelectBox::SignalAsync &FSelectBox::sig_async(){
        return pimpl->signal;
    }
}