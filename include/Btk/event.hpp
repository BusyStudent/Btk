#if !defined(_BTK_EVENT_HPP_)
#define _BTK_EVENT_HPP_
#include <type_traits>
#include <cstdint>
#include "locals.hpp"
#include "string.hpp"
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
    class BTKAPI Event{
        public:
            enum Type:Uint32{
                None = 0,
                KeyBoard = 1,//Key pressed or released

                //MotionEvent
                Enter = 2,//Mouse enter Widget 
                Leave = 3,//Mouse leave Widget
                Motion = 4,//Mouse motion in Widget
                //MouseEvent
                Mouse = 5,//Mouse Click

                DragBegin = 6,//The drag is begin
                Drag = 7,//Is draging now
                DragEnd = 8,//The drag is end
                
                TextInput = 9,//Text the Input
                TextEditing = 10,


                TakeFocus = 11,//The widget take focus
                               //accept the event to task focus

                LostFocus = 12,//<The widget lost focus
                               //Of course you can refuse it

                Enable = 13,//The widget was enabled
                Disable = 14,//The widget was disabled
                
                Wheel = 15,//The mouse Wheel
                // WindowEnter = 16,//The mouse enter the window
                // WindowLeave = 17,//The mouse leave the window
                Resize = 18,//< The widget is be resized

                //Drop
                DropBegin = 19,
                DropEnd = 20,
                DropText = 21,
                DropFile = 22,
                //Hide / Show
                Hide = 23,
                Show = 24,
                //Layout
                LayoutUpdate,
                //SDL
                SDLWindow,//< SDL Window event,Internal use
                //Finger
                Figner,//< Finger click
                FignerMotion,//< Finger motion

                User = 10000,
                UserMax = UINT32_MAX - 1,
                Error = UINT32_MAX
            };
            /**
             * @brief Register a event type
             * @note This function is thread safe
             * @return Type::Error if failed
             */
            static Type Register();
            Type type() const noexcept{
                return _type;
            };
        public:
            Event(Type t):
                _type(t),
                _accepted(false),
                _broadcast(false){};
            Event(const Event &ev):
                _type(ev._type),
                _accepted(false),
                _broadcast(ev._broadcast){};
            ~Event() = default;
            /**
             * @brief is accepted
             * 
             * @return true 
             * @return false 
             */
            bool is_accepted() const noexcept{
                return _accepted;
            }
            bool is_rejected() const noexcept{
                return not _accepted;
            }
            bool is_broadcast() const noexcept{
                return _broadcast;
            }
            void set_broadcast(bool val = true) noexcept{
                _broadcast = val;
            }
            /**
             * @brief Accept the event
             * 
             * @return Always return true
             */
            bool accept() noexcept{
                _accepted = true;
                return true;
            }
            /**
             * @brief Reject the event
             * 
             * @return Always return false
             */
            bool reject() noexcept{
                _accepted = false;
                return false;
            }
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
            bool _broadcast;
        friend class Window;
    };
    /**
     * @brief A event about resize
     * 
     */
    struct BTKAPI ResizeEvent:public Event{
        public:
            ResizeEvent(int w,int h,bool sizing = false):Event(Resize){
                this->w = w;
                this->h = h;
                this->sizing = sizing;
            }
            ResizeEvent(const ResizeEvent &) = default;
            ~ResizeEvent() = default;
            /**
             * @brief Get the size
             * 
             * @return Size 
             */
            Size size() const noexcept{
                return {w,h};
            }
            /**
             * @brief Will we 
             * 
             * @return true 
             * @return false 
             */
            bool is_sizing() const noexcept{
                return sizing;
            }
            int w,h;
            bool sizing;
    };    
    /**
     * @brief A event about mouse click
     * 
     */
    struct BTKAPI MouseEvent:public Event{
        /**
         * @brief Construct a new Mouse Event object
         * 
         * @param event must be MouseButtonEvent
         */
        MouseEvent():Event(Event::Mouse){};
        MouseEvent(const MouseEvent &) = default;
        ~MouseEvent() = default;

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
            Pressed = 1,//< Button UP
            Released = 0,//< Button Down
        }state;
        /**
         * @brief Which button?
         * 
         */
        struct{
            Uint8 value;
            bool is_right() const noexcept{
                return value == SDL_BUTTON_RIGHT;
            }
            bool is_left() const noexcept{
                return value == SDL_BUTTON_LEFT;
            }
            bool is_middle() const noexcept{
                return value == SDL_BUTTON_MIDDLE;
            }
        }button;
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
    struct BTKAPI KeyEvent:public Event{
        /**
         * @brief Construct a new Key Event object
         * 
         * @note event.type must be SDL_KYDOWN or SDL_KEYUP
         * @param event a SDL_Event structure
         */
        KeyEvent():Event(Event::Type::KeyBoard){};
        KeyEvent(const KeyEvent &ev) = default;
        ~KeyEvent() = default;
        //Keycode and scancode
        Scancode scancode;
        Keycode  keycode;
        Keymode  keymode;
        enum{
            Pressed = 1,//< Button UP
            Released = 0,//< Button Down
        }state;
        bool repeat;//is repeat
        //Check kmode
        bool has_kmod(Keymode mode) const noexcept{
            return static_cast<bool>(keymode & mode);
        }
        bool is_pressed() const noexcept{
            return state == Pressed;
        }
        bool is_released() const noexcept{
            return state == Released;
        }
    };
    /**
     * @brief A event about mouse motion
     * 
     */
    struct BTKAPI MotionEvent:public Event{
        MotionEvent(Event::Type type = Event::Type::Motion):
            Event(type){};
        MotionEvent(const MotionEvent &) = default;
        ~MotionEvent() = default;

        //datas
        int x;
        int y;

        int xrel;
        int yrel;
        //methods
        Vec2 position() const noexcept{
            return {x,y};
        }
    };
    /**
     * @brief A Event of text input
     * 
     */
    struct BTKAPI TextInputEvent:public Event{
        TextInputEvent():Event(TextInput){};
        TextInputEvent(const TextInputEvent &) = default;
        ~TextInputEvent() = default;

        u8string_view text;//<Text input buffer(utf-8 encoded)
        /**
         * @brief Get length of the string
         * 
         * @return The utf8 string length
         */
        size_t length() const noexcept;
    };
    /**
     * @brief A Event of text editing
     * 
     */
    struct BTKAPI TextEditingEvent:public Event{
        TextEditingEvent():Event(TextEditing){};
        TextEditingEvent(const TextEditingEvent &) = default;
        ~TextEditingEvent() = default;

        u8string_view text;//<The text
        u8string_view editing;//< Selected text

        size_t selection_begin() const noexcept{
            return text.find(editing);
        }
        size_t selection_end() const noexcept{
            return text.find(editing) + editing.length();
        }
    };
    /**
     * @brief A event about mouse drag
     * 
     */
    struct BTKAPI DragEvent:public Event{
        /**
         * @brief Construct a new Drag Event object
         * 
         * @param type The DragEvent type(It can be Drag DragBegin DragEnd)
         * @param x The x which relative to window
         * @param y The y which relative to window
         * @param xrel The x relative to direction
         * @param yrel The y relative to direction
         */
        DragEvent(Event::Type type,int x,int y,int xrel,int yrel):
            Event(type),
            x(x),
            y(y),
            xrel(xrel),
            yrel(yrel){};
        /**
         * @brief Construct a new Drag Event object from MotionEvent
         * 
         * @param type The DragEvent type(It can be Drag DragBegin DragEnd)
         * @param motion The motion event which you want to get its motion data
         */
        DragEvent(Event::Type type,const MotionEvent &motion):Event(type){
            x = motion.x;
            y = motion.y;
            xrel = motion.xrel;
            yrel = motion.yrel;
        };
        DragEvent(const DragEvent &) = default;
        ~DragEvent() = default;

        Vec2 position() const noexcept{
            return {x,y};
        }

        int x;
        int y;
        int xrel;//< Note It cannot be used at DragEnd
        int yrel;//< Note It cannot be used at DragEnd
    };
    struct BTKAPI WheelEvent:public Event{
        WheelEvent(Uint32 which,Sint64 x,Sint64 y):Event(Type::Wheel){
            this->which = which;
            this->x = x;
            this->y = y;
        }
        WheelEvent(const WheelEvent &) = default;
        ~WheelEvent() = default;

        Uint32 which;//< Which mouse
        Sint64 x;//< Vertical scroll,postive for scroll right
        Sint64 y;//< Horizontal scroll,postive for scroll up
    };
    /**
     * @brief A event of drop
     * 
     */
    struct BTKAPI DropEvent:public Event{
        DropEvent(Type t):Event(t){};
        DropEvent(const DropEvent &) = default;
        ~DropEvent() = default;

        u8string_view text;

        int x = -1,y = -1;
        Vec2 position() const noexcept{
            return {x,y};
        }
    };
    /**
     * @brief Wrap SDL_Event pointer to it
     * 
     */
    struct BTKAPI SDLEvent:public Event{
        SDLEvent(Type t,const SDL_Event *ptr):Event(t),sdl_event(ptr){};
        SDLEvent(const SDLEvent &) = default;
        ~SDLEvent() = default;

        const SDL_Event *sdl_event;
    };
    /**
     * @brief Push event to queue
     * 
     * @param event The event pointer we want to send
     * @param receiver The receiver
     * @param deleter The deleter(can be nullptr on default delete)
     */
    void PushEvent(Event *event,Window &receiver,void (*deleter)(void *));
    void PushEvent(Event *event,Widget &receiver,void (*deleter)(void *));

    //template for it
    template<
        class T,
        class Receiver
    >
    void PushEvent(T *event,Receiver &receiver){
        if constexpr(std::is_trivially_destructible_v<std::decay_t<T>>){
            PushEvent(std::forward<T>(event),std::forward<Receiver>(receiver),nullptr);
        }
        else{
            PushEvent(std::forward<T>(event),std::forward<Receiver>(receiver),[](void *event){
                delete static_cast<T*>(event);
            });
        }
    }
    template<
        class T,
        class Receiver
    >
    void PushEvent(const T &event,Receiver &receiver){
        return PushEvent(new T(event),receiver);
    }
    /**
     * @brief Dispatched event right now
     * 
     * @param receiver 
     * @return true - If the receiver processed it
     * @return false - If the receiver unprocessed it
     */
    bool SendEvent(Event &event,Window &receiver);
    bool SendEvent(Event &event,Widget &receiver);
};


#endif // _BTK_EVENT_HPP_
