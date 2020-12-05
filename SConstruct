#Scons build file

env = Environment(CPPFLAGS  = ["-O3","-std=c++17"])

env.Append(CPPDEFINES='BTK_USE_GFX')
env.Append(CPPDEFINES='NDEBUG')

libs = ['SDL2','SDL2_image','SDL2_ttf']

src = Glob("./src/*.cpp")

src.extend(Glob("./src/thirdparty/*.c"))
#Impl files
src.extend(Glob("./src/impl/*.cpp"))
src.extend(Glob("./src/themes/*.cpp"))
src.extend(Glob("./src/thread/*.cpp"))

src.extend(Glob("./src/widgets/*.cpp"))
src.extend(Glob("./src/platform/*.cpp"))

#Check platform
if env['PLATFORM'] == 'posix':
    src.extend(Glob("./src/platform/x11/*.cpp"))
    libs.append('fontconfig')
elif env['PLATFORM'] == 'win32':
    src.extend(Glob("./src/platform/win32/*.cpp"))


env.SharedLibrary("btk",src,CPPPATH = ['./include'],LIBS = libs)