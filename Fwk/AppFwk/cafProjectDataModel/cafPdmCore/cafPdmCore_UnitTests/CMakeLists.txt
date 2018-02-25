cmake_minimum_required (VERSION 2.8)

find_package ( Qt4 COMPONENTS QtCore QtGui  )
include (${QT_USE_FILE})

project ( cafPdmCore_UnitTests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)


set( PROJECT_FILES

	cafPdmCore_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmCoreBasicTest.cpp
    cafPdmReferenceHelperTest.cpp
    cafPdmChildArrayFieldHandleTest.cpp

	Child.cpp
	Child.h
	Parent.cpp
	Parent.h
	TestObj.cpp
	TestObj.h
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

message(STATUS ${PROJECT_NAME}" - Qt includes : "  ${QT_LIBRARIES})

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
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
