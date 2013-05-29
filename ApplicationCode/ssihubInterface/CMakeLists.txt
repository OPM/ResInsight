cmake_minimum_required (VERSION 2.8)

project (ssihubInterface)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS   
    ssihubDialog.h 
)

if ( (${CMAKE_VERSION} VERSION_LESS 2.8.6) OR (NOT CMAKE_AUTOMOC) )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()


add_library( ${PROJECT_NAME}
    ssihubInterface.cpp
    ssihubWebServiceInterface.cpp
    ssihubDialog.cpp
    ${MOC_FILES_CPP}
)
