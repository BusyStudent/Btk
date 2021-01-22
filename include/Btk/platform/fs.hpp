#if !defined(_BTK_PLATFORM_FS_HPP_)
#define _BTK_PLATFORM_FS_HPP_

#include <string_view>
#include <string>

#include <cstring>
#include <cerrno>
#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #define BTK_GETCWD ::_getcwd
    #define BTK_ACCESS ::_access
    #define BTK_CHDIR  ::_chdir
    #define BTK_F_OK 00
#else
    #include <unistd.h>
    #define BTK_GETCWD ::getcwd
    #define BTK_ACCESS ::access
    #define BTK_CHDIR  ::chdir
    #define BTK_F_OK F_OK
#endif
#include "../exception.hpp"
namespace Btk{
    /**
     * @brief Get current working dir
     * 
     * @return The current working dir
     */
    inline std::string getcwd(){
        char *buf = BTK_GETCWD(nullptr,0);
        if(buf == nullptr){
            throwRuntimeError(strerror(errno));
        }
        std::string s(buf);
        free(buf);
        return s;
    }
    inline std::string getcwd(size_t bufsize){
        std::string s;
        s.resize(bufsize);
        if(BTK_GETCWD(s.data(),bufsize) == nullptr){
            throwRuntimeError(strerror(errno));
        }
        return s;
    }
    /**
     * @brief Check the file exists
     * 
     * @param fname The filename
     * @return true 
     * @return false 
     */
    inline bool exists(std::string_view fname){
        return BTK_ACCESS(fname.data(),BTK_F_OK) == 0;
    }
    /**
     * @brief Change the current work dir
     * 
     * @param path The work dir
     * @return true 
     * @return false 
     */
    inline bool chdir(std::string_view path){
        return BTK_CHDIR(path.data()) == 0;
    }

}


#endif // _BTK_PLATFORM_FS_HPP_
