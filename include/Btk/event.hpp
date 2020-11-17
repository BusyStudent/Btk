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

                //This is all mouse event
                Enter = 2,//Mouse enter Widget 
                Leave = 3,//Mouse leave Widget
                Motion = 4,//Mouse motion in Widget
                Click = 5,//Mouse Click
                
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
        private:
            //Event type
            Type _type;
            bool _accepted;
        friend class Window;
    };
    class ResizeEvent:public Event{
        
    };
    /**
     * @brief A event about mouse
     * 
     */
    class MouseEvent:public Event{
        public:
            //Up or down
            enum MType{
                UP = 0,
                Up = 0,
                DOWN = 1,
                Down = 1,
            };
            //Mouse State
            enum MState{
                Pressed = 0,
                Released = 1
            };
        public:
            MouseEvent(int new_x,int new_y):
                Event(Event::Type::Motion){
                    this->new_x = new_x;
                    this->new_y = new_y;

                    mclicks = -1;
                    mstate = static_cast<MState>(-1);
                    mtype = static_cast<MType>(-1);
                };
            MouseEvent(Type type,int new_x,int new_y):
                Event(type){
                    this->new_x = new_x;
                    this->new_y = new_y;

                    mclicks = -1;
                    mstate = static_cast<MState>(-1);
                    mtype = static_cast<MType>(-1);
                };
            /**
             * @brief Construct a new Mouse Event object
             * 
             * @param event must be MouseButtonEvent
             */
            MouseEvent(const SDL_MouseButtonEvent &);
            ~MouseEvent();
            int x() const noexcept{
                return new_x;
            };
            int y() const noexcept{
                return new_y;
            };
            int clicks() const noexcept{
                return mclicks;
            };
            MState state() const noexcept{
                return mstate;
            };
            MType type() const noexcept{
                return mtype;
            };

            //Check it is pressed
            bool is_pressed() const noexcept{
                return mstate == Pressed;
            }
            //Check it is released
            bool is_released() const noexcept{
                return mstate == Released;
            };

            //Check is button up
            bool is_up() const noexcept{
                return mtype == Up;
            };
            //Check is button down
            bool is_down() const noexcept{
                return mtype == Down;
            };
        private:
            
            int mclicks;//Mouse Clicks count -1 means no clicks
            MState mstate;//Mouse state
            MType  mtype;//Mouse type
            
            int new_x;
            int new_y;
    };
    /**
     * @brief A event about keyboard
     * 
     */
    class KeyEvent:public Event{
        public:
            enum KState{
                //Pressed key
                Pressed = 0,
                //Released Key
                Released = 1,
            };
            enum KType{
                //KeyUp
                UP = 0,
                Up = 0,
                //KeyDown
                DOWN = 1,
                Down = 1,
            };
            /**
             * @brief Construct a new Key Event object
             * 
             * @note event.type must be SDL_KYDOWN or SDL_KEYUP
             * @param event a SDL_Event structure
             */
            KeyEvent(const SDL_Event &event);
            KeyEvent(const KeyEvent &ev)
                :Event(ev){
                
                kcode = ev.kcode;
                scode = ev.scode;
                kmode = ev.kmode;
                kstate = ev.kstate;
                ktype = ev.ktype;
                repeat = ev.repeat;
            };
            ~KeyEvent();
            //Get informations
            KType type() const noexcept{
                return ktype;
            };
            Keycode key() const noexcept{
                return kcode;
            };
            Scancode scancode() const noexcept{
                return scode;
            };
            Keymode keymode() const noexcept{
                return kmode;
            };
            KState state() const noexcept{
                return kstate;
            };
            bool is_repeat() const noexcept{
                return repeat;
            };
        private:
            //Keycode and scancode
            Keycode  kcode;
            Scancode scode;
            Keymode  kmode;
            //Event state
            KState kstate;
            KType  ktype;//Type of this Event
            bool repeat;//is repeat
    };
    /**
     * @brief A event about set Widget rect
     * 
     */
    class SetRectEvent:public Event{
        public:
            SetRectEvent(const Rect n):
                Event(Type::SetRect),
                new_rect(n){};
            SetRectEvent(const SetRectEvent &ev):
                Event(ev),
                new_rect(ev.new_rect){};
            ~SetRectEvent();
            /**
             * @brief Get new rect
             * 
             * @return the new rect
             */
            const Rect &rect() const noexcept{
                return new_rect;
            };
            Vec2 position() const noexcept{
                return {
                    new_rect.x,
                    new_rect.y
                };
            };
        private:
            Rect new_rect;
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
