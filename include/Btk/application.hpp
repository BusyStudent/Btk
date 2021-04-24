#if !defined(_BTK_APPLICATION_HPP_)
#define _BTK_APPLICATION_HPP_

#include "defs.hpp"
#include "object.hpp"
namespace Btk{
    class BTKAPI Application:public HasSlots{
        public:
            Application();
            Application(const Application &) = delete;
            ~Application();
        private:
            
    };
}

#endif // _BTK_APPLICATION_HPP_
