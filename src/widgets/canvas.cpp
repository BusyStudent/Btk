#include "../build.hpp"

#include <Btk/canvas.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>
namespace Btk{
    Canvas::Canvas() = default;
    Canvas::Canvas(int x,int y,int w,int h){
        rect = {x,y,w,h};
    }
    Canvas::~Canvas() = default;
    void Canvas::draw(Renderer &renderer,Uint32){
        if(not draw_fn.empty()){
            // auto viewport = renderer.get_viewport();
            // renderer.set_viewport(rect);
            if(protect_context){
                renderer.save();
            }
            draw_fn(renderer);
            if(protect_context){
                renderer.restore();
            }
            // renderer.set_viewport(viewport);
        }
    }
    bool Canvas::handle(Event &event){
        if(event_fn.empty()){
            return false;
        }
        return event_fn(event);
    }
    #ifndef BTK_DISABLE_SHAPE_API
    //ShapeNode
    ShapeNode::ShapeNode(){
        #ifndef NDEBUG
        _signal_enter.connect([this](){
            BTK_LOGINFO("[%s] Mouse Enter",BTK_typenameof(this));
        });
        _signal_leave.connect([this](){
            BTK_LOGINFO("[%s] Mouse Leave",BTK_typenameof(this));
        });
        _signal_clicked.connect([this](){
            BTK_LOGINFO("[%s] Mouse Clicked",BTK_typenameof(this));
        });
        _signal_collided.connect([this](){
            BTK_LOGINFO("[%s] Coliided",BTK_typenameof(this));
        });
        #endif
    }
    ShapeNode::~ShapeNode(){}

    FRect ShapeNode::bounding_box() const{
        return rectangle<float>();
    }
    FPolygen ShapeNode::polygen() const{
        FPolygen poly;
        //Rect to polygen
        FRect bounds = bounding_box();
        poly.push_back(FPoint(bounds.x,bounds.y));
        poly.push_back(FPoint(bounds.x+bounds.w,bounds.y));
        poly.push_back(FPoint(bounds.x+bounds.w,bounds.y+bounds.h));
        poly.push_back(FPoint(bounds.x,bounds.y+bounds.h));

        return poly;
    }
    bool ShapeNode::handle(Event &event){
        if(Widget::handle(event)){
            return true;
        }
        if(event.type() == Event::Leave){
            notify_mouse_leave();
            return true;
        }
        return false;
    }
    bool ShapeNode::handle_drag(DragEvent &event){
        if(not _dragable){
            return false;
        }
        switch(event.type()){
            case Event::Drag:
                move(x() + event.xrel,y() + event.yrel);
                redraw();
        }
        return event.accept();
    }
    bool ShapeNode::handle_mouse(MouseEvent &event){
        if(event.is_pressed() and bounding_box().has_point(event.position())){
            _signal_clicked.defer_emit();
            return event.accept();
        }
        return event.reject();
    }
    //Rect Node
    ShapeRectNode::ShapeRectNode() = default;
    ShapeRectNode::~ShapeRectNode() = default;
    bool ShapeRectNode::handle(Event &event){
        if(ShapeNode::handle(event)){
            return true;
        }
        if(event.type() == Event::Enter){
            notify_mouse_enter();
            return true;
        }
        return false;
    }
    void ShapeRectNode::draw(Renderer &r,Uint32){
        if(fill){
            r.draw_box(rectangle(),c);
        }
        else{
            r.draw_rect(rectangle(),c);
        }
    }
    //LineNode
    ShapeLineNode::ShapeLineNode() = default;
    ShapeLineNode::~ShapeLineNode() = default;

    void ShapeLineNode::draw(Renderer &r,Uint32){
        r.save();
        r.begin_path();
        r.stroke_width(width);
        r.stroke_color(c);
        r.move_to(line.p1());
        r.line_to(line.p2());
        r.stroke();
        r.restore();
    }
    bool ShapeLineNode::handle_motion(MotionEvent &event){
        //Check point in line
        auto point = event.position().cast<float>();
        if(PointInLine(line,point)){
            if(not in_line){
                //Has not enter before
                in_line = true;
                notify_mouse_enter();
            }
            return true;
        }
        else{
            //Leave
            if(in_line){
                in_line = false;
                notify_mouse_leave();
            }
            return false;//< Let Dispather to find another widget has this p
        }
    }
    bool ShapeLineNode::handle_mouse(MouseEvent &event){
        //Check point in line
        auto point = event.position().cast<float>();
        if(event.is_pressed() and PointInLine(line,point)){
            signal_clicked().defer_emit();
            return event.accept();
        }
        return event.reject();
    }
    FRect ShapeLineNode::bounding_box() const{
        //Calc the line bound
        float x = min(line.x1,line.x2);
        float y = min(line.y1,line.y2);
        float w = std::abs(line.x1 - line.x2);
        float h = std::abs(line.y1 - line.y2);
        return FRect(x,y,w,h);
    }
    #endif
}