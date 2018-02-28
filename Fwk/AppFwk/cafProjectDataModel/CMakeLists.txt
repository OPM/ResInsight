cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui )
include (${QT_USE_FILE})

set( PROJECT_FILES
	cafFactory.h
	cafFixedArray.h
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

target_link_libraries ( ${PROJECT_NAME}
    cafPdmUiCore
    cafPdmXml
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})
