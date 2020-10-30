cmake_minimum_required(VERSION 3.12)

project(cafPdmXml)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Xml)
set(QT_LIBRARIES Qt5::Core Qt5::Xml)

include_directories(..)

set(PROJECT_FILES
    cafInternalPdmFieldIoHelper.cpp
    cafInternalPdmFieldIoHelper.h
    cafInternalPdmFilePathStreamOperators.cpp
    cafInternalPdmFilePathStreamOperators.h
    cafInternalPdmStreamOperators.cpp
    cafInternalPdmStreamOperators.h
    cafInternalPdmXmlFieldCapability.h
    cafInternalPdmXmlFieldCapability.inl
    cafInternalPdmXmlFieldReaderWriter.cpp
    cafInternalPdmXmlFieldReaderWriter.h
    cafPdmXmlFieldHandle.cpp
    cafPdmXmlFieldHandle.h
    cafPdmXmlObjectHandle.cpp
    cafPdmXmlObjectHandle.h
    cafPdmXmlObjectHandleMacros.h
    cafPdmDefaultObjectFactory.cpp
    cafPdmDefaultObjectFactory.h
    cafPdmObjectFactory.h
    cafPdmSettings.h
    cafPdmSettings.cpp
    cafPdmXmlStringValidation.h)

add_library(${PROJECT_NAME} ${PROJECT_FILES})

target_link_libraries(${PROJECT_NAME} cafPdmCore ${QT_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
