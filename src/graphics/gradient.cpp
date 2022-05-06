#include "../build.hpp"

#include <Btk/graphics/base.hpp>
#include <Btk/exception.hpp>

#ifdef BTK_USE_BUILTIN_GRADIENTS
    #include "./gradients_mk.inl"
#endif

namespace Btk{
    Gradient::Gradient(const Gradient &) = default;
    Gradient::Gradient(Gradient &&) = default;
    Gradient::~Gradient() = default;

    Gradient::Gradient(Preset v){
        _type = Linear;
        #ifdef BTK_USE_BUILTIN_GRADIENTS
        mk_gradient(*this,v);
        #else
        throwRuntimeError("No build with this support");
        #endif
    }
}