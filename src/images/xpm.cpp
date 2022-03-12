#include "../build.hpp"
#include "adapter.hpp"

#include <Btk/detail/scope.hpp>
#include <unordered_map>

namespace {
    SDL_Surface *load_xpm_from_string(SDL_RWops *rwops){
        size_t length;
        void *data = SDL_LoadFile_RW(rwops,&length,SDL_FALSE);

        if(data == nullptr){
            return nullptr;
        }
        Btk::SDLScopePtr guard(data);

        //Begin parser
        auto lines = Btk::u8string_view((char*)data,length).split_ref("\n");
        //find strings
        std::vector<char*> vecs;

        Btk_defer [&](){
            for(auto p:vecs){
                SDL_free(p);
            }
        };
        auto strndup = [](const char *str,size_t n) -> char *{
            auto s = (char *)SDL_malloc(n + 1);
            s[n] = '\0';
            memcpy(s,str,n);
            return s;
        };

        for(auto &v:lines){
            if(v.front() == '"' and (v.back() == '"' or v.back() == ',')){
                //Read "xxxxx",
                //Read "xxxxx"
                auto beg = v.begin() + 1;
                auto end = v.end() - 1;
                if(*end == ','){
                    //",
                    end = end - 2;
                }
                else if(*end == '"'){
                    end = end - 1;
                }
                Btk::u8string_view view(beg,end);
                vecs.push_back(strndup(view.data(),view.size()));
            }
        }
        //Send it to array parser
        return Btk::PixBuf::FromXPMArray(vecs.data()).detach();
    }
    #ifndef BTK_NO_XPM_WRITER
    bool save_xpm_from_buf(SDL_RWops *rwops,SDL_Surface *surf,int){
        if(surf->format->format != SDL_PIXELFORMAT_RGBA32){
            //Convert to RGBA32
            return save_xpm_from_buf(rwops,Btk::PixBufRef(surf).convert(SDL_PIXELFORMAT_RGBA32).get(),0);
        }
        Uint32 trpix = Btk::MapRGBA32(0,0,0,0);

        //Use std::string 
        std::string outbuf;
        std::unordered_map<Uint32,std::string> colormap;
        //Map RGBA -> Pixels String

        //Lock if needed
        if(SDL_MUSTLOCK(surf)){
            SDL_LockSurface(surf);
        }
        //Count pixels
        Uint32 *buf = static_cast<Uint32*>(surf->pixels);

        for(int y = 0;y < surf-> h; ++ y){
            for(int x = 0;x < surf->w; ++ x){
                Uint32 pix = buf[y * surf->h + x];
                //Create a place
                colormap[pix];
            }
        }
        //Dirty way
        //There is 95 printable chars
        constexpr int printable = 95;
        //Char peer pixels
        int cpp = std::ceil(float(colormap.size()) / printable);
        
        int char_begin = 0x20;
        int char_end = 0x7E;

        std::string pixstr;
        pixstr.resize(cpp);
        for(auto &c:pixstr){
            c = char_begin;
        }
        //Fill colormap
        int cur_position = 0;//position in pixstr
        for(int y = 0;y < surf-> h; ++ y){
            for(int x = 0;x < surf->w; ++ x){
                Uint32 pix = buf[y * surf->h + x];
                //Create a place
                auto &target = colormap[pix];
                if(not target.empty()){
                    //Generated 
                    continue;
                }
                target = pixstr;
                //Add a char
                pixstr[cur_position] =+ 1;
                if(pixstr[cur_position] == char_end){
                    //This position cannot hold more
                    cur_position += 1;
                }
            }
        }
        //colormap build done

        //Write header
        SDL_WriteString(rwops,"/* XPM */\n"
        "static const char *xpm[] = {\n"
        "/* columns rows colors chars-per-pixel */\n");
        //Write cpp
        SDL_RWpinrt(rwops,"\"%d %d %d %d \",\n",surf->w,surf->h,int(colormap.size()),cpp);
        //Write table
        for(auto &par:colormap){
            auto c = Btk::GetRGBA32(par.first);
            SDL_RWpinrt(rwops,"\"%s   c %h%h%h%h\",\n",par.second,c.r,c.g,c.b,c.a);
        }
        //Writw pixels
        pixstr.resize(surf->w * cpp);

        for(int y = 0;y < surf-> h; ++ y){
            SDL_WriteU8(rwops,'"');
            for(int x = 0;x < surf->w; ++ x){
                Uint32 pix = buf[y * surf->h + x];
                //Create a place
                auto &req = colormap[pix];

                memcpy(&pixstr[x * cpp],req.c_str(),req.size());
            }
            //Line end
            //Write buf
            SDL_RWwrite(rwops,pixstr.c_str(),sizeof(char),pixstr.size());
            SDL_WriteU8(rwops,'"');
            if(y != surf->w - 1){
                //Not Last line
                SDL_WriteU8(rwops,',');
            }
            SDL_WriteU8(rwops,'\n');
        }
        //end
        SDL_WriteString(rwops,"};\n");

        if(SDL_MUSTLOCK(surf)){
            SDL_UnlockSurface(surf);
        }

        return true;
    }
    #else
    #define save_xpm_from_buf nullptr
    #endif
}