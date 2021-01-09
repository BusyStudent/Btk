#if !defined(_BTK_MIXER_HPP_)
#define _BTK_MIXER_HPP_
#include <string_view>
namespace Btk{
namespace Mixer{
    class Music{
        public:
            static Music FromFile();
    };
    class Channal{
        
    };
    class Audio{
        
    };
}
using MixerMusic = Mixer::Music;
using MixerChannal = Mixer::Channal;
}

#endif // _BTK_MIXER_HPP_
