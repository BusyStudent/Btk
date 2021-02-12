#include "../build.hpp"

#include <Btk/font/gdi.hpp>
#include <windows.h>
#include <wingdi.h>

namespace BtkFt{
    
    DGIFont::~DGIFont(){
        DeleteObject(font);
    }
    bool DGIFont::open(const u16string &facename,int ptsize){
        HFONT nfont = CreateFontW(
            0,//default
            0,
            0,
            0,
            FW_DONTCARE,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            DEFAULT_PITCH,
            reinterpret_cast<LPCWSTR>(facename->c_str())
        );
        if(nfont != nullptr){
            //succeed
            DeleteObject(font);
            font = nfont;
            return true;
        }
        return false;
    }
}