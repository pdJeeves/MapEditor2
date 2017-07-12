#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T17:50:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
	../squish-1.11/

LIBS += -fPIC -lsquish -ldrm


TARGET = MapEditor2
TEMPLATE = app


SOURCES += src/main.cpp\
		src/mainwindow.cpp \
	src/face.cpp \
	src/vertex.cpp \
	src/selection.cpp \
	src/direction.cpp \
	src/viewwidget.cpp \
	src/backgroundimage.cpp \
	src/background.cpp \
	src/creaturesio.cpp \
	src/mapeditor.cpp \
	src/metaroomoffset.cpp \
	src/casting.cpp \
	src/commandlist.cpp \
	src/properties.cpp \
    src/grabcommand.cpp \
    src/insertioncommand.cpp \
    src/permeabilitycommand.cpp \
    src/slicecommand.cpp \
    src/deletioncommand.cpp \
    src/commandinterface.cpp \
    src/roomtypecommand.cpp

HEADERS  += src/mainwindow.h \
	src/face.h \
	src/vertex.h \
	src/selection.h \
	src/direction.h \
	src/viewwidget.h \
	src/backgroundimage.h \
	src/byte_swap.h \
	src/background.h \
	src/creaturesio.h \
	src/mapeditor.h \
	src/metaroomoffset.h \
	src/commandinterface.h \
	src/commandlist.h \
	src/properties.h \
    src/grabcommand.h \
    src/insertioncommand.h \
    src/permeabilitycommand.h \
    src/slicecommand.h \
    src/deletioncommand.h \
    src/roomtypecommand.h

FORMS    += src/mainwindow.ui \
	src/metaroomoffset.ui \
	src/properties.ui
