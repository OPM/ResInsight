cmake_minimum_required (VERSION 3.12)

project (cafPdmScripting)

option(CAF_CVF_SCRIPTING "Enable CVF-data support in Scripting" OFF)

# Unity Build
if (CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

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

set (CAF_LIBS cafProjectDataModel)

if (CAF_CVF_SCRIPTING)
    list(APPEND PROJECT_FILES
     cafPdmFieldScriptingCapabilityCvfColor3.h
     cafPdmFieldScriptingCapabilityCvfColor3.cpp
     cafPdmFieldScriptingCapabilityCvfVec3d.h
     cafPdmFieldScriptingCapabilityCvfVec3d.cpp
    )
    list(APPEND CAF_LIBS cafPdmCvf)
endif()



add_library( ${PROJECT_NAME}
    ${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)




target_link_libraries ( ${PROJECT_NAME}
    ${CAF_LIBS}
)

source_group("" FILES ${PROJECT_FILES})
