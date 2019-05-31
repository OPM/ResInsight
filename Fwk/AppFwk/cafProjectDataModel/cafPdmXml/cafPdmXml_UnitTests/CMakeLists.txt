cmake_minimum_required (VERSION 2.8.12)

project ( cafPdmXml_UnitTests )

if (NOT DEFINED(CEE_USE_QT5))
	option(CEE_USE_QT5 "Use Qt5" OFF)
endif(NOT DEFINED(CEE_USE_QT5))

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Xml)
	set(QT_LIBRARIES Qt5::Core Qt5::Xml)
else()
    find_package(Qt4 COMPONENTS REQUIRED QtCore QtXml)
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

# add the executable 
add_executable (${PROJECT_NAME}
	cafPdmXml_UnitTests.cpp
	gtest/gtest-all.cpp

	cafPdmXmlBasicTest.cpp
	cafPdmAdvancedTemplateTest.cpp
	cafPdmXmlNumberTest.cpp
)

if (Qt5Core_FOUND)
    set(QT_LIBRARIES Qt5::Core Qt5::Xml)
endif()

target_link_libraries ( ${PROJECT_NAME}
	cafPdmXml
    ${QT_LIBRARIES}
    ${THREAD_LIBRARY}
)

source_group("" FILES ${PROJECT_FILES})

# Copy Qt Dlls
if (Qt5Core_FOUND)
    foreach (qtlib ${QT_LIBRARIES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
	endforeach(qtlib)
else()
	if (MSVC)
	    set (QTLIBLIST QtCore QtXml)
	    foreach (qtlib ${QTLIBLIST})
	        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,${qtlib}d4.dll,${qtlib}4.dll> $<TARGET_FILE_DIR:${PROJECT_NAME}>
			)
	    endforeach( qtlib )
	endif(MSVC)
endif(Qt5Core_FOUND)