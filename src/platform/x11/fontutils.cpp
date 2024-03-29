#include <fontconfig/fontconfig.h>
#include <langinfo.h>

#include "../../build.hpp"
#include <Btk/platform/x11.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/exception.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>

#include <SDL2/SDL_mutex.h>

#include <mutex>
namespace Btk{
namespace FontUtils{
    static bool was_init = false;
    static FcConfig *config = nullptr;
    static SDL_mutex *mutex = nullptr;
    //mutex for multi threading
    struct LockGuard{
        LockGuard(){
            SDL_LockMutex(mutex);
        }
        LockGuard(const LockGuard &) = delete;
        ~LockGuard(){
            SDL_UnlockMutex(mutex);
        }
    };
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
            mutex = SDL_CreateMutex();
            was_init = true;
            //Register atexit callback
            std::call_once(flag,[](){
                Btk::Init();
                Btk::AtExit(FontUtils::Quit);
            });
        }
    }
    void Quit(){
        if(was_init){
            //Destroy config
            FcConfigDestroy(config);
            FcFini();
            SDL_DestroyMutex(mutex);
            config = nullptr;
            mutex = nullptr;
            was_init = false;
        }
    }
}
namespace FontUtils{
    //Get font file
    FontInfo FindFont(u8string_view fontname){
        if(not was_init){
            Init();
        }
        FcPattern* font;
        FcPattern* pat = FcNameParse(
            reinterpret_cast<const FcChar8*>(fontname.data())   
        );
        LockGuard locker;//Lock the library
        //Config substitute
        FcConfigSubstitute(config, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);
        //Begin match
        FcResult result;
        font = FcFontMatch(config,pat,&result);
        FontInfo info;

        if(font == nullptr){
            goto err;
        }
        FcChar8 *file;
        if(FcPatternGetString(font,FC_FILE,0,&file) != FcResultMatch){
            //Get String
            goto err;
        }
        FcChar8 *fullname;
        if(FcPatternGetString(font,FC_FULLNAME,0,&fullname) != FcResultMatch){
            //Get String
            goto err;
        }
        int index;
        if(FcPatternGetInteger(font,FC_INDEX,0,&index) != FcResultMatch){
            //Get index
            goto err;
        }
        info.filename = reinterpret_cast<char*>(file);
        info.fullname = reinterpret_cast<char*>(fullname);
        info.index = index;
        FcPatternDestroy(font);
        FcPatternDestroy(pat);
        return info;

        err:
            FcPatternDestroy(font);
            FcPatternDestroy(pat);
            throwRuntimeError("Failed to match font");
    }
    u8string GetFileByName(u8string_view fontname){
        if(not was_init){
            Init();
        }
        FcPattern* font;
        FcPattern* pat = FcNameParse(
            reinterpret_cast<const FcChar8*>(fontname.data())   
        );
        LockGuard locker;//Lock the library
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
        BTK_LOGINFO("FcMatch %s => %s",fontname.data(),str);
        u8string s(reinterpret_cast<char*>(str));
        FcPatternDestroy(font);
        FcPatternDestroy(pat);
        return s;
    }
    u16string GetFileByName(u16string_view fontname){
        if(not was_init){
            Init();
        }
        FcPattern* font;
        FcPattern* pat = FcNameParse(
            reinterpret_cast<const FcChar8*>(fontname.to_utf8().c_str())   
        );
        LockGuard locker;//Lock the library
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
        BTK_LOGINFO("FcMatch %s => %s",fontname.to_utf8().c_str(),str);
        u16string s(reinterpret_cast<char*>(str));
        FcPatternDestroy(font);
        FcPatternDestroy(pat);
        return s;
    }
    //Utf16 get font name
    // u16string  GetFileByName(std::u16string_view name){
    //     auto &u8buf = InternalU8Buffer();
    //     u8buf.clear();//clear the buffer
    //     Utf16To8(u8buf,name);

    //     return GetFileByName(u8buf);
    // }
    u8string GetDefaultFont(){
        char *lan = nl_langinfo(_NL_IDENTIFICATION_LANGUAGE);
        return GetFileByName("");
    }

    FontSet GetFontList(){
        if(not was_init){
            Init();
        }
        LockGuard locker;//Lock the library
        FcPattern *pat = FcPatternCreate();
        FcObjectSet *objset = FcObjectSetBuild(FC_FAMILY,FC_STYLE,FC_LANG,FC_FILE,nullptr);
        FcFontSet *fontset = FcFontList(config,pat,objset);
        
        BTK_ASSERT(fontset and objset and pat);

        BTK_LOGINFO("FcFontList => fonts count:%d",fontset->nfont);
        //FcFontSetPrint(fontset);
        return {pat,objset,fontset};
    }
}
    FontSet::~FontSet(){
        FcPatternDestroy(static_cast<FcPattern*>(fc_pat));
        FcObjectSetDestroy(static_cast<FcObjectSet*>(fc_objset));
        FcFontSetDestroy(static_cast<FcFontSet*>(fc_fontset));
    }
    //Move assign
    FontSet &FontSet::operator =(FontSet &&set){
        if(&set != this){
            FcPatternDestroy(static_cast<FcPattern*>(fc_pat));
            FcObjectSetDestroy(static_cast<FcObjectSet*>(fc_objset));
            FcFontSetDestroy(static_cast<FcFontSet*>(fc_fontset));

            fc_pat = set.fc_pat;
            fc_objset = set.fc_objset;
            fc_fontset = set.fc_fontset;

            set.fc_pat = nullptr;
            set.fc_objset = nullptr;
            set.fc_fontset = nullptr;
        }
        return *this;
    }
    /**
     * nfont means the size of the patterns
     * sfont means the size of the array
     */
    size_t FontSet::size() const{
        auto fset = static_cast<FcFontSet*>(fc_fontset);
        BTK_ASSERT(fset->nfont >= 0);
        return fset->nfont;
    }
    FontSet::Font FontSet::operator [](size_t index) const{
        auto fset = static_cast<FcFontSet*>(fc_fontset);
        if(index > fset->nfont){
            throwRuntimeError(u8format("Out of range %zu:%d",index,fset->nfont).c_str());
        }
        return {fset->fonts[index]};
    }

    //FontSet::Font
    u8string_view FontSet::Font::family() const{
        FcChar8 *str = nullptr;
        auto pat = static_cast<FcPattern*>(font);
        if(FcPatternGetString(pat,FC_FAMILY,0,&str) != FcResultMatch){
            throwRuntimeError("Failed to match font");
        }
        else{
            return reinterpret_cast<char*>(str);
        }
    }
    u8string_view FontSet::Font::style() const{
        FcChar8 *str = nullptr;
        auto pat = static_cast<FcPattern*>(font);
        if(FcPatternGetString(pat,FC_STYLE,0,&str) != FcResultMatch){
            throwRuntimeError("Failed to match font");
        }
        else{
            return reinterpret_cast<char*>(str);
        }
    }
    u8string_view FontSet::Font::file() const{
        FcChar8 *str = nullptr;
        auto pat = static_cast<FcPattern*>(font);
        if(FcPatternGetString(pat,FC_FILE,0,&str) != FcResultMatch){
            throwRuntimeError("Failed to match font");
        }
        else{
            return reinterpret_cast<char*>(str);
        }
    }
    //FontSet::Iterator
    FontSet::Iterator &FontSet::Iterator::operator++(){
        auto fset = static_cast<FcFontSet*>(set->fc_fontset);

        if(index + 1 < fset->nfont){
            ++index;
            font.font = fset->fonts[index];
        }
        else{
            //reach at the end
            font.font = nullptr;
        }
        return *this;
    }
    FontSet::Iterator &FontSet::Iterator::operator--(){
        auto fset = static_cast<FcFontSet*>(set->fc_fontset);
        if(index > 0){
            --index;
            font.font = fset->fonts[index];
        }
        return *this;
    }
    //Get iterator
    FontSet::Iterator FontSet::begin(){
        auto fset = static_cast<FcFontSet*>(fc_fontset);
        if(fset->nfont <= 0){
            return {{nullptr},this,0};
        }
        return {{fset->fonts[0]},this,0};
    }
    FontSet::Iterator FontSet::end(){
        auto fset = static_cast<FcFontSet*>(fc_fontset);
        if(fset->nfont <= 0){
            return {{nullptr},this,0};
        }
        return {{nullptr},this,static_cast<size_t>(fset->nfont)};
    }

    //FontMatcher
    FontMatcher::FontMatcher(){
        pattern = FcPatternCreate();
    }
    FontMatcher::~FontMatcher(){
        FcPatternDestroy(static_cast<FcPattern*>(pattern));
    }
};