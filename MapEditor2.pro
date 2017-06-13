#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T17:50:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
	/home/anyuser/Downloads/squish-1.11/ \
	../Kreatures/Engine/ \
	../Kreatures/libFreetures/include/

LIBS += -L/home/anyuser/Downloads/squish-1.11/ -lsquish -ldrm

TARGET = MapEditor2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    face.cpp \
    vertex.cpp \
    selection.cpp \
    direction.cpp \
    viewwidget.cpp \
    backgroundimage.cpp \
    background.cpp \
    creaturesio.cpp \
    mapeditor.cpp \
    metaroomoffset.cpp

HEADERS  += mainwindow.h \
    face.h \
    vertex.h \
    selection.h \
    direction.h \
    viewwidget.h \
    backgroundimage.h \
    byte_swap.h \
    background.h \
    creaturesio.h \
    mapeditor.h \
    metaroomoffset.h

FORMS    += mainwindow.ui \
    metaroomoffset.ui
