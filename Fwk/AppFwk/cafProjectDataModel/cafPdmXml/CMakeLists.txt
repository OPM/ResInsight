cmake_minimum_required (VERSION 2.8)

project (cafPdmXml)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core)
else()
  find_package ( Qt4 COMPONENTS QtCore )
  include (${QT_USE_FILE})
endif()

include_directories (
	..
    ${cafPdmCore_SOURCE_DIR}
)

set( PROJECT_FILES
	cafInternalPdmFieldIoHelper.cpp
	cafInternalPdmFieldIoHelper.h
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

	cafPdmXmlStringValidation.h
)


add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core)
endif()

source_group("" FILES ${PROJECT_FILES})
