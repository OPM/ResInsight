cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui )
include (${QT_USE_FILE})

include_directories (
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
)

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

source_group("" FILES ${PROJECT_FILES})