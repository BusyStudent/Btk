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
}