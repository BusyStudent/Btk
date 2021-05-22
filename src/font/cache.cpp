#include "../build.hpp"

#include <Btk/font/system.hpp>
#include <Btk/font/font.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>

namespace Btk::Ft{
    static CacheSystem *system = nullptr;
    static void sys_cleanup(){
        delete system;
        FT_Done_FreeType(Ft2Library);
    }
    CacheSystem &GlobalCache(){
        if(system == nullptr){
            FT_Init_FreeType(&Ft2Library);
            system = new CacheSystem;
            AtExit(sys_cleanup);
        }
        return *system;
    }
    bool CacheSystem::load_face(Face &f,const char *filename,Uint32 idx){
        Face face;
        face.face = new Ft2Face(filename,idx);
        f = face;
        return true;
    }
    Font *CacheSystem::query(const u8string &name){
        auto iter = faces.find(name);
        if(iter == faces.end()){
            return nullptr;
        }
        return new Font(iter->second);
    }
}