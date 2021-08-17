# Btk

A library for developing Gui based on SDL2 and nanovg  
***Still developing***  

## CI

![C/C++ CI on Linux](https://github.com/BusyStudent/Btk/workflows/C/C++%20CI%20on%20Linux/badge.svg)

[![C/C++ CI on Windows](https://github.com/BusyStudent/Btk/actions/workflows/windows.yml/badge.svg)](https://github.com/BusyStudent/Btk/actions/workflows/windows.yml)

----

## Tiny example  

```cpp
#include <Btk.hpp>
using Btk::Window;
using Btk::Button;
struct App:public Window{
    App():Window("Hello World",100,100){
        //Add a button and connect signal
        auto &btn = add<Button>("Close the window");
        btn.signal_clicked().connect([this](){
            close();
        });
    }
};
int main(){
    App app;
    app.mainloop();
}
```

----

## TODO List  

- Rewrite font system to get better performence  
- Add more useful widgets  
- Add Btk::Bind(...) to bind object and functions  
- Rewrite ImageView By OpenGL to implement AntiAlias

----

## Widgets List

|  Widgets  |  Done?  | TODO | Description |
|  ---      |  ---    | ---  | ---         |
|  Button   |         |      |             |
|  ImageView|         |      |             |
|  TextBox  |         |      |             |
|  ProgressBar  |         |      |             |
|  EmbedWindow  | No      |      |             |
|  ScrollBar|        |      |             |
|  Canvas   |         |      |             |
|  GLCanvas |         |      |             |
|  GLWidget | No      |      |             |
|  Label    |         |      |             |
|  Line     |         |      |             |

----

## Renderer Backend

| Backend    | Done? | Description|
| ---        | ---   | ---        |
| OpenGLESv3 |       |            |
| Direct3D11 |       |            |
| SDLRender  | No    |            |

----

## Building prerequisites

### Linux

- G++ with C++17 support  
- SDL2  
- SDL2-image  (optional)  
- fontconfig (optional,for select font)  
- libgif (optional,for display gif)  
- scons or xmake

### Windows

- MINGW or vs2017  
- SDL2  
- SDL2-image (optional)  
- libgif (optional)  
- scons or xmake

### Android

- Still developing

----

## Build

```console
btk@btk:sudo apt install libsdl2-dev
btk@btk:scons
```

or

```console
xmake
```

## License

MIT
