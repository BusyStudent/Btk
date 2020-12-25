#if !defined(_BTK_POPEN_HPP_)
#define _BTK_POPEN_HPP_

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <string>
#include <string_view>

#ifdef _WIN32
    #include <process.h>
    #include <io.h>
    #define BTK_POPEN  ::_popen
    #define BTK_PCLOSE ::_pclose
#else
    #include <unistd.h>
    #define BTK_POPEN  ::popen
    #define BTK_PCLOSE ::pclose
#endif

#include "../exception.hpp"
namespace Btk{
    /**
     * @brief Pipe stream to or from a process
     * 
     */
    class PStream{
        public:
            PStream() = default;
            /**
             * @brief Construct a new PStream object
             * 
             * @param cmd The commond line
             * @param modes The pipe mode
             */
            PStream(const char *cmd,const char *modes);
            PStream(const PStream &) = delete;
            PStream(PStream &&);
            ~PStream();
            /**
             * @brief Close the stream
             * 
             */
            void close();
            /**
             * @brief Open a new stream
             * 
             * @param cmd The command line
             * @param modes The pipe mode
             */
            void open(const char *cmd,const char *modes);
            void open(std::string_view cmd,std::string_view modes){
                open(cmd.data(),modes.data());
            }
            /**
             * @brief Trt open a new stream
             * 
             * @param cmd The command line
             * @param modes The pipe mode
             * @return true Succeed
             * @return false Failed
             */
            bool try_open(const char *cmd,const char *modes);
            bool try_open(std::string_view cmd,std::string_view modes){
                return try_open(cmd.data(),modes.data());
            }
            /**
             * @brief Check the stream status
             * 
             * @return true Is bad
             * @return false Is good
             */
            bool bad();
            /**
             * @brief Check the stream at eof
             * 
             * @return true Is eof
             * @return false Is not at eof
             */
            bool eof();
            /**
             * @brief Assign from a lval stream
             * 
             * @return PStream& The stream ref
             */
            PStream &operator =(PStream &&);
            FILE *operator *() const noexcept{
                return fptr;
            }
            operator bool(){
                return not bad();
            }
            /**
             * @brief Read a line to string
             * 
             * @param stream The stream
             * @param str The string ref
             * @return PStream& The stream ref
             */
            friend PStream &operator >>(PStream &,std::string &);
            /**
             * @brief Write String to stream
             * It w
             * @return PStream& The stream ref
             */
            friend PStream &operator <<(PStream &,std::string_view);
        private:
            FILE *fptr = nullptr;
    };

    inline PStream::PStream(const char *cmd,const char *modes){
        fptr = BTK_POPEN(cmd,modes);
        if(fptr == nullptr){
            throwRuntimeError(strerror(errno));
        }
    }
    inline PStream::PStream(PStream &&pstream){
        fptr = pstream.fptr;
        pstream.fptr = nullptr;
    }
    inline PStream::~PStream(){
        close();
    }
    //Close stream
    inline void PStream::close(){
        if(fptr != nullptr){
            BTK_PCLOSE(fptr);
            fptr = nullptr;
        }
    }
    //Check it
    inline bool PStream::bad(){
        return std::ferror(fptr);
    }
    inline bool PStream::eof(){
        return std::feof(fptr);
    }
    //Stream open
    inline void PStream::open(const char *cmd,const char *modes){
        FILE *fp = BTK_POPEN(cmd,modes);
        if(fp == nullptr){
            throwRuntimeError(strerror(errno));
        }
        close();
        fptr = fp;
    }
    inline bool PStream::try_open(const char *cmd,const char *modes){
        FILE *fp = BTK_POPEN(cmd,modes);
        if(fp == nullptr){
            return false;
        }
        close();
        fptr = fp;
        return true;
    }
    //Readline
    inline PStream &operator >>(PStream &stream,std::string &str){
        int ch;
        while(not std::feof(*stream)){
            ch = std::fgetc(*stream);
            if(ch == EOF){
                break;
            }
            else if(ch == '\n'){
                break;
            }
            str += char(ch);
        }
        return stream;
    }
    //Write 
    inline PStream &operator <<(PStream &stream,std::string_view data){
        std::fwrite(data.data(),sizeof(char),data.length(),*stream);
        return stream;
    }
    //Assign
    inline PStream &PStream::operator =(PStream &&pstream){
        if(&pstream != this){
            close();
            fptr = pstream.fptr;
            pstream.fptr = nullptr;
        }
        return *this;
    }
};

#endif // _BTK_POPEN_HPP_
