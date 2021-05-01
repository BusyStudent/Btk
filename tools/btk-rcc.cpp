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
#include <cstdint>
#include <vector>

using Btk::getcwd;
using Btk::exists;

namespace{
    void process(std::ostream &output,std::istream &input){
        output << '{';

        std::vector<uint8_t> buf;
        while(not input.eof()){
            int ch = input.get();
            if(ch == EOF){
                break;
            }
            else{
                buf.push_back(ch);
            }
        }
        auto iter = buf.begin();
        for(;iter != buf.end();++iter){
            output << int(*iter);
            if(iter != buf.end() - 1){
                output << ',';
            }
        }

        output << "}";
    }
    void replace_char(std::string &s,char to,char from){
        std::string::size_type pos;
        pos = s.find(from);
        while(pos != s.npos){
            s[pos] = to;
            pos = s.find(from,pos + 1);
        }
    }
    void process_file(const std::string &file){
        std::fstream fs;
        fs.open(file,std::ios::binary | std::ios::in);
        if(not fs.is_open()){
            std::cout << "Couldnot open file " << file << std::endl;
            return;
        }
        auto pos = file.rfind('/');
        if(pos == file.npos){
            std::cout << "Couldnot get the filename" << std::endl;
            return;
        }

        std::string outputf,fname;
        outputf = file.substr(0,pos + 1);
        outputf += file.substr(pos + 1);
        fname = "data_";
        fname += file.substr(pos + 1);
        replace_char(fname,'_','.');
        replace_char(fname,'_',' ');
        outputf += ".h";
        

        std::fstream ofs;
        ofs.open(outputf,std::ios::out);
        if(not ofs.is_open()){
            std::cout << "Couldnot open file " << outputf << std::endl;
            return;
        }
        //Begin process
        ofs << R"(/*Generated by btk-rcc,don't edit it!!!*/)";
        ofs << "\n#include <stdint.h>\n";
        ofs << "static const uint8_t " << fname << " [] = ";
        process(ofs,fs);
        ofs << ';';
        //Done
    }
}
int main(int argc,const char **argv){
    if(argc == 1){
        process(std::cout,std::cin);
        
    }
    else{
        process_file(argv[1]);
    }
}