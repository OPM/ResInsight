cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

add_library( ${PROJECT_NAME}
    cafPdmObject.cpp
    cafPdmDocument.cpp
    cafPdmField.cpp
	cafPdmPointer.cpp
	cafPdmUiItem.cpp
	cafPdmUiOrdering.cpp
	cafPdmUiEditorHandle.cpp
	cafPdmUiFieldEditorHandle.cpp
	cafPdmUiObjectEditorHandle.cpp
)
