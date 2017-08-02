cmake_minimum_required (VERSION 2.8)

project (nightcharts)
find_package(Qt4 COMPONENTS QtCore QtGui)
include(${QT_USE_FILE})

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

