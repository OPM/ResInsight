cmake_minimum_required (VERSION 2.8.12)

project ( cafTestApplication )

# Open GL
find_package( OpenGL )

# Qt
find_package(Qt5 CONFIG COMPONENTS Core)
if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Gui OpenGL Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)




option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

# Qt MOC
set (MOC_HEADER_FILES 
	MainWindow.h 
    WidgetLayoutTest.h
    ManyGroups.h
    CustomObjectEditor.h
    MenuItemProducer.h
)

if ( NOT CMAKE_AUTOMOC )
	if (Qt5Core_FOUND)
		qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	else()
		qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	endif()
endif()

# Resource file
set( QRC_FILES  
    textedit.qrc
)

# Runs RCC on specified files
if (Qt5Core_FOUND)
	qt5_add_resources( QRC_FILES_CPP ${QRC_FILES} )
else()
	qt4_add_resources( QRC_FILES_CPP ${QRC_FILES} )
endif()

if (USE_COMMAND_FRAMEWORK)
    include_directories (
        ${cafCommand_SOURCE_DIR}
    )
    ADD_DEFINITIONS(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)


set( PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    WidgetLayoutTest.cpp
    WidgetLayoutTest.h
    ManyGroups.cpp
    ManyGroups.h
    CustomObjectEditor.cpp
    CustomObjectEditor.h
    MenuItemProducer.cpp
    MenuItemProducer.h
)


# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
)

if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
endif()

set (TAP_LINK_LIBRARIES
    cafUserInterface
    )

if (USE_COMMAND_FRAMEWORK)
    set (TAP_LINK_LIBRARIES
        ${TAP_LINK_LIBRARIES}
        cafCommand
    )
endif(USE_COMMAND_FRAMEWORK)


target_link_libraries ( ${PROJECT_NAME}
    ${TAP_LINK_LIBRARIES}
    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

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
	    set (QTLIBLIST QtCore QtGui QtOpenGl )
	    foreach (qtlib ${QTLIBLIST})
	        
	        # Debug
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_BINARY_DIR}/Debug/${qtlib}d4.dll)
	        
	        # Release
	        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_BINARY_DIR}/Release/${qtlib}4.dll)
	    endforeach( qtlib )
	endif(MSVC)
endif(Qt5Core_FOUND)