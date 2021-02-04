#include "../../build.hpp"

#include <Btk/msgbox/impl.hpp>
#include "internal.hpp"

#undef MessageBox

namespace Btk{
    void MessageBoxImpl::Run(){
        Impl::RefDeleter deleter(this);
        UINT type = 0;

        if(flag == MessageBox::Info){
            type |= MB_ICONINFORMATION;
        }
        else if(flag == MessageBox::Warn){
            type |= MB_ICONWARNING;
        }
        else if(flag == MessageBox::Error){
            type |= MB_ICONERROR;
        }

        MessageBoxA(GetForegroundWindow(),message.c_str(),title.c_str(),type);
    }
    void MessageBoxImpl::unref(){
        --refcount;
        if(refcount <= 0){
            delete this;
        }
    }
}