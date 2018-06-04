TEMPLATE = app
CONFIG -= qt
CONFIG += link_pkgconfig
TARGET = remote_imgui_example
DESTDIR = ./
#CONFIG += link_pkgconfig
#PKGCONFIG += ogg dbus-1
PKGCONFIG += liblz4

# START USER EDITABLE AREA -----------------------------------------------------------------------

# When commented out, main.cpp is built, otherwise main2.cpp is built
#CONFIG+= use_main2
#CONFIG+= use_main3  # dev only
#CONFIG+= use_main4  # dev only
#CONFIG+= use_main5  # dev only
#CONFIG+= use_main6  # dev only

# Only one of these must be active:
CONFIG+= use_glfw3
#CONFIG+= use_glut
#CONFIG+= use_sdl2
#CONFIG+= use_winapi
#CONFIG+= use_direct3d9
# Optional, but it might be mandatory for Windows: only ONE of them is allowed
#CONFIG+= use_glew
#CONFIG+= use_glad	# experimental
#CONFIG+= use_gl3w	# experimental
#------------------------------------------------------------------------------

IMGUI_BASE_PATH=../imgui
WEBBY_BASE_PATH=../webby

#DEFINES+= IMGUI_INCLUDE_IMGUI_USER_H IMGUI_INCLUDE_IMGUI_USER_INL # mandatory for loading addons

INCLUDEPATH+= 	$$IMGUI_BASE_PATH				\
		$$STB_BASE_PATH

#		$$LZ4_BASE_PATH				\
#		$$WEBBY_BASE_PATH				\

HEADERS+=  $$IMGUI_BASE_PATH"/imgui.h"						    \
	   $$IMGUI_BASE_PATH"/imconfig.h"						    \
	   "imgui_impl_glfw_gl2.h"	    \
	   "../imgui_remote.h" \
	   "../imgui_remote_webby.h" \
	   $$WEBBY_BASE_PATH"/webby.h"						    

SOURCES+=  $$IMGUI_BASE_PATH"/imgui.cpp" \
	   $$IMGUI_BASE_PATH"/imgui_draw.cpp" \
	   $$IMGUI_BASE_PATH"/imgui_demo.cpp" \ #\ # optional: for ImGui::ShowTestWindow()
	   $$WEBBY_BASE_PATH"/webby.c" \
	   "../imgui_remote.cpp" \
	   "../imgui_remote_webby.cpp" \
	   main.cpp

#OTHER_FILES+= "../lz4/lz4.h"	\
#	   "../lz4/lz4frame_static.h"	\
#	   "../lz4/lz4hc.h"	\
#	   "../lz4/lz4frame.h"	\
#	   "../lz4/lz4frame_static.h"	\
#	   "../lz4/xxhash.h"	\
#	   "../lz4/lz4.c"	\
#	   "../lz4/lz4hc.c"	\
#	   "../lz4/lz4frame.c"	\
#	   "../lz4/xxhash.c"	\

TARGET = remote_imgui_example


use_glfw3    {
DEFINES+= IMGUI_USE_GLFW_BINDING
PKGCONFIG += glfw3
INCLUDEPATH+=/usr/include/
INCLUDEPATH+=/usr/include/GLFW
HEADERS+= /usr/include/GLFW/glfw3.h
}
use_glut    {
DEFINES+= IMGUI_USE_GLUT_BINDING
LIBS+= -lglut
INCLUDEPATH+=/usr/include/
INCLUDEPATH+=/usr/include/GL
HEADERS+= /usr/include/GL/glut.h
}
use_sdl2    {
DEFINES+= IMGUI_USE_SDL2_BINDING
PKGCONFIG += sdl2
INCLUDEPATH+=/usr/include/
INCLUDEPATH+=/usr/include/SDL2
HEADERS+= /usr/include/SDL2/SDL.h
}
use_glew {
DEFINES+= IMGUI_USE_GLEW
LIBS+=-lGLEW
INCLUDEPATH+=/usr/include/
INCLUDEPATH+=/usr/include/GL
HEADERS+= /usr/include/GL/glew.h
}
use_glad {
GLAD_BASE_PATH ="/home/flix/Desktop/Repositories/glad-openGL4.5"    # This should be changed on a per user basis
DEFINES+= IMGUI_USE_GLAD
INCLUDEPATH+=$$GLAD_BASE_PATH"/include/"
INCLUDEPATH+=$$GLAD_BASE_PATH"/src"
HEADERS+= $$GLAD_BASE_PATH"/include/glad/glad.h"
SOURCES+=$$GLAD_BASE_PATH"/src/glad.c"
}
use_gl3w {
GL3W_BASE_PATH ="../libs/gl3w/"    # This should be changed on a per user basis
DEFINES+= IMGUI_USE_GL3W
INCLUDEPATH+=$$GL3W_BASE_PATH
HEADERS+= $$GL3W_BASE_PATH/GL/gl3w.h
SOURCES+= $$GL3W_BASE_PATH/GL/gl3w.c
}

!use_winapi {
!use_direct3d9	{
LIBS+= -lpthread  -lGL -lX11 -ldl  #-static-libstdc++ -static-libgcc

# Please remove these 3 lines-------------------
#DEFINES+=IMGUIFILESYSTEM_USES_STD_FILESYSTEM
#QMAKE_CXXFLAGS+=--std=c++11
#LIBS+=-lstdc++fs
# ---------------------------------------------

}
}
use_winapi    {
DEFINES+= IMIMPL_USE_WINAPI _WIN32 WIN32 #_MSC_VER
# These paths should be adapted as well:
INCLUDEPATH+=/usr/i686-w64-mingw32/include		\
	     /usr/include/c++/4.6/i686-w64-mingw32	\
	     /usr/include/c++/4.6/i686-w64-mingw32/bits	\
	     /usr/lib/gcc/i686-w64-mingw32/4.6/include
LIBS+= -L"/usr/i686-w64-mingw32/lib" \
       -L"/usr/lib/gcc/i686-w64-mingw32/4.6"

LIBS+= -lopengl32 -luser32 -lkernel32 -static-libgcc -static-libstdc++
#QMAKE_CXXFLAGS+=--std=c++0x
}
use_direct3d9 {
DEFINES+= IMIMPL_USE_DIRECT3D9 _WIN32 WIN32 #_MSC_VER
# These paths should be adapted as well:
INCLUDEPATH+=/usr/i686-w64-mingw32/include		\
	     /usr/include/c++/4.6/i686-w64-mingw32	\
	     /usr/include/c++/4.6/i686-w64-mingw32/bits	\
	     /usr/lib/gcc/i686-w64-mingw32/4.6/include
LIBS+= -L"/usr/i686-w64-mingw32/lib" \
       -L"/usr/lib/gcc/i686-w64-mingw32/4.6"

LIBS+= -ld3d9 -luser32 -lkernel32 -static-libgcc -static-libstdc++
#QMAKE_CXXFLAGS+=--std=c++0x
}

# Dev stuff here (should be commented out)
#DEFINES+=IMGUI_DISABLE_OBSOLETE_FUNCTIONS
