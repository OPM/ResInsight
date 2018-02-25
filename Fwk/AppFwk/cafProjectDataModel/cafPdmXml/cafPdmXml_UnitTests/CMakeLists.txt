cmake_minimum_required (VERSION 2.8)

find_package ( Qt4 COMPONENTS QtCore  )
include (${QT_USE_FILE})

project ( cafPdmXml_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmXml_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmXmlBasicTest.cpp
	cafPdmAdvancedTemplateTest.cpp
	cafPdmXmlNumberTest.cpp
)
message(STATUS ${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})

target_link_libraries ( ${PROJECT_NAME}
	cafPdmXml
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore )
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)
