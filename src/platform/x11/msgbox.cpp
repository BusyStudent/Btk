#include "../../build.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/msgbox/impl.hpp>
namespace Btk{
    void MessageBoxImpl::Run(){
        Impl::RefDeleter deleter(this);
        std::string cmd = "zenity ";

        if(not title.empty()){
            cformat(cmd," --title '%s'",title.c_str());
        }
        if(not message.empty()){
            cformat(cmd," --text '%s'",message.c_str());
        }
        switch(flag){
            case MessageBox::Info:
                cmd += " --info";
                break;
            case MessageBox::Warn:
                cmd += " --warning";
                break;
            case MessageBox::Error:
                cmd += " --error";
                break;
        }
        BTK_LOGINFO("Exec %s",cmd.c_str());
        
        PStream pfd(cmd.c_str(),"r");
        pfd.close();
        signal();
    }
    void MessageBoxImpl::unref(){
        --refcount;
        if(refcount <= 0){
            delete this;
        }
    }
}