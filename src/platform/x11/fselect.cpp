#ifdef __gnu_linux__

#include "../../build.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/impl/atomic.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/window.hpp>
#include <Btk/string.hpp>
#include <Btk/button.hpp>
#include <Btk/async.hpp>
#include <Btk/Btk.hpp>
#include <string>
// namespace Btk{
//     void FSelectBoxImpl::Run(){
//         Impl::RefDeleter deleter(this);
//         //We use zenity or kdialog to impl it
//         PStream pfd;
//         //gen command line
//         u8string cmd("zenity --file-selection");
//         if(not title.empty()){
//             cformat(cmd," --title '%s'",title.c_str());
//         }
//         if(multiple){
//             cmd += " --multiple";
//         }
//         if(save){
//             cmd += " --save";
//         }
//         BTK_LOGINFO("Exec %s",cmd.c_str());
//         if(pfd.try_open(
//             cmd,
//             "r")){
//             pfd >> value;
//             BTK_LOGINFO("Zenity return value %s",value.c_str());
//             if(not signal.empty()){
//                 signal(value);
//             }
//         }
//         else{
//             BTK_LOGINFO("Failed to exec %s %d:%s",cmd.c_str(),errno,strerror(errno));
//         }
//     }
//     void FSelectBoxImpl::unref(){
//         --refcount;
//         if(refcount <= 0){
//             delete this;
//         }
//     }
// }
#endif