project(cafDataLoader)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets)
qt_standard_project_setup()

set(PROJECT_FILES
    cafDataLoader.h cafDataLoader.cpp cafDataLoadTask.h cafDataLoadTask.cpp
    cafDataLoadController.h cafDataLoadController.cpp
)

add_library(${PROJECT_NAME} ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} cafProjectDataModel cafUserInterface ${QT_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /wd4996"
  )
endif()
