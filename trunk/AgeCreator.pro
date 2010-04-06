TEMPLATE = app
TARGET = bin/AgeCreator
CONFIG += debug
QT += opengl

INCLUDEPATH += ../Plasma/core . gui object \
               3rdparty/QtIconLoader 3rdparty/glew
LIBS += -L../Plasma/lib -lHSPlasma
SUBDIRS = 3rdparty/QtIconLoader
DEFINES += GLEW_MX GLEW_STATIC

SOURCES = \
    main.cpp \
    ACAge.cpp \
    ACLayer.cpp \
    ACPage.cpp \
    ACUtil.cpp \
    gui/AC2dWidget.cpp \
    gui/ACAgeTreeView.cpp \
    gui/ACAgeWizard.cpp \
    gui/ACExportDialog.cpp \
    gui/ACGLWidget.cpp \
    gui/ACMainWindow.cpp \
    gui/ACMaterialEditor.cpp \
    gui/ACObjectNameDelegate.cpp \
    object/ACDrawable.cpp \
    object/ACMesh.cpp \
    object/ACObject.cpp \
    object/ACPhysicalDrawable.cpp \
    object/ACSpawnPoint.cpp \
    3rdparty/glew/glew.c \
    3rdparty/QtIconLoader/qticonloader.cpp

HEADERS = \
    ACAge.h \
    ACLayer.h \
    ACPage.h \
    ACUtil.h \
    gui/AC2dWidget.h \
    gui/ACAgeTreeView.h \
    gui/ACAgeWizard.h \
    gui/ACExportDialog.h \
    gui/ACGLWidget.h \
    gui/ACMainWindow.h \
    gui/ACMaterialEditor.h \
    gui/ACObjectNameDelegate.h \
    object/ACDrawable.h \
    object/ACMesh.h \
    object/ACObject.h \
    object/ACPhysicalDrawable.h \
    object/ACSpawnPoint.h \
    3rdparty/QtIconLoader/qticonloader.h

FORMS = \
    gui/ACExportDialog.ui \
    gui/ACMainWindow.ui \
    gui/ACMaterialEditor.ui

RESOURCES = \
    data.qrc

!linux {
    RESOURCES += icons.qrc
}


