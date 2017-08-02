cmake_minimum_required (VERSION 2.8)

project ( WellPathImportSsihubTestApp )
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  set(QT_COMPONENTS_REQUIRED Core Gui OpenGL Network Script ScriptTools)
  find_package(Qt5 COMPONENTS ${QT_COMPONENTS_REQUIRED})
else()
  set (QT_COMPONENTS_REQUIRED QtCore QtGui QtMain QtOpenGl QtNetwork QtScript QtScriptTools)
  find_package (Qt4 COMPONENTS ${QT_COMPONENTS_REQUIRED} REQUIRED)
  include (${QT_USE_FILE})
endif()

# Open GL
find_package( OpenGL )



add_subdirectory(../WellPathImportSsihub "${CMAKE_CURRENT_BINARY_DIR}/WellPathImportSsihub")
add_subdirectory(../../Fwk/AppFwk/cafProjectDataModel "${CMAKE_CURRENT_BINARY_DIR}/cafProjectDataModel")
add_subdirectory(../../Fwk/AppFwk/cafUserInterface "${CMAKE_CURRENT_BINARY_DIR}/cafUserInterface")

#add_subdirectory(../../cafTests/cafTestApplication "${CMAKE_CURRENT_BINARY_DIR}/cafTestApplication")

include_directories(
    ${WellPathImportSsihub_SOURCE_DIR}
	${WellPathImportSsihub_SOURCE_DIR}/../../Fwk/AppFwk/cafProjectDataModel 
	${WellPathImportSsihub_SOURCE_DIR}/../../Fwk/AppFwk/cafUserInterface
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
    TestTools.cpp
    TestTools.h
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} ${QT_COMPONENTS_REQUIRED})
endif()
target_link_libraries ( ${PROJECT_NAME}

    cafProjectDataModel
    cafUserInterface

    WellPathImportSsihub
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
