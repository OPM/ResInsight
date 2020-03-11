cmake_minimum_required (VERSION 2.8.12)

project (cafPdmScripting)

set( PROJECT_FILES
     cafPdmCodeGenerator.h
     cafPdmFieldScriptability.h
     cafPdmFieldScriptability.cpp
     cafPdmFieldIOScriptability.h
     cafPdmFieldIOScriptability.cpp
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
     cafPdmMarkdownGenerator.h
     cafPdmMarkdownGenerator.cpp
     cafPdmMarkdownBuilder.h
     cafPdmMarkdownBuilder.cpp
)


# NOTE! Adding the library as a cmake "OBJECT" library
# to make sure the linker is not pruning the seemingly unused features, 
# and to make sure that the static initialization based registration of the features into the factory is done properly
# see https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/Object-Library
# and https://cmake.org/cmake/help/v3.15/command/add_library.html?highlight=add_library#object-libraries
add_library( ${PROJECT_NAME} OBJECT
 	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries ( ${PROJECT_NAME}
    cafProjectDataModel
    LibCore
)

#  Before cmake 3.12 OBJECT libraries could not use the target_link_libraries command, 
#  So we need to set the POSITION_INDEPENDENT_CODE option  manually

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Not to be used until we can use cmake 3.12 or above 
#target_link_libraries ( ${PROJECT_NAME}
#    cafCommand
#	cafUserInterface
#        ${QT_LIBRARIES}
#)


source_group("" FILES ${PROJECT_FILES})

# cotire
if (COMMAND caf_apply_cotire)
  caf_apply_cotire("${PROJECT_NAME}")
endif()