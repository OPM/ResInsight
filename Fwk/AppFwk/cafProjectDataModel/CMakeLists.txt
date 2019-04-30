cmake_minimum_required (VERSION 2.8.12)

project (cafProjectDataModel)

# Qt
if (CAF_USE_QT5)
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
    include(${QT_USE_FILE})
endif(CAF_USE_QT5)

set( PROJECT_FILES
	cafFactory.h
	cafOmpMutex.h
	cafPdmDocument.cpp
	cafPdmDocument.h
	cafPdmObjectGroup.cpp
	cafPdmObjectGroup.h
	cafPdmObject.h
	cafPdmField.h
)

add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

if (CAF_USE_QT5)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif(CAF_USE_QT5)

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

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()
