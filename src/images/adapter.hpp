#if !defined(_BTK_INTERNAL_IMAGE_ADAPTER_HPP_)
#define _BTK_INTERNAL_IMAGE_ADAPTER_HPP_

#include "../build.hpp"

#ifdef BTK_HAS_PNG
    namespace Btk{
        BTKHIDDEN void RegisterPNG();
    }
#else
    namespace Btk{
        BTKINLINE void RegisterPNG(){}
    }
#endif

namespace Btk{
    struct BTKHIDDEN ImageLibrary{

    };
}

#endif // _BTK_INTERNAL_IMAGE_ADAPTER_HPP_
