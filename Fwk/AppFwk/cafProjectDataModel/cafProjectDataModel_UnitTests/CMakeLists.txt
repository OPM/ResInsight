cmake_minimum_required (VERSION 2.8)

find_package ( Qt4 COMPONENTS QtCore QtGui )
include (${QT_USE_FILE})

project ( cafProjectDataModel_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set( PROJECT_FILES
	cafPdmBasicTest.cpp
	cafProjectDataModel_UnitTests.cpp
        Child.cpp
        Parent.cpp
        TestObj.cpp

)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
	gtest/gtest-all.cpp
)
message(${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})

target_link_libraries ( ${PROJECT_NAME}
	cafProjectDataModel 
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

source_group("" FILES ${PROJECT_FILES})

# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore QtGui )
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)
