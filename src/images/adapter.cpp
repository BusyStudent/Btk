#include "../build.hpp"
#include <Btk/impl/core.hpp>
#include "adapter.hpp"
namespace Btk{
    void InitImageAdapter(){
        RegisterSDLImage();
        RegisterSTBII();
        RegisterWEBP();
        RegisterPNG();
    }
}