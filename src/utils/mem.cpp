#include "../build.hpp"

#include <Btk/utils/mem.hpp>
#include <algorithm>
#include <cstddef>
#include <cctype>

#include "../libs/utf8.h"

namespace Btk{
    using utf8::unchecked::utf16to8;
    using utf8::unchecked::utf8to16;
    using std::back_insert_iterator;
        /**
     * @brief A helper class for count the length we inserted
     * 
     */
    template<class T>
    struct CountedBackInserter{
        CountedBackInserter(T &c):
            inserter(c){

        }
        CountedBackInserter& operator ++(){
            return *this;
        }
        CountedBackInserter  operator ++(int){
            return *this;
        }
        CountedBackInserter& operator *(){
            return *this;
        }
        template<class Value>
        CountedBackInserter& operator=(Value &&value){
            inserter = std::forward<Value>(value);
            length += 1;
            return *this;
        }
        back_insert_iterator<T> inserter;
        //The string length we inserted
        size_t length = 0;
    };
    
    size_t Utf16To8(u8string& ret,u16string_view _input){
        auto input = _input.base();
        auto iter = CountedBackInserter(ret.base());
        utf16to8(input.begin(),input.end(),iter);
        return iter.length;
    }
    size_t Utf8To16(u16string& ret,u8string_view _input){
        auto iter = CountedBackInserter(ret.base());
        auto input = _input.base();
        utf8to16(input.begin(),input.end(),iter);
        return iter.length;
    }
}
namespace Btk{
    //Parsing
    Sint64 ParseInt(u8string_view _str){
        auto str = _str.base();
        std::string_view::iterator iter;
        Sint64 value = 0;
        Sint64 n = 1;


        iter = (str.end() - 1);
        while(iter != (str.begin() - 1)){
            switch(*iter){
                case '0':
                    break;
                case '1':
                    value += n * 1;
                    break;
                case '2':
                    value += n * 2;
                    break;
                case '3':
                    value += n * 3;
                    break;
                case '4':
                    value += n * 4;
                    break;
                case '5':
                    value += n * 5;
                    break;
                case '6':
                    value += n * 6;
                    break;
                case '7':
                    value += n * 7;
                    break;
                case '8':
                    value += n * 8;
                    break;
                case '9':
                    value += n * 9;
                    break;
                case '-':
                    if(iter == str.begin()){
                        //Only alow at the text begin
                        value = - value;
                        break;
                    }
                    else{
                        //If is allspace at the prev
                        while(iter != str.begin()){
                            --iter;
                            if(*iter == '\t' or *iter == ' '){
                                continue;
                            }
                            else{
                                //error
                                return INT64_MAX;
                            }
                        }
                        return - value;
                    }
                case '\t':
                case ' ':
                    //Ignore space
                    --iter;
                    continue;
                default:
                    //error
                    return INT64_MAX;
            }
            n *= 10;
            --iter;
        }
        return value;
    }
    Sint64 ParseHex(u8string_view _str){
        auto str = _str.base();
        std::string_view::iterator iter;
        Sint64 value = 0;
        Sint64 n = 1;


        iter = (str.end() - 1);
        while(iter != (str.begin() - 1)){
            switch(*iter){
                case '0':
                    break;
                case '1':
                    value += n * 1;
                    break;
                case '2':
                    value += n * 2;
                    break;
                case '3':
                    value += n * 3;
                    break;
                case '4':
                    value += n * 4;
                    break;
                case '5':
                    value += n * 5;
                    break;
                case '6':
                    value += n * 6;
                    break;
                case '7':
                    value += n * 7;
                    break;
                case '8':
                    value += n * 8;
                    break;
                case '9':
                    value += n * 9;
                    break;
                case 'A':
                case 'a':
                    value += n * 10;
                    break;
                case 'B':
                case 'b':
                    value += n * 11;
                    break;
                case 'C':
                case 'c':
                    value += n * 12;
                    break;
                case 'D':
                case 'd':
                    value += n * 13;
                    break;
                case 'E':
                case 'e':
                    value += n * 14;
                    break;
                case 'F':
                case 'f':
                    value += n * 15;
                    break;
                case '-':
                    if(iter == str.begin()){
                        //Only alow at the text begin
                        value = - value;
                        break;
                    }
                    else{
                        //If is allspace at the prev
                        while(iter != str.begin()){
                            --iter;
                            if(*iter == '\t' or *iter == ' '){
                                continue;
                            }
                            else{
                                //error
                                return INT64_MAX;
                            }
                        }
                        return - value;
                    }
                case '\t':
                case 'x':
                case 'X':
                case ' ':
                    //Ignore space 0x etc
                    --iter;
                    continue;
                default:
                    //error
                    return INT64_MAX;
            }
            n *= 16;
            --iter;
        }
        return value;
    }
}