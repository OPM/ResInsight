cmake_minimum_required (VERSION 2.8)

project ( cafProjectDataModel_UnitTests )

include_directories (
	${CMAKE_SOURCE_DIR}/cafProjectDataModel 
	${CMAKE_SOURCE_DIR}/cafTests

	#Remove when RigStatistics is out
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ModelVisualization
)

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmBasicTest.cpp
	cafProjectDataModel_UnitTests.cpp
	${CMAKE_SOURCE_DIR}/cafTests/gtest/gtest-all.cpp
)

target_link_libraries ( ${PROJECT_NAME}
	cafProjectDataModel 
	${QT_LIBRARIES} 
)


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
