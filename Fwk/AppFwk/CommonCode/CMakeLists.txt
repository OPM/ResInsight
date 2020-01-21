cmake_minimum_required (VERSION 2.8.12)

project (CommonCode)

# Unity Build
if (COMMAND caf_apply_unity_build)
  caf_apply_unity_build("${PROJECT_NAME}")
endif()

# Open GL
find_package( OpenGL )

# Qt

# These headers need to go through Qt's MOC compiler
set( MOC_HEADER_FILES    
    cafMessagePanel.h
    )

if (CEE_USE_QT5)
	find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets OpenGL)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )	
else()
    find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
    include(${QT_USE_FILE})
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif(CEE_USE_QT5)

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

# cotire
if (COMMAND caf_apply_cotire)
  list(APPEND CAF_COTIRE_EXCLUDE_FILES
      cafEffectGenerator.cpp
  )
  caf_apply_cotire("${PROJECT_NAME}")
endif()
