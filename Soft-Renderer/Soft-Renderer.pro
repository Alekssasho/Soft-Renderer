TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
QMAKE_CXXFLAGS += -fopenmp


macx{
LIBS += -L/usr/local/lib -lSDL2 -lSDL2_image
INCLUDEPATH += /usr/local/include
}

win32{
QMAKE_LFLAGS *= -fopenmp
LIBS += C:\Libraries\SDL2-2.0.3\lib\x86\SDL2main.lib \
    C:\Libraries\SDL2-2.0.3\lib\x86\SDL2.lib
INCLUDEPATH += C:\Libraries\SDL2-2.0.3\include \
            C:\Libraries\glm
}

SOURCES += main.cpp \
    camera.cpp \
    mesh.cpp \
    device.cpp \
    color.cpp \
    json/json.cpp \
    texture.cpp

HEADERS += \
    camera.h \
    mesh.h \
    device.h \
    color.h \
    json/json.h \
    texture.h

