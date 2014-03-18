TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

LIBS += -L/usr/local/lib -lSDL2

INCLUDEPATH += /usr/local/include

SOURCES += main.cpp \
    camera.cpp \
    mesh.cpp \
    device.cpp \
    color.cpp

HEADERS += \
    camera.h \
    mesh.h \
    device.h \
    color.h

