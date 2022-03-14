#if !defined(_BTK_REGEX_HPP_)
#define _BTK_REGEX_HPP_
#include "defs.hpp"
#include "string.hpp"
#include "exception.hpp"

namespace Btk{
    class BTKAPI RegexError:public RuntimeError{
        public:
            /**
             * @brief Construct a new Regex Error object
             */
            RegexError() = default;
            RegexError(const RegexError &) = default;
            ~RegexError();
            /**
             * @brief Get the regex expression
             * 
             * @return u8string_view 
             */
            u8string_view expression() const noexcept{
                return _expression;
            }
        private:
            u8string _expression;
        friend class Regex;
    };
    /**
     * @brief Simple regex utils
     * 
     */
    class BTKAPI Regex{
        public:
            enum Flags:Uint32{
                Basic = 0,//Posix Extend Expression
                ECMAScript = 1 << 1,//< ECMAScript 
                ICase = 1<< 2//< Ignore the case
            };
        public:
            Regex() = default;
            /**
             * @brief Construct a new Regex object
             * 
             * @param r 
             */
            Regex(Regex &&r){
                regex = r.regex;
                r.regex = nullptr;
            }
            /**
             * @brief Construct a new Regex object
             * 
             * @param exp The Regex expression
             * @param flags The Expression Flags
             */
            Regex(u8string_view exp,Flags flags = Flags::Basic):Regex(){
                compile(exp,flags);
            }
            ~Regex(){
                cleanup();
            }

            void compile(u8string_view exp,Flags flags = Flags::Basic);
            void cleanup();
            /**
             * @brief Get the compiled regex expression
             * 
             * @return u8string_view 
             */
            auto expression() const -> u8string_view;
            
            auto match(const u8string &str,size_t max = size_t(-1)) -> StringList;
            void replace(u8string &str,u8string_view to,size_t max = size_t(-1));

            auto replace_to(u8string_view str,u8string_view to,size_t max = size_t(-1)) -> u8string{
                u8string input(str);
                replace(input,to,max);
                return input;
            }

            //Assign
            Regex &operator =(Regex &&re){
                if(this == &re){
                    return *this;
                }
                regex = re.regex;
                re.regex = nullptr;
                return *this;
            }
        private:
            struct Impl;
            Impl *regex = nullptr;
    };
    BTK_FLAGS_OPERATOR(Regex::Flags,Uint32);
}

#endif // _BTK_REGEX_HPP_
