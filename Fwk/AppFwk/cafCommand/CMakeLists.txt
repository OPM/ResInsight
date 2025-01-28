project(cafCommand)

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
    cafCmdExecCommandManager.cpp
    cafCmdExecCommandManager.h
    cafCmdExecuteCommand.h
    cafCmdUiCommandSystemImpl.h
    cafCmdUiCommandSystemImpl.cpp
    cafCmdFieldChangeExec.cpp
    cafCmdFieldChangeExec.h
    cafCmdSelectionHelper.cpp
    cafCmdSelectionHelper.h
    cafCmdSelectionChangeExec.cpp
    cafCmdSelectionChangeExec.h
    cafCmdFeature.cpp
    cafCmdFeature.h
    cafCmdFeatureManager.cpp
    cafCmdFeatureManager.h
    cafCmdFeatureMenuBuilder.cpp
    cafCmdFeatureMenuBuilder.h
)

add_library(${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} cafProjectDataModel ${QT_LIBRARIES})

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /wd4996"
  )
endif()
