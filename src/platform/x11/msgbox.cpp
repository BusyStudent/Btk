#include "../../build.hpp"
#include <Btk/msgbox/impl.hpp>
#include "internal.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/platform/x11.hpp>
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
    bool MessageBox(u8string_view _title,u8string_view _msg,int flag){
        //Copy into buffer;
        u8string title(_title);
        u8string msg(_msg);
        
        if(has_zenity){
            u8string_view cmd;
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