#-------------------------------------------------
#
# Project created by QtCreator 2013-02-27T22:05:02
#
#-------------------------------------------------

QT       -= core gui

TARGET = RelationalObjectModel
TEMPLATE = lib

DEFINES += RELATIONALOBJECTMODEL_LIBRARY \
           USE_LIBXML \
           POSIX

SOURCES += \
    utilities.cpp \
    stdafx.cpp \
    ROMNode.cpp \
    ROMDictionary.cpp \
    LinearEngine.cpp

HEADERS += \
    utilities.h \
    stdafx.h \
    ROMNode.h \
    ROMDictionaryAttribute.h \
    ROMDictionary.h \
    LinearEngine.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE8C5DEAF
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = RelationalObjectModel.dll
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

unix:!symbian: LIBS += -lxml2

unix:!symbian: LIBS += -lxslt

symbian: LIBS += -lEDSEngine
else:unix:!macx: LIBS += -L../EDSEngine/ -lEDSEngine

INCLUDEPATH += ../EDSEngine
DEPENDPATH += ../EDSEngine

QMAKE_CXXFLAGS += -std=c++0x
