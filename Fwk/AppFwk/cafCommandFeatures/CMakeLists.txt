project(cafCommandFeatures)

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
    # Default features
    AddAndDelete/cafCmdAddItemExec.cpp
    AddAndDelete/cafCmdAddItemExec.h
    AddAndDelete/cafCmdAddItemExecData.cpp
    AddAndDelete/cafCmdAddItemExecData.h
    AddAndDelete/cafCmdAddItemFeature.cpp
    AddAndDelete/cafCmdAddItemFeature.h
    AddAndDelete/cafCmdDeleteItemExec.cpp
    AddAndDelete/cafCmdDeleteItemExec.h
    AddAndDelete/cafCmdDeleteItemExecData.cpp
    AddAndDelete/cafCmdDeleteItemExecData.h
    AddAndDelete/cafCmdDeleteItemFeature.cpp
    AddAndDelete/cafCmdDeleteItemFeature.h
    ToggleCommands/cafToggleItemsFeature.cpp
    ToggleCommands/cafToggleItemsFeature.h
    ToggleCommands/cafToggleItemsFeatureImpl.cpp
    ToggleCommands/cafToggleItemsFeatureImpl.h
    ToggleCommands/cafToggleItemsOffFeature.cpp
    ToggleCommands/cafToggleItemsOffFeature.h
    ToggleCommands/cafToggleItemsOnFeature.cpp
    ToggleCommands/cafToggleItemsOnFeature.h
    ToggleCommands/cafToggleItemsOnOthersOffFeature.cpp
    ToggleCommands/cafToggleItemsOnOthersOffFeature.h
)

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent
# scope CMakeList.txt in the application folder (parent scope) must use the
# following syntax to make sure the QRC_FILES variable contains appended files
# in subfolders

# set( QRC_FILES ${QRC_FILES} <Path/To/ApplicationResourceFile.qrc> )

set(QRC_FILES
    ${QRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Resources/cafCommandFeatures.qrc
    PARENT_SCOPE
)

# NOTE! Adding the library as a cmake "OBJECT" library to make sure the linker
# is not pruning the seemingly unused features, and to make sure that the static
# initialization based registration of the features into the factory is done
# properly see
# https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Object-Library
# and
# https://cmake.org/cmake/help/v3.15/command/add_library.html?highlight=add_library#object-libraries

add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES} ${MOC_SOURCE_FILES})

target_link_libraries(
  ${PROJECT_NAME} cafCommand cafUserInterface ${QT_LIBRARIES}
)

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127 /wd4996"
  )
endif()


