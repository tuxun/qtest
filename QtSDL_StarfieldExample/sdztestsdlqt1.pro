#-------------------------------------------------
#
# Project created by QtCreator 2014-11-24T15:57:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sdztestsdlqt1
TEMPLATE = app
INCLUDEPATH += /usr/local/include/
LIBS +=-L/SDL/lib/x86 -lSDL
LIBS +=-L/SDL/lib/x86 -lSDLmain
SOURCES += main.cpp

HEADERS  +=
