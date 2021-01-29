#include "../build.hpp"

#include <Btk/impl/scope.hpp>
#include <Btk/impl/mixer.hpp>
namespace Btk{
namespace Mixer{
    using Impl::ScopePtr;
    struct WAVMusic:public MusicImpl{
        SDL_AudioSpec *wav_spec;
    };
    MusicImpl *LoadWAV(SDL_RWops *rwops){
        ScopePtr ptr(new WAVMusic);

        if(SDL_LoadWAV_RW(rwops,0,&ptr->spec,nullptr,nullptr)){

        }


        ptr.release();
        
        return ptr.get();
    }
}
}