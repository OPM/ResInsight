cmake_minimum_required (VERSION 2.8.12)

project (nightcharts)

if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
    include(${QT_USE_FILE})
endif(RESINSIGHT_BUILD_WITH_QT5)

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

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (RESINSIGHT_BUILD_WITH_QT5)
	target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)
endif(RESINSIGHT_BUILD_WITH_QT5)

source_group("" FILES ${PROJECT_FILES})
