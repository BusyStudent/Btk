#if !defined(_BTK_EVENT_HPP_)
#define _BTK_EVENT_HPP_
#include <cstdint>
#include <variant>
#include "defs.hpp"
union SDL_Event;
namespace Btk{
    class Window;
    class Widget;
    /**
     * @brief A base event of all events
     * 
     */
    class Event{
        public:
            enum Type:Uint32{
                WidgetUpdate = 0,//Update widget's rect
                USER = 1000,
                USER_MAX = UINT32_MAX - 1,
                ERROR = UINT32_MAX
            };
            /**
             * @brief Register a event type
             * 
             * @return Type::ERROR if failed
             */
            static Type Register();
        public:
            Event(Type t):type(t){};
            virtual ~Event();
        private:
            Type type;
        friend class Window;
    };
    class MouseMotionEvent:public Event{
        //...
    };
    class ResizeEvent:public Event{
        
    };
    class WidgetUpdateEvent:public Event{

    };
    /**
     * @brief Push event to queue
     * 
     * @param event The event pointer we want to send
     * @param receiver The receiver
     */
    void PushEvent(Event *event,Window &receiver);
    void PushEvent(Event *event,Widget &receiver);
    /**
     * @brief Dispatched right now
     * 
     * @param receiver 
     * @return true - If the receiver accepted it
     * @return false - If the receiver rejected it
     */
    bool SendEvent(Event &event,Window &receiver);
    bool SendEvent(Event &event,Widget &receiver);
    /**
     * @brief This function was called by System to dispatch our event
     * 
     * @param ev SDL_Event structure 
     */
    void DispatchEvent(const SDL_Event &ev,void *);
};


#endif // _BTK_EVENT_HPP_
