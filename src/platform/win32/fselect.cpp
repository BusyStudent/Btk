#include "../../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/async/async.hpp>
#include <Btk/impl/atomic.hpp>
#include <algorithm>
#include <string>

#include <ShlObj.h>
#include <ShObjIdl.h>

#include "internal.hpp"

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

    void FSelectBoxImpl::Run(){
        using utf8::unchecked::utf16to8;
        using std::back_inserter;
        using Win32::ComInstance;
        using Win32::ComMemPtr;


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