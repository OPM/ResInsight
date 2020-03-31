cmake_minimum_required (VERSION 2.8.12)

project ( cafTestApplication )

# Open GL
find_package( OpenGL )

# Qt

# Qt MOC
set (MOC_HEADER_FILES 
	MainWindow.h 
    WidgetLayoutTest.h
    CustomObjectEditor.h
    MenuItemProducer.h
)

# Resource file
set( QRC_FILES  
    ${QRC_FILES} 
    textedit.qrc
)

if (NOT DEFINED(CEE_USE_QT5))
	option(CEE_USE_QT5 "Use Qt5" OFF)
endif(NOT DEFINED(CEE_USE_QT5))

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	qt5_add_resources( QRC_FILES_CPP ${QRC_FILES} )
else()
    find_package(Qt4 COMPONENTS REQUIRED QtCore QtGui QtMain QtOpenGl)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	qt4_add_resources( QRC_FILES_CPP ${QRC_FILES} )
endif(CEE_USE_QT5)

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

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

    TamComboBox.h
    TamComboBox.cpp
)


# add the executable 
add_executable ( ${PROJECT_NAME}
	${PROJECT_FILES}
    ${MOC_SOURCE_FILES}
    ${QRC_FILES_CPP}
    $<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12. Remove when we can use target_link_libraries with OBJECT libraries
)

set (TAP_LINK_LIBRARIES
    cafUserInterface
    )

if (USE_COMMAND_FRAMEWORK)
    set (TAP_LINK_LIBRARIES
        ${TAP_LINK_LIBRARIES}
        cafCommand
        #cafCommandFeatures # Not possible using cmake version < 3.12. Use when we can use target_link_libraries with OBJECT libraries
    )
endif(USE_COMMAND_FRAMEWORK)

# According to ivarun this is needed on OpenSuse, and Fedora. See: https://github.com/OPM/ResInsight/pull/7
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND EXTERNAL_LINK_LIBRARIES
        rt
        pthread
    )
endif()

target_link_libraries ( ${PROJECT_NAME}
    ${TAP_LINK_LIBRARIES}
    ${QT_LIBRARIES}
    ${EXTERNAL_LINK_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})

if (CEE_USE_QT5)
    foreach (qtlib ${QT_LIBRARIES})
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
	endforeach(qtlib)
else()
	# Copy Qt Dlls
	if (MSVC)
	    set (QTLIBLIST QtCore QtGui QtOpenGl QtXml)
	    foreach (qtlib ${QTLIBLIST})
	        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_BINARY_DIR}/$<IF:$<CONFIG:Debug>,${qtlib}d4.dll,${qtlib}4.dll> $<TARGET_FILE_DIR:${PROJECT_NAME}>
			)
	    endforeach( qtlib )
	endif(MSVC)
endif(CEE_USE_QT5)


# cotire
if (COMMAND caf_apply_cotire)
  set_source_files_properties (qrc_textedit.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
  set_source_files_properties (qrc_cafCommandFeatures.cpp PROPERTIES COTIRE_EXCLUDED TRUE)
  set_source_files_properties (qrc_cafAnimControl.cpp PROPERTIES COTIRE_EXCLUDED TRUE)

  caf_apply_cotire("${PROJECT_NAME}")
endif()
