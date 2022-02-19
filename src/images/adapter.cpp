#include "../build.hpp"
#include <Btk/detail/core.hpp>
#include "adapter.hpp"
namespace Btk{
    void InitImageAdapter(){
        RegisterSDLImage();
        RegisterSTBII();
        RegisterWEBP();
        RegisterWIC();
        RegisterPNG();
        RegisterGIF();
    }
}