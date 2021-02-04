#include "../../build.hpp"

#ifdef _WIN32

#include <Btk/thirdparty/utf8.h>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/msgbox/impl.hpp>
#include <Btk/async/async.hpp>
#include <Btk/impl/atomic.hpp>
#include <algorithm>
#include <string>

#include <ShlObj.h>
#include <ShObjIdl.h>

#include "internal.hpp"
//Win32 native FSelectBox impl
namespace Btk{
    void FSelectBoxImpl::Run(){
        using utf8::unchecked::utf16to8;
        using std::back_inserter;
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
                        value.clear();
                        std::wstring_view view(fname);
                        utf16to8(view.begin(),view.end(),back_inserter(value));

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