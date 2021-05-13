#include "../../build.hpp"

#include <Btk/platform/dbus.hpp>
#include <Btk/application.hpp>
#include <Btk/exception.hpp>
//App implment
namespace Btk{
    struct BTKHIDDEN ApplicationImpl{
        ApplicationImpl();
        ApplicationImpl(const ApplicationImpl &) = delete;
        ~ApplicationImpl();

        bool set_name(u8string_view name);

        DBus::Connection con;
        DBus::Error err;
    };
    ApplicationImpl::ApplicationImpl(){
        dbus_error_init(&err);
        
        con = DBus::GetBus(DBUS_BUS_SESSION,&err);
        if(err.is_set()){
            //Has error
            throwRuntimeError(err.message);
        }
    }
    ApplicationImpl::~ApplicationImpl(){
        //dbus_connection_close(con);
    }
}
namespace Btk{
    Application::Application(){
        app = new ApplicationImpl;
        
    }
    Application::~Application(){
        delete app;
    }
    bool Application::notify(u8string_view title,u8string_view body){
        if(app == nullptr){
            return false;
        }
        auto &con = app->con;
        auto *err = &(app->err);

        DBusMessage *msg = dbus_message_new_method_call(
            nullptr,
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            "Notify"
        );
        if(msg == nullptr){
            return false;
        }
        dbus_message_set_auto_start(msg,TRUE);
        dbus_message_set_destination(msg,"org.freedesktop.Notifications");
        dbus_message_set_serial(msg,3);
        //Init message
        DBusMessageIter args;
        dbus_message_iter_init_append(msg,&args);
        //Fill it
        /*Notify Method args
            (
            String app_name, 
            UInt32 replaces_id, 
            String app_icon, 
            String summary, 
            String body, 
            Array of [String] actions, 
            Dict of {String, Variant} hints,
            Int32 timeout
            )
            return (UInt32 arg_0)
        
        */
        Uint32 id = 0;
        Int32 timeout = 0;
        const char *empty_str = "";
        
        dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&empty_str);
        dbus_message_iter_append_basic(&args,DBUS_TYPE_UINT32,&id);
        dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&empty_str);
        
        u8string t(title);
        u8string b(body);
        const char *t_str = t.c_str();
        const char *b_str = b.c_str();
        dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&t_str);
        dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&b_str);
        
        DBusMessageIter sub;
        
        dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,DBUS_TYPE_STRING_AS_STRING,&sub);
        dbus_message_iter_close_container(&args,&sub);

        dbus_message_iter_open_container(&args,DBUS_TYPE_ARRAY,DBUS_TYPE_STRING_AS_STRING,&sub);
        dbus_message_iter_close_container(&args,&sub);


        dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&timeout);
        
        dbus_message_iter_init_closed(&args);
        //End


        //dbus_connection_send(con,msg,nullptr);
        //dbus_connection_flush(con);
        con.flush();
        dbus_message_unref(msg);
        return true;
    }
}