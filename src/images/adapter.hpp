#if !defined(_BTK_INTERNAL_IMAGE_ADAPTER_HPP_)
#define _BTK_INTERNAL_IMAGE_ADAPTER_HPP_

#include "../build.hpp"

#ifdef BTK_HAS_PNG
    namespace Btk{
        BTKHIDDEN void RegisterPNG();
    }
#else
    namespace Btk{
        inline void RegisterPNG(){}
    }
#endif

#ifdef BTK_HAS_SDLIMG
    namespace Btk{
        BTKHIDDEN void RegisterSDLImage();
    }
#else
    namespace Btk{
        inline void RegisterSDLImage(){}
    }

#endif

namespace Btk{
    struct BTKHIDDEN ImageLibrary{

    };
}

#endif // _BTK_INTERNAL_IMAGE_ADAPTER_HPP_
