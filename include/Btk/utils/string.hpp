#if !defined(_BTK_UTILS_STRING)
#define _BTK_UTILS_STRING
#include <string>
#include <string_view>
#include "mem.hpp"
#include "../defs.hpp"
namespace Btk{
    /**
     * @brief Utf16 encoded string
     * 
     */
    class BTKAPI String{
        public:
            String() = default;
            template<class ...Args>
            String(Args &&...args):str(std::forward<Args>(args)...){}
            /**
             * @brief Construct a new String object
             * 
             * @param u8str The utf8 encoded string
             */
            String(std::string_view u8str);
            String(std::u16string_view utf16);
            String(const String &) = default;
            String(String &&) = default;
            ~String() = default;
        public:
            #ifdef _WIN32
            //methods in windows
            static_assert(sizeof(wchar_t) == sizeof(char16_t));
            /**
             * @brief In windows char16_t to wchar_t
             */
            operator std::wstring_view() const{
                return {reinterpret_cast<const wchar_t*>(str.data()),str.length()};
            }
            String(const wchar_t *s):
                str(reinterpret_cast<const char16_t*>(s)){};
            String(std::wstring_view ws):
                str(reinterpret_cast<const char16_t*>(ws.data()),ws.length()){};

            #endif
        public:
            typedef std::u16string::iterator iterator;
            typedef std::u16string::const_iterator const_iterator;
            typedef std::u16string::reference reference;
            typedef std::u16string::const_reference const_reference;
            typedef std::u16string::size_type size_type;

            static constexpr size_type nops = std::u16string::npos;

            operator std::u16string_view() const{
                return str;
            }
            operator std::string() const{
                return Utf16To8(str);
            }

            std::u16string *operator ->() noexcept{
                return &str;
            }
            std::u16string &operator *() noexcept{
                return str;
            }
            const std::u16string *operator ->() const noexcept{
                return &str;
            }
            const  std::u16string &operator *() const noexcept{
                return str;
            }
            bool operator ==(std::u16string_view view) const{
                return str == view;
            }
            bool operator !=(std::u16string_view view) const{
                return str != view;
            }
            bool operator ==(const String &s) const{
                return str == s.str;
            }
            bool operator !=(const String &s) const{
                return str != s.str;
            }
            
            String &operator =(std::u16string_view u16){
                str = u16;
                return *this;
            }
            String &operator =(const String &u16){
                str = u16;
                return *this;
            }
            String &operator =(String &&u16){
                str = std::move(u16);
                return *this;
            }

            String &operator +=(std::u16string_view u16){
                str += u16;
                return *this;
            }
            String &operator +=(std::string_view u8){
                Utf8To16(str,u8);
                return *this;
            }
            String &operator +=(const String &s){
                str += s.str;
                return *this;
            }


            String operator +(std::u16string_view u16) const{
                String s;
                s.str = str;
                s.str += u16;
                return s;
            }
            String operator +(std::string_view u8) const{
                String s;
                s.str = str;
                Utf8To16(s.str,u8);
                return s;
            }
            String operator +(const String &s) const{
                return str + s.str;
            }

            reference operator [](size_type size){
                return str[size];
            }
            const_reference operator [](size_type size) const{
                return str[size];
            }
        public:
            /**
             * @brief String is empty?
             * 
             * @return true 
             * @return false 
             */
            bool empty() const{
                return str.empty();
            }
            size_t size() const{
                return str.size();
            }
            size_t length() const{
                return str.length();
            }

            iterator begin(){
                return str.begin();
            }
            iterator end(){
                return str.end();
            }
            const_iterator begin() const{
                return str.begin();
            }
            const_iterator end() const{
                return str.end();
            }
            /**
             * @brief Find chars by
             * 
             * @tparam Args 
             * @param args 
             * @return size_type 
             */
            template<class ...Args>
            size_type find(Args &&...args) const{
                return str.find(std::forward<Args>(args)...);
            }

            template<class ...Args>
            iterator index(Args &&...args){
                size_type i = find(std::forward<Args>(args)...);
                if(i == nops){
                    return end();
                }
                return begin() + i;
            }
            template<class ...Args>
            iterator index(Args &&...args) const{
                size_type i = find(std::forward<Args>(args)...);
                if(i == nops){
                    return end();
                }
                return begin() + i;
            }

            //Append text
            String &append(std::u16string_view txt){
                str += txt;
                return *this;
            }
            String &append(const String &u16){
                str += u16.str;
                return *this;
            }
            String &append(std::string_view u8){
                Utf8To16(str,u8);
                return *this;
            }
            /**
             * @brief Trim space and tab in the string's begin and end
             * 
             */
            String trim() const;
            /**
             * @brief convert string to utf8 string
             * 
             * @return std::string 
             */
            std::string to_utf8() const{
                return Utf16To8(str);
            }
            /**
             * @brief Change encoding by iconv
             * 
             * @param to target code(default to locale)
             * @return std::string 
             */
            std::string encode(const char *to = "") const;
        public:
            //static methods
            static String Format(const char16_t *fmt,...);
            template<class ...Args>
            static String format(const char16_t *fmt,Args &&...args){
                return Format(fmt,std::forward<Args>(args)...);
            }
        private:
            std::u16string str;
    };


    inline String::String(std::string_view utf8):String(){
        Utf8To16(str,utf8);
    }
    inline String::String(std::u16string_view u16):str(u16){

    }
    
    using u16string = String;
}



#endif // _BTK_UTILS_STRING
