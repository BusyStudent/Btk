from PyBtk import Window
win = Window("Hello World",100,100)

def on_resize(w,h):
    win.title = "%d %d" % (w,h)
win.on_resize = on_resize
win.set_resizeable(True)
win.mainloop()