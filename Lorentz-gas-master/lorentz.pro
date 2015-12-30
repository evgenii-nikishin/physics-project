QT += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app
CONFIG -= console

TARGET = lorentz

HEADERS = src/model.h \
          src/widget.h \
          src/window.h \
          src/qcustomplot.h \
    src/aboutdialog.h

SOURCES = src/model.cpp \
          src/main.cpp \
          src/widget.cpp \
          src/window.cpp \
          src/qcustomplot.cpp \
    src/aboutdialog.cpp

FORMS = src/window.ui \
    src/aboutdialog.ui

TRANSLATIONS = translations/lorentz_ru.ts

RESOURCES += resources.qrc






