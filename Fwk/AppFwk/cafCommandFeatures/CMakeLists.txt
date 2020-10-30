cmake_minimum_required(VERSION 3.12)

project(cafCommandFeatures)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# These headers need to go through Qt's MOC compiler
set(MOC_HEADER_FILES)

# Qt
find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})

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
    ToggleCommands/cafToggleItemsOnOthersOffFeature.h)

# NOTE! Resources in this subfolder appends to the variable QRC_FILES in parent
# scope CMakeList.txt in the application folder (parent scope) must use the
# following syntax to make sure the QRC_FILES variable contains appended files
# in subfolders

# set( QRC_FILES ${QRC_FILES} <Path/To/ApplicationResourceFile.qrc> )

set(QRC_FILES
    ${QRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/Resources/cafCommandFeatures.qrc
    PARENT_SCOPE)

# NOTE! Adding the library as a cmake "OBJECT" library to make sure the linker
# is not pruning the seemingly unused features, and to make sure that the static
# initialization based registration of the features into the factory is done
# properly see
# https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Object-Library
# and
# https://cmake.org/cmake/help/v3.15/command/add_library.html?highlight=add_library#object-libraries
add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES} ${MOC_SOURCE_FILES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:cafCommand,INCLUDE_DIRECTORIES> # Needed for cmake version
                                                      # < 3.12. Remove when we
                                                      # can use
                                                      # target_link_libraries
    $<TARGET_PROPERTY:cafUserInterface,INCLUDE_DIRECTORIES> # Needed for cmake
                                                            # version < 3.12.
                                                            # Remove when we can
                                                            # use
                                                            # target_link_libraries
)

# Before cmake 3.12 OBJECT libraries could not use the target_link_libraries
# command, So we need to set the POSITION_INDEPENDENT_CODE option  manually

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Not to be used until we can use cmake 3.12 or above target_link_libraries (
# ${PROJECT_NAME} cafCommand cafUserInterface ${QT_LIBRARIES} )

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
