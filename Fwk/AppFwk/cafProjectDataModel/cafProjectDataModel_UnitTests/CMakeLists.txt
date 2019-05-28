cmake_minimum_required (VERSION 2.8.12)

project ( cafProjectDataModel_UnitTests )

# Qt
option(CEE_USE_QT5 "Use Qt5" OFF)

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Xml Gui)
	set(QT_LIBRARIES Qt5::Core Qt5::Xml Qt5::Gui)
else()
    find_package(Qt4 COMPONENTS REQUIRED QtCore QtXml QtGui)
    include(${QT_USE_FILE})
endif(CEE_USE_QT5)


if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(STATUS "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

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

target_link_libraries ( ${PROJECT_NAME}
	cafProjectDataModel 
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

source_group("" FILES ${PROJECT_FILES})

# Copy Qt Dlls
if (CEE_USE_QT5)
    foreach (qtlib ${QT_LIBRARIES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
	endforeach(qtlib)
	# Copy Qt Dlls	
else()
	# Copy Qt Dlls
	if (MSVC)
	    set (QTLIBLIST QtCore QtXml)
	    foreach (qtlib ${QTLIBLIST})
	        
	        # Debug
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_BINARY_DIR}/Debug/${qtlib}d4.dll)
	        
	        # Release
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_BINARY_DIR}/Release/${qtlib}4.dll)
	    endforeach( qtlib )
	endif(MSVC)
endif(CEE_USE_QT5)