cmake_minimum_required (VERSION 2.8)

project ( ssihubTestApplication )


set (QT_COMPONENTS_REQUIRED QtCore QtGui QtMain QtOpenGl QtNetwork QtScript QtScriptTools)
find_package (Qt4 COMPONENTS ${QT_COMPONENTS_REQUIRED} REQUIRED)
include (${QT_USE_FILE})

# Open GL
find_package( OpenGL )


add_subdirectory(../ssihubInterface "${CMAKE_CURRENT_BINARY_DIR}/ssihubInterface")

include_directories(
    ${ssihubInterface_SOURCE_DIR}
)


# Qt MOC
set ( QT_MOC_HEADERS 
)

qt4_wrap_cpp( MOC_FILES_CPP
    ${QT_MOC_HEADERS}
)

# Resource file
set( QRC_FILES  
)

# Runs RCC on specified files
qt4_add_resources( QRC_FILES_CPP
    ${QRC_FILES}
)


# add the executable 
add_executable ( ${PROJECT_NAME}
    main.cpp
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
)

target_link_libraries ( ${PROJECT_NAME}
    ssihubInterface
    ${QT_LIBRARIES}
)

# Copy Qt Dlls
if (MSVC)
    foreach (qtlib ${QT_COMPONENTS_REQUIRED})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)
