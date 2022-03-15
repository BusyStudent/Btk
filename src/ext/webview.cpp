#include "../build.hpp"

#include <Btk/platform.hpp>

#ifdef _WIN32
//For IWebBroswer2
//TODO: Replace with WebView2
#include <ExDisp.h>
#include <ExDispid.h>

namespace{
    struct WebView{
        Btk::ComPtr<IWebBrowser2> browser;
    };
}


#elif defined(__gnu_linux__)


#endif