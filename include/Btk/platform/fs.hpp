#if !defined(_BTK_PLATFORM_FS_HPP_)
#define _BTK_PLATFORM_FS_HPP_

#include <type_traits>
#include <utility>
#include <string>

#include <cstring>
#include <cstdlib>
#include <cerrno>
#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #define BTK_GETCWD ::_getcwd
    #define BTK_ACCESS ::_access
    #define BTK_CHDIR  ::_chdir
    #define BTK_F_OK 00
    //We need WinAPI for list dir
    #include <fileapi.h>
#else
    #include <unistd.h>
    #include <dirent.h>
    #define BTK_GETCWD ::getcwd
    #define BTK_ACCESS ::access
    #define BTK_CHDIR  ::chdir
    #define BTK_F_OK F_OK
#endif
#include "../exception.hpp"
#include "../string.hpp"
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
    inline bool exists(u8string_view fname){
        return BTK_ACCESS(fname.data(),BTK_F_OK) == 0;
    }
    /**
     * @brief Change the current work dir
     * 
     * @param path The work dir
     * @return true 
     * @return false 
     */
    inline bool chdir(u8string_view path){
        return BTK_CHDIR(path.data()) == 0;
    }
    /**
     * @brief For Value in the path
     * 
     * @tparam Callable 
     * @tparam Args 
     * @param callable 
     * @param args 
     * @return true 
     * @return false 
     */
    template<class Callable,class ...Args>
    bool ForPath(Callable &&callable,Args &&...args){
        //Does the callable has return value
        constexpr bool has_retvalue = not 
        std::is_same_v<
            void,
            std::invoke_result_t<Callable,u8string_view,Args...>
        >;

        char *path = std::getenv("PATH");
        if(path == nullptr){
            return false;
        }
        u8string_view dir;
        #ifdef _WIN32
        char delim = ';';
        #else
        char delim = ':';
        #endif
        char *end,*cur = path;
        do{
            end = strchr(cur,delim);
            if(end != nullptr){
                dir = u8string_view(cur,end - cur);
            }
            else{
                dir = u8string_view(cur);
            }
            if constexpr(has_retvalue){
                if(not callable(dir,std::forward<Args>(args)...)){
                    break;
                }
            }
            else{
                callable(dir,std::forward<Args>(args)...);
            }
            
            cur = end + 1;
        }
        while(end != nullptr);
        return true;
    }
    /**
     * @brief Check a file in path
     * 
     * @return true 
     * @return false 
     */
    inline bool FileInPath(u8string_view fname){
        u8string buffer;
        bool in_path = false;
        ForPath([&](u8string_view dir){
            buffer = dir;
            #ifdef __WIN32
            //Win32 add \\ or /
            char back = buffer.back();
            if(back != '/' or back != '\\'){
                buffer.push_back('/');
            }
            #else
            //Add '/
            if(buffer.back() != '/'){
                buffer.push_back('/');
            }
            #endif
            buffer += dir;
            in_path = exists(buffer);
            if(in_path == true){
                //End the loop
                return false;
            }
            return true;
        });
        return in_path;
    }
    #if BTK_LINUX
    inline StringList ListDir(u8string_view dirname = {}){
        if(dirname.empty()){
            dirname = "./";
        }
        ::DIR *dir = ::opendir(dirname.data());
        if(dir == nullptr){
            throwRuntimeError(strerror(errno));
        }
        //Open dir
        StringList list;
        //For dir
        ::dirent *ent;
        ent = ::readdir(dir);
        for(;ent != nullptr ; ent = ::readdir(dir)){
            //Ignore . and ..
            if(ent->d_name[0] == '.'){
                if(ent->d_name[1] == '\0'){
                    continue;
                }
                else if(ent->d_name[1] == '.' and ent->d_name[2] == '\0'){
                    continue;
                }
            }
            list.push_back(ent->d_name);
        }
        ::closedir(dir);
        return list;
    }
    #elif BTK_WIN32
    inline StringList ListDir(u8string_view dirname = {}){
        u16string us = dirname.to_utf16();
        us.append(u"*.*");
        
        WIN32_FIND_DATAW data;
        HANDLE hd = ::FindFirstFileW(us.w_str(),&data);

        StringList strlist;
        strlist.push_back(u8string::from_utf16(data.cFileName));

        while(::FindNextFileW(hd,&data)){
            strlist.push_back(u8string::from_utf16(data.cFileName));
        }
        ::CloseHandle(hd);
        return strlist;
    }
    #endif
    
    struct FileMapping{
        enum Flags:Uint32{
            Read = 1 << 0,
            Write = 1 << 1,
            ReadAndWrite = Read | Write
        };
        void *address = nullptr;
        size_t size = 0;
        
        #if BTK_WIN32
        HANDLE view = INVALID_HANDLE_VALUE;
        #endif
    };
    auto MapFile(u8string_view f,FileMapping::Flags flags) -> FileMapping;
    void UnmapFile(const FileMapping &);
}


#endif // _BTK_PLATFORM_FS_HPP_
