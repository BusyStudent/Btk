#include "../../build.hpp"
#include "internal.hpp"

#include <Btk/impl/application.hpp>
#include <Btk/application.hpp>

#include <shellapi.h>
#include <WinUser.h>

namespace Btk{
    ApplicationImpl::ApplicationImpl(){

    }
    ApplicationImpl::~ApplicationImpl(){
        DestroyWindow(app_handle);
    }
    //FIXME:It will block the message loop
    bool Application::notify(u8string_view title,u8string_view msg){
        Win32::ComInstance<IUserNotification> notification;
        HRESULT hr = CoCreateInstance(
            CLSID_UserNotification,
            nullptr,
            CLSCTX_ALL,
            IID_IUserNotification,
            reinterpret_cast<void **>(&notification)
        );
        if(FAILED(hr)){
            return false;
        }
        notification->SetBalloonInfo(
            reinterpret_cast<LPCWSTR>(title.to_utf16().c_str()),
            reinterpret_cast<LPCWSTR>(msg.to_utf16().c_str()),
            NIIF_INFO
        );
        return SUCCEEDED(notification->Show(nullptr,INFINITE));
    }
}