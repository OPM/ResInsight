cmake_minimum_required (VERSION 2.8.12)

project (cafPdmScripting)

set( PROJECT_FILES
     cafPdmCodeGenerator.h
     cafPdmAbstractFieldScriptingCapability.h
     cafPdmAbstractFieldScriptingCapability.cpp
     cafPdmFieldScriptingCapability.h
     cafPdmFieldScriptingCapability.cpp
	   cafPdmObjectMethod.h
	   cafPdmObjectMethod.cpp
     cafPdmObjectScriptingCapabilityRegister.h
     cafPdmObjectScriptingCapabilityRegister.cpp
     cafPdmPythonGenerator.h
     cafPdmPythonGenerator.cpp
     cafPdmScriptIOMessages.h
     cafPdmScriptIOMessages.cpp
     cafPdmObjectScriptingCapability.h
     cafPdmObjectScriptingCapability.cpp
     cafPdmScriptResponse.h
     cafPdmScriptResponse.cpp
     cafPdmMarkdownGenerator.h
     cafPdmMarkdownGenerator.cpp
     cafPdmMarkdownBuilder.h
     cafPdmMarkdownBuilder.cpp
)


add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
)


source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()