#include "../../build.hpp"

#include <Btk/msgbox/impl.hpp>
#include <Btk/utils/mem.hpp>
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
        MessageBoxW(
            GetForegroundWindow(),
            reinterpret_cast<const wchar_t*>(Utf8To16(message).c_str()),
            reinterpret_cast<const wchar_t*>(Utf8To16(title).c_str()),
            type);
    }
    void MessageBoxImpl::unref(){
        --refcount;
        if(refcount <= 0){
            delete this;
        }
    }
}