cmake_minimum_required (VERSION 2.8)

project (CommonCode)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafMessagePanel.h
    )

qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )

add_library( ${PROJECT_NAME}
    cafEffectCache.cpp
    cafEffectGenerator.cpp
    cafLog.cpp
    cafMessagePanel.cpp
    cafMouseState.cpp
    cafUtils.cpp
    cvfStructGrid.cpp
    
    cvfStructGridGeometryGenerator.cpp

    ${MOC_FILES_CPP}
)
