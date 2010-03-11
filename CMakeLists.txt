project(AGECREATOR)
cmake_minimum_required(VERSION 2.6)
find_package(HSPlasma)
find_package(Qt4 REQUIRED)

set(PLASMA_DIR "./plasma" CACHE PATH "Location of Plasma library sources")

set(QT_USE_QTOPENGL 1)

if(NOT HSPlasma_FOUND)
  add_library(HSPlasma SHARED IMPORTED)
  set_target_properties(HSPlasma PROPERTIES
                        IMPORTED_LOCATION ${HSPLASMA_LIBRARY}
                        IMPORTED_IMPLIB ${HSPLASMA_IMPLIB}
  )

  include_directories(${HSPLASMA_INCLUDE})
else(NOT HSPlasma_FOUND)
  include_directories(${Plasma_INCLUDE_DIRS})
endif(NOT HSPlasma_FOUND)

include(${QT_USE_FILE})

set(AC_SRCS
    main.cpp
    ACAge.cpp
    ACLayer.cpp
    ACPage.cpp
    ACUtil.cpp
    gui/ACAgeTreeView.cpp
    gui/ACAgeWizard.cpp
    gui/ACExportDialog.cpp
    gui/ACGLWidget.cpp
    gui/ACMainWindow.cpp
    gui/ACNewLayerDialog.cpp
    gui/ACNewObjectDialog.cpp
    object/ACDrawable.cpp
    object/ACObject.cpp
    object/ACPhysicalDrawable.cpp
    object/ACSpawnPoint.cpp
)

set(AC_HDRS
    ACAge.h
    ACLayer.h
    ACPage.h
    ACUtil.h
    gui/ACAgeTreeView.h
    gui/ACAgeWizard.h
    gui/ACExportDialog.h
    gui/ACGLWidget.h
    gui/ACMainWindow.h
    gui/ACNewLayerDialog.h
    gui/ACNewObjectDialog.h
    object/ACDrawable.h
    object/ACObject.h
    object/ACPhysicalDrawable.h
    object/ACSpawnPoint.h
)

set(AC_UIS
    gui/ACExportDialog.ui
    gui/ACMainWindow.ui
    gui/ACNewLayerDialog.ui
    gui/ACNewObjectDialog.ui
)

QT4_WRAP_UI(AC_UI_HDRS ${AC_UIS})
qt4_wrap_cpp(AC_MOC_SRCS ${AC_HDRS})

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  qt4_add_resources(AC_QRC_SRCS icons.qrc)
endif(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/object
                ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

add_subdirectory(3rdparty/QPropertyEditor)
include_directories(3rdparty/QPropertyEditor)

add_executable(AgeCreator WIN32 ${AC_SRCS} ${AC_MOC_SRCS} ${AC_HDRS} ${AC_UI_HDRS} ${AC_QRC_SRCS})
set_target_properties(AgeCreator PROPERTIES
                      COMPILE_DEFINITIONS QT_NO_CAST_FROM_ASCII
)
target_link_libraries(AgeCreator HSPlasma QPropertyEditor ${QT_LIBRARIES})
install(TARGETS AgeCreator
        DESTINATION bin
)
