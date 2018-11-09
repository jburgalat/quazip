# Quazip project file (v0.7.6)

TEMPLATE = lib
CONFIG += qt warn_on
QT -= gui


# The ABI version.

!win32:VERSION = 1.0.0

# 1.0.0 is the first stable ABI.
# The next binary incompatible change will be 2.0.0 and so on.
# The existing QuaZIP policy on changing ABI requires to bump the
# major version of QuaZIP itself as well. Note that there may be
# other reasons for chaging the major version of QuaZIP, so
# in case where there is a QuaZIP major version bump but no ABI change,
# the VERSION variable will stay the same.

# For example:

# QuaZIP 1.0 is released after some 0.x, keeping binary compatibility.
# VERSION stays 1.0.0.
# Then some binary incompatible change is introduced. QuaZIP goes up to
# 2.0, VERSION to 2.0.0.
# And so on.

# install in Qt directories will also define a prf file so we can include quazip using:
#  CONFIG += quazip(d)

INSTALL_IN_QT = true

# This one handles dllimport/dllexport directives.
DEFINES += QUAZIP_COMPILE_LIBRARY

# datetime bug on windaube
win32:DEFINES += NOMINMAX

# Input sources

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += \
        $$PWD/minizip_crypt.h \
        $$PWD/ioapi.h \
        $$PWD/JlCompress.h \
        $$PWD/quaadler32.h \
        $$PWD/quachecksum32.h \
        $$PWD/quacrc32.h \
        $$PWD/quagzipfile.h \
        $$PWD/quaziodevice.h \
        $$PWD/quazipdir.h \
        $$PWD/quazipfile.h \
        $$PWD/quazipfileinfo.h \
        $$PWD/quazip_global.h \
        $$PWD/quazip.h \
        $$PWD/quazipnewinfo.h \
        $$PWD/unzip.h \
        $$PWD/zip.h

SOURCES += $$PWD/qioapi.cpp \
           $$PWD/JlCompress.cpp \
           $$PWD/quaadler32.cpp \
           $$PWD/quacrc32.cpp \
           $$PWD/quagzipfile.cpp \
           $$PWD/quaziodevice.cpp \
           $$PWD/quazip.cpp \
           $$PWD/quazipdir.cpp \
           $$PWD/quazipfile.cpp \
           $$PWD/quazipfileinfo.cpp \
           $$PWD/quazipnewinfo.cpp \
           $$PWD/unzip.c \
           $$PWD/zip.c

# For Qt5 we use the embedded version of zlib.
# We need to have Qt5 sources files installed on the system.
INCLUDEPATH += $$absolute_path($$[QT_INSTALL_PREFIX]/../Src/qtbase/src/3rdparty/zlib/src)

# JB 11052018: additions for progress report
HEADERS += $$PWD/jlcompress_obj.hpp \
           $$PWD/jlworker.hpp
SOURCES += $$PWD/jlcompress_obj.cpp \
           $$PWD/jlworker.cpp


# add QT5 src zlib header to Quazip installation.
headers.files += $$absolute_path($$[QT_INSTALL_PREFIX]/../Src/qtbase/src/3rdparty/zlib/src/zlib.h)

# uppdate target name for Win/Mac in debug mode
win32{
    CONFIG(debug, debug|release):TARGET = $$join(TARGET,,,d)
}else:macx{
    CONFIG(debug, debug|release):TARGET = $$join(TARGET,,,_debug)
}


# Generate the prf file (ugly but it works)

QZP_LIB_NAME            = $$TARGET
QZP_DLL_EXPORT          = QUAZIP_USE_LIBRARY
QZP_INCLUDE_INSTALL_DIR =
QZP_LIBRARY_INSTALL_DIR =
QZP_EXTRA_MODULES       =
QZP_EXTRA_LIBS          =
QZP_CPP_FLAGS           =


LIBS += $$QZP_EXTRA_LIBS

# should we install in Qt directory ?
contains(INSTALL_IN_QT,true){
    BUILD_DIR= $$[QT_INSTALL_PREFIX]
}else{
    win32{
        BUILD_DIR= $$PWD/build
    }else:unix{
        BUILD_DIR = /$(HOME)/local
    }
}
QZP_INCLUDE_INSTALL_DIR=$${BUILD_DIR}/include
windows:QZP_LIBRARY_INSTALL_DIR=$${BUILD_DIR}/bin
unix:QZP_LIBRARY_INSTALL_DIR=$${BUILD_DIR}/lib
QZP_PRF_INSTALL_DIR=$${BUILD_DIR}/mkspecs/features

# Installation rules

# Target
target.path = $${QZP_LIBRARY_INSTALL_DIR}

# associated headers
headers_files.files = $$HEADERS
headers_files.path = $${QZP_INCLUDE_INSTALL_DIR}/quazip

# A feature file build from scratch
prf_tpl = \
"QZP_INCDIR = $${QZP_INCLUDE_INSTALL_DIR}" \
"QZP_LIBDIR = $${QZP_LIBRARY_INSTALL_DIR}" \
"CONFIG *= qt" \
"QT     += widgets svg printsupport concurrent $${QZP_EXTRA_MODULES}" \
"LINKAGE =" \
"exists(\$\${QZP_LIBDIR}/qplotlib.framework) {" \
"  QMAKE_CXXFLAGS += -F\$\${QZP_LIBDIR}" \
"  LIBS *= -F\$\${QZP_LIBDIR}" \
"  INCLUDEPATH += \$\${QZP_LIBDIR}/qplotlib.framework/Headers" \
"  LINKAGE = -framework quazip" \
"}" \
"isEmpty(LINKAGE) {" \
"  INCLUDEPATH += \$\${QZP_INCDIR}/quazip" \
"  LIBS += -L\$\${QZP_LIBDIR}" \
"  LINKAGE = -l$${QZP_LIB_NAME}" \
"  LINKAGE += $${QZP_EXTRA_LIBS}" \
"}" \
"DEFINES +=$${QZP_CPP_FLAGS}" \
"windows:DEFINES +=$${QZP_DLL_EXPORT}" \
"LIBS += \$\$LINKAGE"
# The name of the file deppends on the build
CONFIG(debug, debug|release):PRF_FILE=$$OUT_PWD/quazipd.prf
CONFIG(release, debug|release):PRF_FILE=$$OUT_PWD/quazip.prf
write_file("$${PRF_FILE}",prf_tpl)

prf_file.files = $${PRF_FILE}
prf_file.path = $${QZP_PRF_INSTALL_DIR}

INSTALLS += target \
    headers_files \
    prf_file

# Ugly hack on windows to put the lib file where it belongs if we install in Qt directories
contains(INSTALL_IN_QT,true){
    win32:!win32-g++:{
        movelib.target =  $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.lib")
        movelib.path = $$shell_path("$${BUILD_DIR}/lib/")
        movelib.commands = if exist $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.lib") $(MOVE) $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.lib") $$shell_path("$${BUILD_DIR}/lib/")
        movelib.depends = install_target
        INSTALLS += movelib
        # also copy pdb file.
        CONFIG(debug,debug|release){
        movepdb.target =  $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.pdb")
        movepdb.path = $$shell_path("$${BUILD_DIR}/lib/")
        movepdb.commands = if exist $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.pdb") $(MOVE) $$shell_path("$${BUILD_DIR}/bin/$${QZP_LIB_NAME}.pdb") $$shell_path("$${BUILD_DIR}/lib/")
        movepdb.depends = install_target
        INSTALLS += movepdb
        }
    }
}
