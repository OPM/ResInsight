project (nightcharts)

if (CEE_USE_QT6)
    find_package(Qt6 COMPONENTS REQUIRED Core Gui Widgets)
    set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
    qt_standard_project_setup()
else()
    find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
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

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})
