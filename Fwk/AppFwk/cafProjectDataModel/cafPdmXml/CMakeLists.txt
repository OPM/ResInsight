cmake_minimum_required (VERSION 2.8.12)

project (cafPdmXml)

# Qt
if (CAF_USE_QT5)
	find_package(Qt5 COMPONENTS Core Xml REQUIRED)
else()
	find_package(Qt4 COMPONENTS QtCore QtXml REQUIRED)
    include(${QT_USE_FILE})
endif(CAF_USE_QT5)

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

if (CAF_USE_QT5)
    set(QT_LIBRARIES Qt5::Core Qt5::Xml)
endif(CAF_USE_QT5)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
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

if (COMMAND ri_apply_cotire)
  ri_apply_cotire()
endif()
