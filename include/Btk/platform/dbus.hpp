#if !defined(_BTK_PLATFORM_DBUS_HPP_)
#define _BTK_PLATFORM_DBUS_HPP_
#include <dbus/dbus.h>
#include <chrono>
#include <initializer_list>
#include "../exception.hpp"
#include "../string.hpp"


namespace Btk::DBus{
    struct MessageIter;
    /**
     * @brief DBusMessage Wrapper
     * 
     */
    struct Message{
        /**
         * @brief Construct a new empty Message object
         * 
         */
        Message():msg(nullptr){};
        Message(const Message &);
        Message(Message &&);
        ~Message();
        void unref();
        MessageIter add();

        DBusMessage *msg;
    };
    struct MethodCall:public Message{
        template<class ...Args>
        void set_args(Args &&...args);
    };
    struct MessageIter:public DBusMessageIter{
        MessageIter &operator <<(Uint32);
        MessageIter &operator <<(Sint32);
        MessageIter &operator <<(const char *);
        MessageIter &operator <<(const u8string &);

        void close();
    };
    /**
     * @brief DBusConnection Wrapper
     * 
     */
    struct Connection{
        /**
         * @brief Construct a new empty Connection object
         * 
         */
        Connection():con(nullptr){};
        Connection(const Connection &) = default;
        Connection(Connection &&);
        ~Connection();
        void unref();
        void flush();
        bool send(Message &msg,dbus_uint32_t *client_serial = nullptr);
        Connection &operator =(Connection &&);
        
        DBusConnection *con;
    };
    /**
     * @brief DBus Error
     * 
     */
    struct Error:public DBusError{
        Error();
        Error(Error &&);
        ~Error();

        bool is_set() const noexcept;
        operator bool() const noexcept{
            return is_set();
        }
    };
    Connection GetBus(DBusBusType t,DBusError *err = nullptr);
    MethodCall NewMethodCall(const char *bus_name,
                          const char *path,
                          const char *interface,
                          const char *method);
    Message NewMethodReturn();
}
//Impl begin
namespace Btk::DBus{
    //Connection
    inline Connection::Connection(Connection &&c){
        con = c.con;
        c.con = nullptr;
    }
    inline Connection::~Connection(){
        unref();
    }
    inline void Connection::unref(){
        if(con != nullptr){
            dbus_connection_unref(con);
            con = nullptr;
        }
    }
    inline void Connection::flush(){
        dbus_connection_flush(con);
    }
    inline bool Connection::send(Message &msg,dbus_uint32_t *client_serial){
        return dbus_connection_send(con,msg.msg,client_serial);
    }
    inline Connection &Connection::operator =(Connection &&c){
        unref();
        con = c.con;
        c.con = nullptr;
        return *this;
    }
    //Error
    inline Error::Error(){
        dbus_error_init(this);
    }
    inline Error::Error(Error &&err){
        dbus_error_init(this);
        dbus_move_error(&err,this);
    }
    inline Error::~Error(){
        dbus_error_free(this);
    }
    inline bool Error::is_set() const noexcept{
        return dbus_error_is_set(this);
    }
    //Message
    inline Message::~Message(){
        unref();
    }
    inline Message::Message(const Message &m){
        msg = dbus_message_copy(m.msg);
    }
    inline Message::Message(Message &&m){
        msg = m.msg;
        m.msg = nullptr;
    }
    inline void Message::unref(){
        if(msg != nullptr){
            dbus_message_unref(msg);
            msg = nullptr;
        }
    }
    //GetBus
    inline Connection GetBus(DBusBusType t,DBusError *err){
        auto *con = dbus_bus_get(t,err);
        Connection c;
        c.con = con;
        return c;
    }
    //Message
    inline MethodCall NewMethodCall(
                          const char *bus_name,
                          const char *path,
                          const char *interface,
                          const char *method){
        auto *msg = dbus_message_new_method_call(
            bus_name,
            path,
            interface,
            method
        );
        MethodCall m;
        m.msg = msg;
        return m;
    }
    inline MessageIter Message::add(){
        MessageIter iter;
        dbus_message_iter_init_append(msg,&iter);
        return iter;
    }
    //Iter
    inline MessageIter &MessageIter::operator <<(Uint32 v){
        dbus_message_iter_append_basic(this,DBUS_TYPE_UINT32,&v);
        return *this;
    }
    inline MessageIter &MessageIter::operator <<(Sint32 v){
        dbus_message_iter_append_basic(this,DBUS_TYPE_INT32,&v);
        return *this;
    }
    inline MessageIter &MessageIter::operator <<(const char *s){
        dbus_message_iter_append_basic(this,DBUS_TYPE_STRING,&s);
        return *this;
    }
    inline MessageIter &MessageIter::operator <<(const u8string &s){
        return *this << s.c_str();
    }
    inline void MessageIter::close(){
        dbus_message_iter_init_closed(this);
    }
}
#endif // _BTK_PLATFORM_DBUS_HPP_
