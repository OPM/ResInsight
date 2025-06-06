project(cafAnimControl)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
qt_standard_project_setup()

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent
# scope CMakeList.txt in the application folder (parent scope) must use the
# following syntax to make sure the QRC_FILES variable contains appended files
# in subfolders

# set( QRC_FILES ${QRC_FILES} Resources/MyaApplication.qrc )

set(QRC_FILES
    ${QRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Resources/cafAnimControl.qrc
    PARENT_SCOPE
)

set(PROJECT_FILES
    cafFrameAnimationControl.h cafFrameAnimationControl.cpp
    cafAnimationToolBar.h cafAnimationToolBar.cpp cafPopupMenuButton.h
    cafPopupMenuButton.cpp
)

add_library(${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4127")
endif()
