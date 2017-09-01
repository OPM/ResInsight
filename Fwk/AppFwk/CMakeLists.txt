cmake_minimum_required (VERSION 2.8)

project (CeeApp)


find_package (Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
include (${QT_USE_FILE})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "-std=c++0x")
endif() 

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()


# Improvement Cmake 3.x
# If we have compiler requirements for this library, list them
#target_compile_features(lib
#    PUBLIC cxx_auto_type
#    PRIVATE cxx_variadic_templates)



#libraries
add_subdirectory (cafProjectDataModel/cafPdmCore)
add_subdirectory (cafProjectDataModel/cafPdmUiCore)
add_subdirectory (cafProjectDataModel/cafPdmXml)

add_subdirectory(cafProjectDataModel)
add_subdirectory(cafCommand)
add_subdirectory(cafUserInterface)

#executables
add_subdirectory(cafTests/cafTestApplication)

if( UNIX )
    set(THREAD_LIBRARY "pthread")
endif()

add_subdirectory (cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory (cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
add_subdirectory (cafProjectDataModel/cafProjectDataModel_UnitTests)

# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET  cafPdmCore cafPdmCore_UnitTests cafPdmXml cafPdmXml_UnitTests  cafPdmUiCore  PROPERTY FOLDER "PdmCore")

