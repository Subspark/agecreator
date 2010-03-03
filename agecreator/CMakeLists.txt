project(AGECREATOR)
cmake_minimum_required(VERSION 2.6)
find_package(HSPlasma)
find_package(Qt4)

set(PLASMA_DIR "./plasma" CACHE PATH "Location of Plasma library sources")

add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(NOT HSPlasma_FOUND)
  file(RELATIVE_PATH PLASMA_RELATIVE_DIR ${AGECREATOR_SOURCE_DIR} ${PLASMA_DIR})
  add_subdirectory(${PLASMA_RELATIVE_DIR})
  include_directories(${PLASMA_RELATIVE_DIR}/core)
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
    gui/ACAgeWizard.cpp
    gui/ACExportDialog.cpp
    gui/ACMainWindow.cpp
    gui/ACNewLayerDialog.cpp
    gui/ACNewObjectDialog.cpp
    object/ACDrawable.cpp
    object/ACObject.cpp
    object/ACSpawnPoint.cpp
)

set(AC_HDRS
    ACAge.h
    ACLayer.h
    ACPage.h
    ACUtil.h
    gui/ACAgeWizard.h
    gui/ACExportDialog.h
    gui/ACMainWindow.h
    gui/ACNewLayerDialog.h
    gui/ACNewObjectDialog.h
    object/ACDrawable.h
    object/ACObject.h
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

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/object
                ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

add_executable(AgeCreator ${AC_SRCS} ${AC_MOC_SRCS} ${AC_HDRS} ${AC_UI_HDRS})
target_link_libraries(AgeCreator HSPlasma ${QT_LIBRARIES})
