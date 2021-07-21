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
        //Try to use the cache
        if(face->family_name != nullptr){
            auto iter = faces.find(face->family_name);
            if(iter != faces.end() and iter->second->face_index == idx){
                //It is exists and hit the cache
                face = iter->second;
                BTK_LOGINFO("[System::Font]HitCache %s in openfont %s %u",face->family_name,filename,idx);
            }
            else{
                //Add into cache
                BTK_LOGINFO("[System::Font]Add Face %s",face->family_name);
                faces[face->family_name] = face;
            }
        }
        f = face;
        return true;
    }
    Font *CacheSystem::load_font(const u8string &name,Uint32 idx){
        Font *f = query(name);
        if(f != nullptr and f->face->face_index == idx){
            f->ref();
            return f;
        }
        delete f;
        auto filename = FontUtils::GetFileByName(name);
        Face face;
        if(load_face(face,filename.c_str(),idx)){
            return new Font(face);
        }
        return nullptr;
    }
    Font *CacheSystem::query(const u8string &name){
        auto iter = faces.find(name);
        if(iter == faces.end()){
            return nullptr;
        }
        return new Font(iter->second);
    }
}
namespace Btk{
    void AddMemFont(u8string_view name,const void *buf,size_t bufsize,bool dup){
        BTK_ASSERT(buf != nullptr and bufsize > 0);
        Ft::FontBuffer fbuf(buf,bufsize,dup);
        Ft::GlobalCache().memfonts[u8string(name)] = fbuf;
    }
}