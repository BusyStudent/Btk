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
    //Process Handle

    #define BTK_PHANDLE   ::intptr_t
#else
    #include <unistd.h>
    #define BTK_READ      ::read
    #define BTK_WRITE     ::write
    #define BTK_CLOSE     ::close
    #define BTK_POPEN     ::popen
    #define BTK_PCLOSE    ::pclose
    #define BTK_PIPE(FDS) ::pipe(FDS)
    //Process Handle
    #define BTK_PHANDLE   ::pid_t
#endif

#include "alloca.hpp"

#include "../exception.hpp"
#include "../string.hpp"
#include "../defs.hpp"
namespace Btk{
    //Helper functions
    using process_t = BTK_PHANDLE;
    
    struct _vspawn_arg{
        const char *str;
        size_t n;
    };
    /**
     * @brief Open a new process with args
     * 
     * @internal Donnot use it directly
     * @param nargs 
     * @param args
     */
    BTKAPI process_t _vspawn(size_t nargs,const _vspawn_arg args[]);
    BTKAPI FILE*     _vpopen(size_t nargs,const _vspawn_arg args[]);
    //Translate args
    inline _vspawn_arg _spawn_tr_impl(const char *s){
        return {
            s,
            std::strlen(s)
        };
    }
    inline _vspawn_arg _spawn_tr_impl(const u8string &s){
        return {
            s.c_str(),
            s.size()
        };
    }
    inline _vspawn_arg _spawn_tr_impl(const std::string &s){
        return {
            s.c_str(),
            s.size()
        };
    }
    inline _vspawn_arg _spawn_tr_impl(u8string_view s){
        return {
            s.data(),
            s.size()
        };
    }
    inline _vspawn_arg _spawn_tr_impl(std::string_view s){
        return {
            s.data(),
            s.size()
        };
    }
    template<class T>
    inline _vspawn_arg _spawn_tr(T &&v){
        return _spawn_tr_impl(std::forward<T>(v));
    }
    template<size_t n>
    inline _vspawn_arg _spawn_tr(const char (&s)[n]){
        return {
            s,
            n
        };
    }
    //Helper end

    //Proc function begin
    template<class T,class ...Args>
    inline process_t spawn(T &&filename,Args &&...args){
        // return _spawn(
        //     sizeof...(Args) + 1,
        //     _spawn_tr(std::forward<T>(filename)),
        //     _spawn_tr(std::forward<Args>(args))...
        // );
        const _vspawn_arg arr [] = {
            _spawn_tr(std::forward<T>(filename)),
            _spawn_tr(std::forward<Args>(args))...
        };
        return _vspawn(sizeof...(Args) + 1,arr);
    }
    template<class T,class ...Args>
    inline FILE*     popen(T &&filename,Args &&...args){
        const _vspawn_arg arr [] = {
            _spawn_tr(std::forward<T>(filename)),
            _spawn_tr(std::forward<Args>(args))...
        };
        return _vpopen(sizeof...(Args) + 1,arr);
    }
    //version for StringList and StringRefList
    inline FILE*    vpopen(const StringList &strs){
        //auto args = (_vspawn_arg*) Btk_SmallAlloc(sizeof(_vspawn_arg) * strs.size());;
        _vspawn_arg args[strs.size()];
        
        for(size_t n = 0;n < strs.size();n++){
            args[n] = {strs[n].c_str(),strs[n].size()};
        }
        return _vpopen(strs.size(),args);
    }
    inline FILE*    vpopen(const StringRefList &strs){
        _vspawn_arg args[strs.size()];
        for(size_t n = 0;n < strs.size();n++){
            args[n] = {strs[n].data(),strs[n].size()};
        }
        return _vpopen(strs.size(),args);
    }
    //Proc function end

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
