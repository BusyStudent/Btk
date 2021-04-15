#include "../build.hpp"
#include "internal.hpp"

#include <Btk/font.hpp>
#include <Btk/Btk.hpp>

namespace BtkFt{
    Library *library = nullptr;
    void Init(){
        using namespace Btk::FontUtils;
        if(library == nullptr){
            library = new Library;
            Instance().add_font(GetDefaultFont().c_str(),0);
            Btk::AtExit(BtkFt::Quit);
        }
        
    }
    void Quit(){
        delete library;
        library = nullptr;
    }
    Face *Library::find_font(std::string_view name){
        std::lock_guard locker(mtx);
        std::map<std::string,Face*>::iterator iter;
        if(name == ""){
            iter = faces_map.begin();
        }
        else{
            iter = faces_map.find(std::string(name));
        }
        if(iter == faces_map.end()){
            return nullptr;
        }
        return iter->second;
    }
    Face *Library::add_font(const char *filename,FaceIndex index){
        assert(filename);
        std::lock_guard locker(mtx);
        auto face = new Face(filename,index);
        
        faces_map[face->name] = face;

        return face;
    }
    void Face::unref(){
        std::lock_guard locker(*this);
        --refcount;
        if(refcount <= 0){
            Instance().faces_map.erase(name);
            delete this;
        }
    }
}