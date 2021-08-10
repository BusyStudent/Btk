#if !defined(_BTK_POPEN_HPP_)
#define _BTK_POPEN_HPP_

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <array>
#include <atomic>
#include <string>
#include <string_view>
#include <initializer_list>

#ifdef _WIN32
    #include <process.h>
    #include <fcntl.h>
    #include <io.h>
    #define BTK_READ      ::_read
    #define BTK_WRITE     ::_write
    #define BTK_CLOSE     ::_close
    #define BTK_POPEN     ::_popen
    #define BTK_PCLOSE    ::_pclose
    #define BTK_PIPE(FDS) ::_pipe(FDS,1024,_O_BINARY)
#else
    #include <unistd.h>
    #define BTK_READ      ::read
    #define BTK_WRITE     ::write
    #define BTK_CLOSE     ::close
    #define BTK_POPEN     ::popen
    #define BTK_PCLOSE    ::pclose
    #define BTK_PIPE(FDS) ::pipe(FDS)
#endif

#include "../exception.hpp"
#include "../string.hpp"
#include "../defs.hpp"
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
            friend PStream &operator >>(PStream &,u8string &);
            /**
             * @brief Write String to stream
             * It w
             * @return PStream& The stream ref
             */
            friend PStream &operator <<(PStream &,std::string_view);
            friend PStream &operator <<(PStream &,u8string_view);
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
    inline PStream &operator >>(PStream &stream,u8string &str){
        return operator >>(stream,str.base());
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

    /**
     * @brief Message Channal
     * 
     * @tparam T 
     */
    template<class T>
    class MessageChannal{
        public:
            static_assert(std::is_pod_v<T>);

            MessageChannal(){
                //Make pipe
                int fds[2];
                BTK_PIPE(fds);
                r = fds[0];
                w = fds[1];
            }
            MessageChannal(const MessageChannal &) = delete;
            MessageChannal(MessageChannal && c){
                r = c.r;
                w = c.w;
                //Set to invaild
                c.r = -1;
                c.w = -1;
            }
            ~MessageChannal(){
                BTK_CLOSE(r);
                BTK_CLOSE(w);
            }

            //Read and write
            /**
             * @brief Write a single value
             * 
             * @param value 
             */
            void write(const T &value){
                write(&value,1);
            }
            /**
             * @brief Write value
             * 
             * @param value The pointer to value
             * @param n How many
             */
            void write(const T *value,size_t n){
                auto ret = BTK_WRITE(w,value,sizeof(T) * n);
                items_count += ret / sizeof(T);
            }
            /**
             * @brief Write a value array
             * 
             * @tparam N 
             */
            template<size_t N>
            void write(const T (&arr)[N]){
                write(&arr,N);
            }
            template<size_t N>
            void write(const std::array<T,N> &arr){
                write(arr.data(),N);
            }
            /**
             * @brief Write a inited list
             * 
             * @param l 
             */
            void write(std::initializer_list<int> l){
                for(auto &v:l){
                    write(v);
                }
            }
            /**
             * @brief Read a single value
             * 
             * @param value 
             */
            void read(T &value){
                read(&value,1);
            }
            void read(T *value,size_t n){
                auto ret = BTK_READ(r,value,sizeof(T) * n);
                //Deincrease the count
                items_count -= ret / sizeof(T);
            }
            /**
             * @brief Read a value array
             * 
             * @tparam N 
             */
            template<size_t N>
            void read(T (&arr)[N]){
               read(&arr,N);
            }
            template<size_t N>
            void read(std::array<T,N> &arr){
                read(arr.data(),N);
            }
            //Helpers
            template<size_t n>
            std::array<T,n> read(){
                std::array<T,n> arr;
                read(arr);
                return arr;
            }
            T read(){
                T value;
                read(value);
                return value;
            }

            template<class ...Args>
            MessageChannal &operator <<(Args &&...args){
                write(std::forward<Args>(args)...);
                return *this;
            }
            template<class ...Args>
            MessageChannal &operator >>(Args &&...args){
                read(std::forward<Args>(args)...);
                return *this;
            }
            /**
             * @brief The channal is empty?
             * 
             * @return true 
             * @return false 
             */
            bool empty() const noexcept{
                return items_count == 0;
            }
        private:
            int r,w;
            std::atomic<size_t> items_count;
    };
};

#endif // _BTK_POPEN_HPP_
