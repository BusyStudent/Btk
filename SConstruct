#Scons build file

env = Environment(CXXFLAGS = "-std=c++17")

# env.Append(CPPDEFINES ='BTK_USE_GFX')

libs = ['SDL2','SDL2_image','SDL2_ttf']

src = Glob("./src/*.cpp")

src.extend(Glob("./src/thirdparty/*.c"))
#Impl files
src.extend(Glob("./src/detail/*.cpp"))
src.extend(Glob("./src/themes/*.cpp"))
src.extend(Glob("./src/utils/*.cpp"))
src.extend(Glob("./src/widgets/*.cpp"))
src.extend(Glob("./src/images/stb_image.cpp"))
src.extend(Glob("./src/images/adapter.cpp"))
src.extend(Glob("./src/font/*.cpp"))
src.extend(Glob("./src/gl/*.cpp"))
src.extend(Glob("./src/platform/*.cpp"))

#Check platform
if env['PLATFORM'] == 'posix':
    src.extend(Glob("./src/platform/x11/*.cpp"))
    libs.append('fontconfig')
elif env['PLATFORM'] == 'win32' or env['PLATFORM'] == 'msys':
    src.extend(Glob("./src/platform/win32/*.cpp"))
    libs.append('Gdi32')
#Check debug mode
debug = ARGUMENTS.get('debug', 0)
if int(debug):
    #Is debugging
    env.Append(CFLAGS = '-g')
    env.Append(CXXFLAGS = ' -g')
else:
    #No debug
    env.Append(CPPDEFINES='NDEBUG')
    env.Append(CXXFLAGS = ' -O3')
    env.Append(CFLAGS = ["-O3"])


env.SharedLibrary("btk",src,CPPPATH = ['./include'],LIBS = libs)