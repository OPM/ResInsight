cmake_minimum_required (VERSION 3.12)

project (cafProjectDataModel)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)

set( PROJECT_FILES
	cafFactory.h
	cafOmpMutex.h
	cafPdmDocument.cpp
	cafPdmDocument.h
	cafPdmObjectGroup.cpp
	cafPdmObjectGroup.h
	cafPdmObject.cpp
	cafPdmObject.h
)

add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmUiCore
    cafPdmXml
    ${QT_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
