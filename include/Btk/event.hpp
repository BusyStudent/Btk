#if !defined(_BTK_EVENT_HPP_)
#define _BTK_EVENT_HPP_
#include <cstdint>
#include <variant>
#include "locals.hpp"
#include "defs.hpp"
#include "rect.hpp"
union  SDL_Event;
struct SDL_MouseButtonEvent;
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
                SetRect = 0,//Update widget's rect
                KeyBoard = 1,//Key pressed or released

                //MotionEvent
                Enter = 2,//Mouse enter Widget 
                Leave = 3,//Mouse leave Widget
                Motion = 4,//Mouse motion in Widget
                //MouseEvent
                Click = 5,//Mouse Click

                DragBegin = 6,//The drag is begin
                Drag = 7,//Is draging now
                DragEnd = 8,//The drag is end
                
                USER = 1000,
                USER_MAX = UINT32_MAX - 1,
                ERROR = UINT32_MAX
            };
            /**
             * @brief Register a event type
             * @note This function is thread safe
             * @return Type::ERROR if failed
             */
            static Type Register();
            Type type() const noexcept{
                return _type;
            };
        public:
            Event(Type t):
                _type(t),
                _accepted(false){};
            Event(const Event &ev):
                _type(ev._type),
                _accepted(false){};
            virtual ~Event();
            /**
             * @brief is accepted
             * 
             * @return true 
             * @return false 
             */
            bool is_accepted() const noexcept{
                return _accepted;
            };
            void accept() noexcept{
                _accepted = true;
            };
            void reject() noexcept{
                _accepted = false;
            };
            /**
             * @brief Chaneg the event type
             * 
             * @warning It is very dangerous to use
             * @param t The new type
             */
            void set_type(Type t) noexcept{
                _type = t;
            };
        private:
            //Event type
            Type _type;
            bool _accepted;
        friend class Window;
    };
    class ResizeEvent:public Event{
        
    };    
    /**
     * @brief A event about mouse click
     * 
     */
    struct MouseEvent:public Event{
        /**
         * @brief Construct a new Mouse Event object
         * 
         * @param event must be MouseButtonEvent
         */
        MouseEvent():Event(Event::Type::Click){};
        MouseEvent(const MouseEvent &) = default;
        ~MouseEvent();

        bool is_pressed() const noexcept{
            return state == Pressed;
        };
        bool is_released() const noexcept{
            return state == Released;
        };
        bool is_up() const noexcept{
            return state == Pressed;
        };
        bool is_down() const noexcept{
            return state == Released;
        };
        Vec2 position() const noexcept{
            return {x,y};
        };
        /**
         * @brief Mouse state
         * 
         */
        enum{
            Pressed,//< Button UP
            Released//< Button Down
        }state;
        /**
         * @brief Clicks count
         * 
         */
        Uint8 clicks;
        /**
         * @brief Mouse position
         * 
         */
        int x;
        int y;
    };
    /**
     * @brief A event about keyboard
     * 
     */
    struct KeyEvent:public Event{
        /**
         * @brief Construct a new Key Event object
         * 
         * @note event.type must be SDL_KYDOWN or SDL_KEYUP
         * @param event a SDL_Event structure
         */
        KeyEvent():Event(Event::Type::KeyBoard){};
        KeyEvent(const KeyEvent &ev) = default;
        ~KeyEvent();
        //Keycode and scancode
        Scancode scancode;
        Keycode  keycode;
        Keymode  keymode;
        enum{
            Pressed,
            Released
        }state;
        bool repeat;//is repeat
    };
    /**
     * @brief A event about set Widget rect
     * 
     */
    struct SetRectEvent:public Event{
        SetRectEvent(const Rect n):
            Event(Type::SetRect),
            rect(n){};
        SetRectEvent(const SetRectEvent &ev) = default;
        ~SetRectEvent();

        Vec2 position() const noexcept{
            return {rect.x,rect.y};
        };

        Rect rect;
    };
    /**
     * @brief A event about mouse drag
     * 
     */
    struct DragEvent:public Event{
        DragEvent();
        DragEvent(const DragEvent &) = default;
        ~DragEvent();

        int x;
        int y;
        int refx;
        int refy;
    };
    /**
     * @brief A event about mouse motion
     * 
     */
    struct MotionEvent:public Event{
        MotionEvent(Event::Type type = Event::Type::Motion):
            Event(type){};
        MotionEvent(const MotionEvent &) = default;
        ~MotionEvent();

        //datas
        int x;
        int y;

        int xrel;
        int yrel;
        //methods
        Vec2 position() const noexcept{
            return {x,y};
        };
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
     * @brief Dispatched event right now
     * 
     * @param receiver 
     * @return true - If the receiver processed it
     * @return false - If the receiver unprocessed it
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
