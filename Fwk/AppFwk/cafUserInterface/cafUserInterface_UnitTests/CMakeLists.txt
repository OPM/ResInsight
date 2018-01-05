cmake_minimum_required (VERSION 2.8)

find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
include (${QT_USE_FILE})

project ( cafUserInterface_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..

    ${cafProjectDataModel_SOURCE_DIR}
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
    
    ${cafUserInterface_SOURCE_DIR}
   
)


set( PROJECT_FILES

	cafUserInterface_UnitTests.cpp 
	
	cafPdmUiTreeViewModelTest.cpp 
	cafPdmUiTreeSelectionQModelTest.cpp 

	gtest/gtest-all.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

message(STATUS ${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})

target_link_libraries ( ${PROJECT_NAME}
	cafProjectDataModel
    cafPdmUiCore
    cafPdmCore
    cafPdmXml
    cafUserInterface
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore QtGui)
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)

