#if !defined(_BTK_DEFS_HPP_)
#define _BTK_DEFS_HPP_

#ifdef _WIN32
    #ifdef (_MSV_VER_)
    #define BTKEXPORT __declspec(dllexport)
    #define BTKIMPORT __declspec(dllimport)
    #else
    #define BTKEXPORT __attribute__(dllexport)
    #define BTKIMPORT __attribute__(dllimport)
    #endif
#else
    //ignore this
    #define BTKEXPORT     
    #define BTKIMPORT 
#endif

#ifdef _BTK_SOURCE
    //source file
    #define BTKAPI BTKEXPORT
#else
    #define BTKAPI BTKIMPORT
#endif

#endif // _BTK_DEFS_HPP_
