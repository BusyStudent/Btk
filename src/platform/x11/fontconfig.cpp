#include <SDL2/SDL_ttf.h>
#include <fontconfig/fontconfig.h>

#include "../../build.hpp"
#include <Btk/platform/x11.hpp>
#include <Btk/exception.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <mutex>
namespace Btk{
namespace FontUtils{
    static bool was_init = false;
    static FcConfig *config = nullptr;
    void Init(){
        static std::once_flag flag;
        if(not was_init){
            if(FcInit() == FcFalse){
                //Init fail
                throwRuntimeError("FcInit() failed");
            }
            config = FcInitLoadConfigAndFonts();
            if(config == nullptr){
                //FIXME
                //What should i do??
            }
            was_init = true;
            //Register atexit callback
            std::call_once(flag,[](){
                Btk::Init();
                Btk::AtExit(FontUtils::Quit);
            });
        }
    };
    void Quit(){
        if(was_init){
            //Destroy config
            FcConfigDestroy(config);
            FcFini();
            config = nullptr;
            was_init = false;
        }
    };
};
namespace FontUtils{
    //Get font file
    std::string GetFileByName(std::string_view fontname){
        if(not was_init){
            Init();
        }
        FcPattern* font;
        FcPattern* pat = FcNameParse(
            reinterpret_cast<const FcChar8*>(fontname.data())   
        );
        //Config substitute
        FcConfigSubstitute(config, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);
        //Begin match
        FcResult result;
        font = FcFontMatch(config,pat,&result);
        
        if(font == nullptr){
            FcPatternDestroy(pat);
            throwRuntimeError("Failed to match font");
        }
        FcChar8 *str;
        if(FcPatternGetString(font,FC_FILE,0,&str) != FcResultMatch){
            //Get String
            FcPatternDestroy(font);
            FcPatternDestroy(pat);
            throwRuntimeError("Failed to match font");
        }
        std::string s(reinterpret_cast<char*>(str));
        FcPatternDestroy(font);
        FcPatternDestroy(pat);
        return s;
    }
};
};