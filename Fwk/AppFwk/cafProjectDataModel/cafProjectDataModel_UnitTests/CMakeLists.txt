cmake_minimum_required (VERSION 2.8)

option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui )
  include (${QT_USE_FILE})
endif()

project ( cafProjectDataModel_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

include_directories (
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
    
    ${cafProjectDataModel_SOURCE_DIR}
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
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui)
endif()

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
    cafPdmUiCore
    cafPdmXml
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
