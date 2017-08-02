cmake_minimum_required (VERSION 2.8)

option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core)
else()
  find_package ( Qt4 COMPONENTS QtCore  )
  include (${QT_USE_FILE})
endif()

project ( cafPdmXml_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${cafPdmCore_SOURCE_DIR}
)


# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmXml_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmXmlBasicTest.cpp
	cafPdmAdvancedTemplateTest.cpp
)
message(STATUS ${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core)
endif()

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
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
