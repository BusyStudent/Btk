#if !defined(_BTK_REGEX_HPP_)
#define _BTK_REGEX_HPP_
#include "defs.hpp"
#include "string.hpp"
#include "exception.hpp"

#ifndef BTK_REGEX_IMPL
    #define BTK_REGEX_IMPL void
    #define BTK_REGEX_RESULT void
#endif

namespace Btk{
    class BTKAPI RegexError:public RuntimeError{
        public:
            /**
             * @brief Construct a new Regex Error objectP
             * @internal The constructor is internal,Donnot use it
             * @param private_ The private
             */
            BTKHIDDEN
            explicit RegexError(void *private_);
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
    };
    //TODO ADD regex
    class BTKAPI Regex{
        public:
            enum Flags:Uint32{
                Basic = 0,//Posix Expression
                Extended = 1 << 0,//Posix Extend expression
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
            Regex(u8string_view exp,Flags flags = Flags::Basic);
            ~Regex();
            /**
             * @brief Get the compiled regex expression
             * 
             * @return u8string_view 
             */
            u8string_view expression() const;
        private:
            BTK_REGEX_IMPL *regex = nullptr;
    };
    BTK_FLAGS_OPERATOR(Regex::Flags,Uint32);
    class BTKAPI MatchResult{
        public:
            MatchResult();
            ~MatchResult();
        private:
            BTK_REGEX_RESULT *result;
    };
}

#endif // _BTK_REGEX_HPP_
