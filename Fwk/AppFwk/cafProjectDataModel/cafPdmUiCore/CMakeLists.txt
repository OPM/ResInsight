cmake_minimum_required (VERSION 2.8)

project (cafPdmUiCore)		

# Qt
find_package ( Qt4 COMPONENTS QtCore QtGui )
include (${QT_USE_FILE})

include_directories (
    ..
)

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    
    cafPdmUiFieldEditorHandle.h
	cafPdmUiSelection3dEditorVisualizer.h
)

if ( NOT CMAKE_AUTOMOC )
    qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )
endif()

set( PROJECT_FILES

    cafInternalPdmFieldTypeSpecializations.h
    cafInternalPdmUiFieldCapability.h
    cafInternalPdmUiFieldCapability.inl
    cafInternalPdmUiCommandSystemInterface.h

    cafPdmUiEditorHandle.cpp
    cafPdmUiEditorHandle.h
    cafPdmUiFieldEditorHandle.cpp
    cafPdmUiFieldEditorHandle.h
    cafPdmUiFieldHandle.cpp
    cafPdmUiFieldHandle.h
    cafPdmUiFieldSpecialization.h
    cafPdmUiItem.cpp
    cafPdmUiItem.h
    cafPdmUiModelChangeDetector.cpp
    cafPdmUiModelChangeDetector.h
    cafPdmUiObjectEditorHandle.cpp
    cafPdmUiObjectEditorHandle.h
    cafPdmUiWidgetObjectEditorHandle.cpp
    cafPdmUiWidgetObjectEditorHandle.h
    cafPdmUi3dObjectEditorHandle.cpp
    cafPdmUi3dObjectEditorHandle.h
    cafPdmUiObjectHandle.cpp
    cafPdmUiObjectHandle.h
    cafPdmUiOrdering.cpp
    cafPdmUiOrdering.h
    cafPdmUiCommandSystemProxy.cpp
    cafPdmUiCommandSystemProxy.h
    cafPdmUiTreeOrdering.cpp
    cafPdmUiTreeOrdering.h
    cafUiTreeItem.h
    cafPdmUiGroup.cpp
    cafPdmUiGroup.h

    cafSelectionManager.cpp
    cafSelectionManager.h
    cafSelectionChangedReceiver.h
    cafSelectionChangedReceiver.cpp
    cafSelectionManagerTools.h
	cafPdmUiSelection3dEditorVisualizer.h
	cafPdmUiSelection3dEditorVisualizer.cpp
)

add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
	${MOC_FILES_CPP}
)

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC) 
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127")
endif()

source_group("" FILES ${PROJECT_FILES})
