cmake_minimum_required (VERSION 2.8)

project (nightcharts)
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui Widgets)
else()
  find_package(Qt4 COMPONENTS QtCore QtGui)
  include(${QT_USE_FILE})
endif()

list (APPEND MAIN_SOURCE_FILES
    nightcharts.h
    nightcharts.cpp
    nightchartswidget.h
    nightchartswidget.cpp
)

add_library( ${PROJECT_NAME}
    STATIC
    ${MAIN_SOURCE_FILES}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets)
endif()
