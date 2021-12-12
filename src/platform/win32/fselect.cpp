#include "../../build.hpp"

#if 0

#ifdef _WIN32

#include <Btk/msgbox/fselect.hpp>
#include <Btk/msgbox/impl.hpp>
#include <Btk/impl/atomic.hpp>
#include <Btk/async.hpp>
#include <algorithm>
#include <string>

#include <ShlObj.h>
#include <ShObjIdl.h>

#include "internal.hpp"
//Win32 native FSelectBox impl
namespace Btk{
    void FSelectBoxImpl::Run(){
        using Win32::ComInstance;
        using Win32::ComMemPtr;

        Impl::RefDeleter deleter(this);

        ComInstance<IFileOpenDialog> instance;
        HRESULT hr;
        hr = CoCreateInstance(CLSID_FileOpenDialog,
                        nullptr,
                        CLSCTX_ALL,
                        IID_IFileOpenDialog,
                        reinterpret_cast<void**>(&instance));

        if(SUCCEEDED(hr)){
            hr = instance->Show(GetForegroundWindow());
            if(SUCCEEDED(hr)){
                ComInstance<IShellItem> item;
                //We get the item
                hr = instance->GetResult(&item);
                if(SUCCEEDED(hr)){
                    ComMemPtr<wchar_t> fname;
                    hr = item->GetDisplayName(SIGDN_FILESYSPATH,&fname);
                    
                    if(SUCCEEDED(hr)){
                        //Store the name
                        value = u16string_view(fname).to_utf8();

                        BTK_LOGINFO("GetPath:%s",value.c_str());

                        signal(value);
                        
                        return;
                    }
                }
            }
        }
        else{
            //What shoud i do?
            BTK_LOGINFO("Failed to create ComInstance");
        }
    }
    void FSelectBoxImpl::unref(){
        --refcount;
        if(refcount <= 0){
            delete this;
        }
    }
}
#endif
#endif