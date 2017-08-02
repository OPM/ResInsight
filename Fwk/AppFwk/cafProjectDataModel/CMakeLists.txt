cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui )
  include (${QT_USE_FILE})
endif()

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
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui)
endif()

source_group("" FILES ${PROJECT_FILES})
