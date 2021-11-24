#include <Python.h>
#include <Btk.hpp>

#include <exception>
#include <functional>

#include "internal.hpp"

#include <pybind11/functional.h>

//Init /  Entry
namespace PyBtk{

struct PyWindow:public Btk::Window{
    template<class ...Args>
    PyWindow(Args &&...args):
        Btk::Window(std::forward<Args>(args)...)
        {
            signal_close().connect(&PyWindow::on_close,this);
            signal_resize().connect(&PyWindow::on_resize,this);
            signal_dropfile().connect(&PyWindow::on_dropfile,this);
        }
    ~PyWindow(){
        disconnect_all();
    }

    std::function<bool()> on_close_fn;
    std::function<void(int,int)> on_resize_fn;
    std::function<void(std::string_view )> on_dropfile_fn;

    bool on_close(){
        if(on_close_fn != nullptr){
            return on_close_fn();
        }
        return true;
    }
    void on_dropfile(Btk::u8string_view view){
        if(on_dropfile_fn != nullptr){
            on_dropfile_fn(view.base());
        }
    }
    void on_resize(int w,int h){
        if(on_resize_fn != nullptr){
            on_resize_fn(w,h);
        }
    }
};

PYBIND11_MODULE(PyBtk,mod){
    mod.doc() = "Btk Python interface";
    mod.def("Init",Btk::Init);
    mod.def("Exit",Btk::Exit);
    mod.def("run" ,Btk::run);

    mod.attr("EXIT_SUCCESS") = EXIT_SUCCESS;
    mod.attr("EXIT_FAILURE") = EXIT_FAILURE;

    mod.def_submodule("detail").
        attr("version") = "0.0.1"
    ;

    //Def window
    py::class_<PyWindow>(mod,"Window",py::dynamic_attr())
        .def(py::init<const std::string&,int,int>())
        .def(py::init<const std::string&,int,int,Btk::WindowFlags>())
        .def("mainloop",[](PyWindow &window){
            if(not window.exists()){
                //Check exists
                Btk::throwRuntimeError("Bad window");
            }
            return window.mainloop();
        })
        .def("exists",&PyWindow::exists)
        .def("resize",&PyWindow::resize)
        .def("show",&PyWindow::show)
        .def("move",&PyWindow::move)
        .def("draw",&PyWindow::draw)
        .def("close",&PyWindow::close)
        .def("set_fullscreen",&PyWindow::set_fullscreen)
        .def("set_resizeable",&PyWindow::set_resizeable)
        .def("set_boardered",&PyWindow::set_boardered)
        //Query
        .def("position",[](PyWindow &win){
            auto [x,y] = win.position();
            return std::make_tuple(x,y);
        })
        .def("internal_data",&PyWindow::internal_data)
        //Attr
        .def_property("w",
            &PyWindow::w,
            [](PyWindow &window,int new_w){
                window.resize(new_w,window.h());
            })
        .def_property("h",
            &PyWindow::h,
            [](PyWindow &window,int new_h){
                window.resize(window.w(),new_h);
            })
        .def_property("title",
            &PyWindow::title,
            &PyWindow::set_title
            )
        .def_readwrite("on_close",&PyWindow::on_close_fn)
        .def_readwrite("on_resize",&PyWindow::on_resize_fn)
        .def_readwrite("on_dropfile",&PyWindow::on_dropfile_fn)
        //repr
        .def("__repr__",[](PyWindow &window) -> Btk::u8string{
            if(not window.exists()){
                return "<PyBtk.Window destroyed=True>";
            }
            return Btk::u8format(
                "<PyBtk.Window(%s,%d,%d)>",
                window.title().data(),
                window.w(),
                window.h()
            );
        })
        
    ;
    //Def enum
    py::enum_<Btk::Orientation>(mod,"Orientation")
        .value("H",Btk::Orientation::H)
        .value("V",Btk::Orientation::V)
        .value("Horizontal",Btk::Orientation::Horizontal)
        .value("Vertical",Btk::Orientation::Vertical)
    ;
    py::enum_<Btk::WindowFlags>(mod,"WindowFlags")
        .value("Default",Btk::WindowFlags::None)
        .value("OpenGL",Btk::WindowFlags::OpenGL)
        .value("Vulkan",Btk::WindowFlags::Vulkan)
        .value("SkipTaskBar",Btk::WindowFlags::SkipTaskBar)
    ;
    
}

}
