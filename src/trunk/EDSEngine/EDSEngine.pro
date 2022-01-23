#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T02:53:03
#
#-------------------------------------------------

QT       -= core gui

TARGET = EDSEngine
TEMPLATE = lib

DEFINES += EDSENGINE_LIBRARY \
        USE_LIBXML \
        USE_JAVASCRIPT \
        NOPYTHON \
        POSIX

INCLUDEPATH += /usr/include/libxml2 \
               /usr/include/js \
               /usr/include/python2.7

SOURCES += \
    utilities.cpp \
    TableSet.cpp \
    stdafx.cpp \
    RuleTable.cpp \
    KnowledgeBase.cpp \
    EDSEngine.cpp \
    Decode.cpp \
    Bimapper.cpp

HEADERS += \
    XMLWrapper.h \
    utilities.h \
    TableSet.h \
    stdafx.h \
    RuleTable.h \
    RuleCell.h \
    Numerics.h \
    KnowledgeBase.h \
    Decode.h \
    Bimapper.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE8C2997C
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = EDSEngine.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

unix:!symbian: LIBS += -lmozjs185

unix:!symbian: LIBS += -lxml2

unix:!symbian: LIBS += -lxslt

unix:!symbian: LIBS += -lpython2.7

unix:!symbian: LIBS += -lboost_python-py27

unix:!symbian: LIBS += -lboost_iostreams-mt

unix:!symbian: LIBS += -lboost_system

QMAKE_CXXFLAGS += -std=c++0x
