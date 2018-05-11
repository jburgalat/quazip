TEMPLATE = app
QT -= gui
QT += network
CONFIG += qtestlib
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
INCLUDEPATH += .
!win32: LIBS += -lz
win32 {
    # workaround for qdatetime.h macro bug
    DEFINES += NOMINMAX
}

# For Qt5 we use the embedded version of zlib.
# We need to have Qt5 sources files installed on the system.
greaterThan(QT_MAJOR_VERSION, 4): INCLUDEPATH += $$absolute_path($$[QT_INSTALL_PREFIX]/../Src/qtbase/src/3rdparty/zlib)

CONFIG(staticlib): DEFINES += QUAZIP_STATIC

# Input
HEADERS += qztest.h \
testjlcompress.h \
testquachecksum32.h \
testquagzipfile.h \
testquaziodevice.h \
testquazipdir.h \
testquazipfile.h \
testquazip.h \
    testquazipnewinfo.h \
    testquazipfileinfo.h

SOURCES += qztest.cpp \
testjlcompress.cpp \
testquachecksum32.cpp \
testquagzipfile.cpp \
testquaziodevice.cpp \
testquazip.cpp \
testquazipdir.cpp \
testquazipfile.cpp \
    testquazipnewinfo.cpp \
    testquazipfileinfo.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../quazip/release/ -lquazip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/debug/ -lquazipd
else:mac:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/ -lquazip_debug
else:unix: LIBS += -L$$OUT_PWD/../quazip/ -lquazip

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/../quazip
