#include "../build.hpp"

#include <Btk/utils/mem.hpp>
#include <cstddef>
#include <cctype>
namespace Btk{
    size_t U16Strlen(const char16_t *str){
        if(str == nullptr){
            return 0;
        }
        size_t len = 0;
        while(*str != u'\0'){
            ++len;
            ++str;
        }
        return len;
    }
    int U16Strcmp(const char16_t *s1,const char16_t *s2){
        if(s1 == s2){
            return 0;
        }
        if(s1 == nullptr or s2 == nullptr){
            return -1;
        }
        while(true){
            if(*s1 == *s2){
                if(*s1 == u'\0'){
                    //reach the end
                    return 0;
                }
                ++s1;
                ++s2;
                continue;
            }
            else{
                return *s1 > *s2;
            }
        }
    };
    int U16Strcasecmp(const char16_t *s1,const char16_t *s2){
        if(s1 == s2){
            return 0;
        }
        if(s1 == nullptr or s2 == nullptr){
            return -1;
        }
        while(true){
            if(*s1 == *s2){
                if(*s1 == u'\0'){
                    //reach the end
                    return 0;
                }
                ++s1;
                ++s2;
                continue;
            }
            //Try to use case cmp
            else{
                if(isalpha(*s1) and isalpha(*s2)){
                    if(tolower(*s1) == tolower(*s2)){
                        ++s1;
                        ++s2;
                        continue;
                    }
                }
                else{
                    return *s1 > *s2;
                }
            }
        }
    };
};