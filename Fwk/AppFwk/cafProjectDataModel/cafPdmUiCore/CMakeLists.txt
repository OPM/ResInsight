cmake_minimum_required (VERSION 2.8)

project (cafPdmUiCore)		

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui Widgets)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui )
  include (${QT_USE_FILE})
endif()

include_directories (
	
    ${cafPdmCore_SOURCE_DIR}
    ..
)

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
    cafPdmUiObjectHandle.cpp
    cafPdmUiObjectHandle.h
    cafPdmUiOrdering.cpp
    cafPdmUiOrdering.h
    cafPdmUiCommandSystemProxy.cpp
    cafPdmUiCommandSystemProxy.h
    cafPdmUiTreeOrdering.cpp
    cafPdmUiTreeOrdering.h
    cafUiTreeItem.h

    cafSelectionManager.cpp
    cafSelectionManager.h


)


add_library( ${PROJECT_NAME}
	${PROJECT_FILES}
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui Widgets)
endif()

source_group("" FILES ${PROJECT_FILES})
