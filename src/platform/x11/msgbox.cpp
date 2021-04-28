#include "../../build.hpp"
#include "internal.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/platform/x11.hpp>
#include <Btk/msgbox/impl.hpp>
namespace Btk{
    void MessageBoxImpl::Run(){
        Impl::RefDeleter deleter(this);
        // std::string cmd = "zenity ";

        // if(not title.empty()){
        //     cformat(cmd," --title '%s'",title.c_str());
        // }
        // if(not message.empty()){
        //     cformat(cmd," --text '%s'",message.c_str());
        // }
        // switch(flag){
        //     case MessageBox::Info:
        //         cmd += " --info";
        //         break;
        //     case MessageBox::Warn:
        //         cmd += " --warning";
        //         break;
        //     case MessageBox::Error:
        //         cmd += " --error";
        //         break;
        // }
        // BTK_LOGINFO("Exec %s",cmd.c_str());
        
        // PStream pfd(cmd.c_str(),"r");
        // pfd.close();
        // signal();
        X11::MessageBox(title,message,flag);
        signal();
    }
    void MessageBoxImpl::unref(){
        --refcount;
        if(refcount <= 0){
            delete this;
        }
    }
}
namespace Btk::X11{
    //Native MessageBox
    bool MessageBox(std::string_view _title,std::string_view _msg,int flag){
        //Copy into buffer;
        std::string title(_title);
        std::string msg(_msg);
        
        if(has_zenity){
            std::string_view cmd;
            switch(flag){
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
            Execute("zenity","--text",msg,cmd,"--title",title);
        }
        else if(has_kdialog){
            //TODO
            return false;
        }
        else{
            return false;
        }
        return true;
    }

}