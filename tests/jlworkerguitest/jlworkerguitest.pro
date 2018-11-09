QT += core gui widgets

CONFIG += c++11

TARGET = JlWorkerGUI

TEMPLATE = app

SOURCES += main.cpp \
    dialog.cpp \
    quaziptreemodel.cpp \
    widgets.cpp \
    times.cpp

HEADERS += \
    dialog.hpp \
    quaziptreemodel.hpp \
    widgets.hpp \
    times.hpp


win32:DEFINES+=QUAZIP_USE_LIBRARY

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../quazip/release/ -lquazip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../quazip/debug/ -lquazipd
else:mac:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../quazip/debug/ -lquazip_debug
else:unix: LIBS += -L$$OUT_PWD/../../quazip/ -lquazip

INCLUDEPATH += $$PWD/../..

DEPENDPATH += $$PWD/../../quazip


# For Qt5 we use the embedded version of zlib.
# We need to have Qt5 sources files installed on the system.
INCLUDEPATH += $$absolute_path($$[QT_INSTALL_PREFIX]/../Src/qtbase/src/3rdparty/zlib/src)
