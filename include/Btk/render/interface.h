#if !defined(_BTKRENDER_INTERFACE_H_)
#define _BTKRENDER_INTERFACE_H_
#include "../defs.hpp"

#ifdef __cplusplus
    extern "C"{
#endif

/**
 * @brief Btk Abstruct Renderer
 * 
 */
typedef void BtkRI;

BTKAPI int BtkRI_Line(BtkRI *render);
BTKAPI int BtkRI_Rect(BtkRI *render);




#ifdef __cplusplus
    }
#endif


#endif // _BTKRENDER_INTERFACE_H_
