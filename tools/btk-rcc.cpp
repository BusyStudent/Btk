extern "C"{
    #include "../src/libs/ini.h"
    #include "../src/libs/ini.c"
}
#include <Btk/platform/fs.hpp>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

using Btk::getcwd;
using Btk::exists;

namespace{
    using cstring = const char *;
    bool cmp(cstring s1,cstring s2){
        #ifdef _WIN32
        return ::_stricmp(s1,s2) == 0;
        #else
        return ::strcasecmp(s1,s2) == 0;
        #endif
    }
    struct Config{
        std::string input_file;
        std::string output_dir;

        bool strict = true;
    };
    /**
     * @brief Error happended
     * 
     * @tparam Args 
     * @param args 
     */
    template<class ...Args>
    void panic(Args &&...args){
        //std::cerr << std::forward<Args>(args...);
        std::cerr << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int  handler(Config &conf,cstring section,
                 cstring name, cstring value){

        if(section == nullptr){
            if(cmp(name,"strict")){

            }
            else if(conf.strict){
                //Is strict mode
                panic("Invaid ",name,':',value);
            }
        }
        return 1;
    }
    /**
     * @brief Process source file
     * 
     * @param conf The config
     */
    void process(Config &conf){
        //a wrapper for handler
        auto ini_wrapper = [](void *c,cstring s,cstring n,cstring v) -> int{
            return handler(*static_cast<Config*>(c),s,n,v);
        };
        if(ini_parse(conf.input_file.c_str(),ini_wrapper,&conf) == -1){
            std::cerr << "Couldnot open file "<< conf.input_file << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    /**
     * @brief Parse command line args
     * 
     * @param argc 
     * @param argv 
     * @return Config 
     */
    Config parse_arg(int argc,const char **argv){
        if(argc == 1){
            std::cout << "Useage btk-rcc [sourcefile.conf]" << std::endl;
            std::exit(EXIT_SUCCESS);
        }
        Config config;
        for(int i = 1;i < argc;i ++){
            const char *arg = argv[i];
            //Is option
            if(arg[0] == '-'){
                //TODO
            }
            else{
                //sourcefile
                config.input_file = arg;
            }
        }
        return config;
    }
}
int main(int argc,const char **argv){
    auto conf = parse_arg(argc,argv);
    process(conf);
    return EXIT_SUCCESS;
}