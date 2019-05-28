cmake_minimum_required (VERSION 2.8.12)

project (nightcharts)

if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Widgets)
	set(QT_LIBRARIES Qt5::Core Qt5::Widgets)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})	
else()
    find_package(Qt4 COMPONENTS QtCore QtWidgets)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
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

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})
