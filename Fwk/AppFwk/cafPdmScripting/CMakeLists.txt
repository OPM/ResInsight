cmake_minimum_required (VERSION 2.8.12)

project (cafPdmScripting)

set( PROJECT_FILES
     cafPdmCodeGenerator.h
     cafPdmFieldScriptability.h
     cafPdmFieldScriptability.cpp
     cafPdmValueFieldScriptability.h
     cafPdmValueFieldScriptability.cpp
	 cafPdmObjectMethod.h
	 cafPdmObjectMethod.cpp
     cafPdmObjectScriptabilityRegister.h
     cafPdmObjectScriptabilityRegister.cpp
     cafPdmPythonGenerator.h
     cafPdmPythonGenerator.cpp
     cafPdmScriptIOMessages.h
     cafPdmScriptIOMessages.cpp
     cafPdmObjectScriptability.h
     cafPdmObjectScriptability.cpp
     cafPdmScriptResponse.h
     cafPdmScriptResponse.cpp
)


add_library( ${PROJECT_NAME}
 	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries( ${PROJECT_NAME} LibCore cafProjectDataModel ${QT_LIBRARIES} )

source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()