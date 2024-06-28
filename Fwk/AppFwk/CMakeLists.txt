cmake_minimum_required(VERSION 3.15)

project(CeeApp)

if(MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code
  # related to TR1
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(
    "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
  )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Qt
if(NOT DEFINED (CEE_USE_QT5))
  option(CEE_USE_QT5 "Use Qt5" ON)
endif(NOT DEFINED (CEE_USE_QT5))

if(NOT DEFINED (CEE_USE_QT6))
  option(CEE_USE_QT6 "Use Qt6" OFF)
endif(NOT DEFINED (CEE_USE_QT6))

# CeeViz is not available here, exclude it from the build
set(CAF_EXCLUDE_CVF ON)

# Improvement Cmake 3.x If we have compiler requirements for this library, list
# them target_compile_features(lib PUBLIC cxx_auto_type PRIVATE
# cxx_variadic_templates)

# libraries
add_subdirectory(cafProjectDataModel/cafPdmCore)
add_subdirectory(cafProjectDataModel/cafPdmUiCore)
add_subdirectory(cafProjectDataModel/cafPdmXml)

add_subdirectory(cafProjectDataModel)
add_subdirectory(cafCommand)
add_subdirectory(cafCommandFeatures)
add_subdirectory(cafUserInterface)
add_subdirectory(cafPdmScripting)
add_subdirectory(cafDataLoader)

# executables
add_subdirectory(cafTests/cafTestApplication)

if(UNIX)
  set(THREAD_LIBRARY "pthread")
endif()

add_subdirectory(cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory(cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
add_subdirectory(cafProjectDataModel/cafProjectDataModel_UnitTests)
add_subdirectory(cafUserInterface/cafUserInterface_UnitTests)
add_subdirectory(cafPdmScripting/cafPdmScripting_UnitTests)

# Organize sub-projects into folders on Visual Studio Turn on using solution
# folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(
  TARGET cafPdmCore cafPdmCore_UnitTests cafPdmXml cafPdmXml_UnitTests
         cafPdmUiCore PROPERTY FOLDER "PdmCore"
)
