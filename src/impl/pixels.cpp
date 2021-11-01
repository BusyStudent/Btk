#include <SDL2/SDL.h>
#ifdef BTK_HAS_SDLIMG
    #include <SDL2/SDL_image.h>
#endif
#ifdef BTK_USE_GFX
    #include <Btk/thirdparty/SDL2_rotozoom.h>
#else
    #define STB_IMAGE_RESIZE_STATIC
    #define STB_IMAGE_RESIZE_IMPLEMENTATION
    #define STBIR_MALLOC  SDL_malloc
    #define STBIR_REALLOC SDL_realloc
    #define STBIR_FREE    SDL_free
    #include "../libs/stb_image_resize.h"
#endif

#include "../build.hpp"

#include <Btk/impl/scope.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/pixels.hpp>
#include <Btk/rwops.hpp>
#include <Btk/rect.hpp>
#include <ostream>
namespace Btk{
    //Load XPM DATA
    static inline PixBuf load_xpm_from(const char *const*);

    PixBuf::~PixBuf(){
        SDL_FreeSurface(surf);
    }
    PixBuf::PixBuf(u8string_view file){
        surf = LoadImage(RWops::FromFile(file.data(),"rb").get());
        if(surf == nullptr){
            throwSDLError();
        }
    }
    PixBuf::PixBuf(int w,int h,Uint32 fmt){
        surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,
            h,
            SDL_BYTESPERPIXEL(fmt),
            fmt
        );
        if(surf == nullptr){
            throwSDLError();
        }
    }
    PixBuf PixBuf::ref() const{
        surf->refcount ++;
        return PixBuf(surf);
    }
    PixBuf PixBuf::clone() const{
        SDL_Surface *surf = SDL_DuplicateSurface(this->surf);
        if(surf == nullptr){
            throwSDLError(SDL_GetError());
        }
        else{
            return PixBuf(surf);
        }
    }
    //Lock
    void PixBufRef::lock() const{
        if(not must_lock()){
            return;
        }
        if(SDL_LockSurface(surf) != 0){
            throwSDLError();
        }
    }
    void PixBufRef::unlock() const noexcept{
        if(not must_lock()){
            return;
        }
        SDL_UnlockSurface(surf);
    }
    void PixBufRef::set_rle(bool val){
        if(SDL_SetSurfaceRLE(surf,val) == -1){
            throwSDLError();
        }
    }
    //save PixBuf
    void PixBufRef::save_bmp(RWops &rw){
        if(SDL_SaveBMP_RW(surf,rw.get(),false) == -1){
            throwSDLError();
        }
    }
    void PixBufRef::save_bmp(u8string_view fname){
        auto rw = RWops::FromFile(fname.data(),"wb");
        PixBufRef::save_bmp(rw);
    }
    void PixBufRef::save(RWops &rw,u8string_view type,int quality){
        if(surf == nullptr){
            throwSDLError("empty pixbuf");
        }
        SaveImage(rw.get(),surf,type,quality);
    }
    void PixBufRef::save(u8string_view filename,u8string_view type,int quality){
        auto rw = RWops::FromFile(filename.data(),"wb");
        if(type.empty()){
            //TODO add code to detect type from filename
            size_t pos = filename.rfind('.');
            if(pos != filename.npos){
                //try to get the ext name
                auto ntype = filename.substr(pos + 1).strip();
                BTK_LOGINFO("Get extension name '%s'",ntype.c_str());
                PixBufRef::save(rw,ntype,quality);
                return;
            }
            //No ext name abliable
        }
        PixBufRef::save(rw,type,quality);
    }
    //operators
    PixBuf &PixBuf::operator =(SDL_Surface *sf){
        SDL_FreeSurface(surf);
        surf = sf;
        return *this;
    }
    PixBuf &PixBuf::operator =(PixBuf &&sf){
        SDL_FreeSurface(surf);
        surf = sf.surf;
        sf.surf = nullptr;
        return *this;
    }
    PixBuf PixBufRef::convert(Uint32 fmt) const{
        SDL_Surface *surf = SDL_ConvertSurfaceFormat(this->surf,fmt,0);
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
    }
    PixBuf PixBufRef::zoom(double w_f,double_t h_f){
        #ifdef BTK_USE_GFX
        SDL_Surface *surf = zoomSurface(this->surf,w_f,h_f,SMOOTHING_ON);
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
        #else
        BTK_UNIMPLEMENTED();
        #endif
    }
    void PixBufRef::bilt(PixBufRef buf,const Rect *src,Rect *dst){
        if(SDL_BlitSurface(buf.get(),src,surf,dst) != 0){
            throwSDLError();
        }
    }
    void PixBuf::begin_mut(){
        if(empty()){
            return;
        }
        if(surf->refcount != 1){
            //Copy it and unref
            SDL_Surface *new_surf = SDL_DuplicateSurface(
                surf
            );
            if(new_surf == nullptr){
                throwSDLError();
            }
            SDL_FreeSurface(surf);
            surf = new_surf;
        }
    }
    //static method
    PixBuf PixBuf::FromMem(const void *mem,size_t size){
        auto rw = RWops::FromMem(mem,size);
        return FromRWops(rw);
    }
    PixBuf PixBuf::FromFile(u8string_view file){
        return PixBuf(file);
    }
    PixBuf PixBuf::FromFile(FILE *f){
        auto rw = RWops::FromFP(f,false);
        return FromRWops(rw);
    }
    PixBuf PixBuf::FromXPMArray(const char *const*da){
        return load_xpm_from(da);
    }
    PixBuf PixBuf::FromRWops(RWops &rwops){
        SDL_Surface *surf = LoadImage(rwops.get());
        if(surf == nullptr){
            throwSDLError();
        }
        return PixBuf(surf);
    }
}
namespace Btk{
    //PixelFormat
    PixFmt::PixFmt(Uint32 pixfmt){
        fmt = SDL_AllocFormat(pixfmt);
        if(fmt == nullptr){
            throwSDLError();
        }
    }
    PixFmt::~PixFmt(){
        SDL_FreeFormat(fmt);
    }
    //Map RGB
    Uint32 PixFmt::map_rgb (Uint8 r,Uint8 g,Uint8 b) const{
        return SDL_MapRGB(fmt,r,g,b);
    }
    Uint32 PixFmt::map_rgba(Uint8 r,Uint8 g,Uint8 b,Uint8 a) const{
        return SDL_MapRGBA(fmt,r,g,b,a);
    }
    //Get names
    u8string_view PixFmt::name() const{
        return SDL_GetPixelFormatName(fmt->format);
    }
    std::ostream &operator <<(std::ostream &os,Color c){
        os << '(' << int(c.r) << ',' << int(c.g) << ',' << int(c.b) << ',' << int(c.a) << ')';
        return os;
    }
    Color ParseColor(u8string_view view){
        auto base = view.base();
        Color c;
        if(base[0] == '#'){
            //Like #RRGGBBAA
            base = base.substr(1);
        }
        if(base.length() != 6 and base.length() != 8){
            //bad color
            throwRuntimeError("Bad color");
        }
        c.r = ParseHex(base.substr(0,2));
        c.g = ParseHex(base.substr(2,2));
        c.b = ParseHex(base.substr(4,2));
        if(base.length() == 8){
            c.a = ParseHex(base.substr(6,2));
        }
        else{
            c.a = 255;
        }
        return c;
    }
    //Decoder / Encoder
    void ImageDecoder::open(SDL_RWops *rwops,bool autoclose){
        fstream = rwops;
        auto_close = autoclose;
        decoder_open();
        _is_opened = true;
    }
    void ImageDecoder::open(u8string_view filename){
        SDL_RWops *rwops = SDL_RWFromFile(filename.data(),"rb");
        if(rwops == nullptr){
            throwSDLError();
        }
        return open(rwops,true);
    }
    void ImageDecoder::close(){
        if(_is_opened){
            Btk_defer [=](){
                if(auto_close){
                    SDL_RWclose(fstream);
                }
            };
            _is_opened = false;
            decoder_close();
        }
    }
    PixBuf ImageDecoder::read_frame(size_t frame_idx,const Rect *r){
        Rect rect;
        if(r == nullptr){
            rect.x = 0;
            rect.y = 0;
            auto[w,h] = frame_size(frame_idx);
            rect.w = w;
            rect.h = h;
        }
        else{
            rect = *r;
        }
        PixBuf buf(rect.w,rect.h,container_format());
        read_pixels(frame_idx,r,buf->pixels);
        return buf;
    }
    ImageDecoder::~ImageDecoder(){
        close();
    }
}
//XPM
#include <unordered_map>
#include <string_view>
namespace Btk{
    // #ifdef BTK_HAS_SDLIMG
    #ifdef BTK_HAS_SDLIMG
    static inline PixBuf load_xpm_from(const char *const*v){
        auto surf = IMG_ReadXPMFromArray(const_cast<char**>(v));
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
    }
    #else
    static inline PixBuf load_xpm_from(const char *const*arr){
        //using our's parser
        if(arr == nullptr){
            throwRuntimeError("Null array");
        }
        //First read xpm info
        int w,h,colors_count,cpp;
        if(sscanf(arr[0],"%d %d %d %d",&w,&h,&colors_count,&cpp) != 4){
            throwRuntimeError("Bad xpm");
        }
        //Cpp is the len of color name

        //Make map
        std::unordered_map<std::string_view,Color> colormap = {
            //builtin color
            {"none" ,Color(0,0,0,0)},
            {"white" ,Color(255,255,255)},
            {"black",Color(0,0,0)},
            {"red",Color(255,0,0)},
            {"green",Color(0,255,0)},
            {"blue",Color(0,0,255)},
        };
        colormap.rehash(colormap.size() + colors_count);

        //For array add all colors
        size_t max_line = colors_count + h;
        for(size_t idx = 1;idx < max_line + 1;idx++){
            const char *line = arr[idx];
            if(line[cpp + 1] != 'c'){
                //COLOR c 0x...
                //Is not color
                continue;
            }
            std::string_view view(line,cpp);
            std::string_view colorinfo(&(line[cpp + 3]));
            //Got it color
            Color color;
            if(colorinfo[0] != '#'){
                //Using the color in the table
                auto iter = colormap.find(colorinfo);
                if(iter != colormap.end()){
                    //founded
                    color = iter->second;
                    colormap.insert(std::make_pair(view,color));
                    continue;
                }
                //Not founded,Try ignore the case
                for(auto &par:colormap){
                    if(par.first.length() == colorinfo.length()){
                        //Same length,Compare ignore the case
                        int ret = strncasecmp(
                            par.first.data(),
                            colorinfo.data(),
                            colorinfo.length()
                        );
                        if(ret == 0){
                            //Founded
                            color = par.second;
                            colormap.insert(std::make_pair(view,color));
                            continue;
                        }
                    }
                }
                //Not founded
                throwRuntimeError("Bad xpm");
            }
            else{
                color = ParseColor(colorinfo);
                colormap.insert(std::make_pair(view,color));
            }
        }
        //Begin parse the pixels
        int x = 0,y = 0;
        //Make pixbuf
        PixBuf buffer(w,h,PixelFormat::RGBA32);
        Uint32 *pixels = static_cast<Uint32*>(buffer->pixels);
        for(size_t idx = 1;idx < max_line + 1;idx++){
            const char *line = arr[idx];
            if(line[cpp + 1] == 'c'){
                //COLOR c 0x...
                //Is not color
                continue;
            }
            for(size_t n = 0;n < w * cpp;n += cpp){
                std::string_view pix(line + n,cpp);
                //Get pixel string
                auto color = colormap.at(pix);
                pixels[w * y + x] = MapRGBA32(color);
                x += 1;
            }
            //Reset x,increase y to next line
            x  = 0;
            y += 1;
        }
        return buffer;
    }
    #endif
}