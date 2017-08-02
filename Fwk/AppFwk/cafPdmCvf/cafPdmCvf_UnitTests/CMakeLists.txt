cmake_minimum_required (VERSION 2.8)

option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core)
else()
  find_package ( Qt4 COMPONENTS QtCore  )
  include (${QT_USE_FILE})
endif()

project ( cafPdmCvf_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${cafPdmCore_SOURCE_DIR}
	${cafPdmXml_SOURCE_DIR}
	${cafPdmCvf_SOURCE_DIR}
	
    ${LibCore_SOURCE_DIR}
)


set( PROJECT_FILES

	cafPdmCvf_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmCoreVec3dTest.cpp
	cafPdmCoreColor3fTest.cpp
    cafPdmCoreMat4dTest.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core)
endif()

source_group("" FILES ${PROJECT_FILES})

message(STATUS ${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
    cafPdmXml
    LibCore
    cafPdmCvf
    ${QT_LIBRARIES}
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
