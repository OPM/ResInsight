cmake_minimum_required (VERSION 2.8)

project (CommonCode)

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl )
include (${QT_USE_FILE})

# Open GL
find_package( OpenGL )

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}

	${cafPdmCore_SOURCE_DIR}
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafMessagePanel.h
    )

qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )

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
	
	cvfStructGridGeometryGenerator.cpp
	cvfStructGridGeometryGenerator.h
	cvfStructGridScalarDataAccess.h
    cafVecIjk.cpp
    cafVecIjk.h

    ${MOC_FILES_CPP}
)
