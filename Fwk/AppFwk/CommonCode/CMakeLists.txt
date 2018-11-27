cmake_minimum_required (VERSION 2.8.12)

project (CommonCode)

# Qt
find_package(Qt5Core CONFIG QUIET)
if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Gui OpenGL Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)

# Open GL
find_package( OpenGL )

# These headers need to go through Qt's MOC compiler
set( MOC_HEADER_FILES    
    cafMessagePanel.h
    )

if ( NOT CMAKE_AUTOMOC )
	if (Qt5Core_FOUND)
		qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	else()
		qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
	endif()
endif()


add_library( ${PROJECT_NAME}
	cafEffectCache.cpp
	cafEffectCache.h
	cafEffectGenerator.cpp
	cafEffectGenerator.h
	cafLog.cpp
	cafLog.h
	cafMessagePanel.cpp
	cafMessagePanel.h
	cafMouseState.cpp
	cafMouseState.h
	cafUtils.cpp
	cafUtils.h
	cvfStructGrid.cpp
	cvfStructGrid.h
	cvfCellRange.cpp
	cvfCellRange.h
	cafColorTable.cpp
	cafColorTable.h
	cafContourLines.cpp
	cafContourLines.h
	
	cvfStructGridGeometryGenerator.cpp
	cvfStructGridGeometryGenerator.h
	cvfStructGridScalarDataAccess.h
    cafVecIjk.cpp
    cafVecIjk.h

    ${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
endif()

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibViewing
    LibRender
    LibGuiQt

    cafPdmCore

    ${QT_LIBRARIES}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif() 

source_group("" FILES ${PROJECT_FILES})
