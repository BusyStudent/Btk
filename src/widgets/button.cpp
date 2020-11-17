
#include "../build.hpp"
#include <Btk/button.hpp>
#include <Btk/event.hpp>
namespace Btk{
    bool AbstructButton::handle(Event &event){
        using Type = Event::Type;
        switch(event.type()){
            case Type::Enter:{
                is_entered = true;
                break;
            }
            case Type::Leave:{
                is_entered = false;
                break;
            }
            case Type::SetRect:{
                //SetPositions
                rect = static_cast<SetRectEvent&>(event).rect();
                break;
            }
            case Type::Click:{
                //Click button
                auto &ev = static_cast<MouseEvent&>(event);
                onclick(ev.x(),ev.y());
                break;
            }
            default:
                return false;
        }
        event.accept();
        return true;
    }
};