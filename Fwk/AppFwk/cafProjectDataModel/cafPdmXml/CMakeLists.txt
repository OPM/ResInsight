cmake_minimum_required (VERSION 2.8)

project (cafPdmXml)

# Qt
find_package ( Qt4 COMPONENTS QtCore )	
include (${QT_USE_FILE})

include_directories (
	..
)

set( PROJECT_FILES
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

	cafPdmXmlStringValidation.h
)


add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})
