cmake_minimum_required (VERSION 2.8)

project ( WellPathImportSsihubTestApp )


find_package(Qt5 CONFIG COMPONENTS Core)
if (Qt5Core_FOUND)
	message(STATUS "Found Qt5")
	find_package(Qt5 CONFIG REQUIRED Core Gui OpenGl Network Script ScriptTools Widgets)
else()
    set (QT_COMPONENTS_REQUIRED QtCore QtGui QtMain QtOpenGl QtNetwork QtScript QtScriptTools)
	find_package(Qt4 COMPONENTS ${QT_COMPONENTS_REQUIRED} REQUIRED)
    include(${QT_USE_FILE})
	if (QT4_FOUND)
		message(STATUS "Found Qt4")
	endif(QT4_FOUND)
endif(Qt5Core_FOUND)


# Open GL
find_package( OpenGL )



add_subdirectory(../WellPathImportSsihub "${CMAKE_CURRENT_BINARY_DIR}/WellPathImportSsihub")
add_subdirectory(../../Fwk/AppFwk/cafProjectDataModel "${CMAKE_CURRENT_BINARY_DIR}/cafProjectDataModel")
add_subdirectory(../../Fwk/AppFwk/cafUserInterface "${CMAKE_CURRENT_BINARY_DIR}/cafUserInterface")

#add_subdirectory(../../cafTests/cafTestApplication "${CMAKE_CURRENT_BINARY_DIR}/cafTestApplication")

include_directories(
    ${WellPathImportSsihub_SOURCE_DIR}
	${WellPathImportSsihub_SOURCE_DIR}/../../Fwk/AppFwk/cafProjectDataModel 
	${WellPathImportSsihub_SOURCE_DIR}/../../Fwk/AppFwk/cafUserInterface
)


# Qt MOC
set ( QT_MOC_HEADERS 
)

if (Qt5Core_FOUND)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
else()
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif()

# Resource file
set( QRC_FILES  
)

# Runs RCC on specified files
if (Qt5Core_FOUND)
	qt5_add_resources( QRC_FILES_CPP
	    ${QRC_FILES}
	)
else()
	qt4_add_resources( QRC_FILES_CPP
	    ${QRC_FILES}
	)
endif()


# add the executable 
add_executable ( ${PROJECT_NAME}
    main.cpp
    TestTools.cpp
    TestTools.h
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
)

if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGl Qt5::Network Qt5::Script Qt5::ScriptTools Qt5::Widgets)
endif()

target_link_libraries ( ${PROJECT_NAME}

    cafProjectDataModel
    cafUserInterface

    WellPathImportSsihub
    ${QT_LIBRARIES}
)

# Copy Qt Dlls
if (Qt5Core_FOUND)
    foreach (qtlib ${QT_LIBRARIES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
	endforeach(qtlib)
	# Copy Qt Dlls	
else()
	# Copy Qt Dlls
	if (MSVC)
	    foreach (qtlib ${QT_COMPONENTS_REQUIRED})
	        
	        # Debug
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_BINARY_DIR}/Debug/${qtlib}d4.dll)
	        
	        # Release
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_BINARY_DIR}/Release/${qtlib}4.dll)
	    endforeach( qtlib )
	endif(MSVC)
endif(Qt5Core_FOUND)
