cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

add_library( ${PROJECT_NAME}
	cafAppEnum.h
	cafFactory.h
	cafFixedArray.h
	cafOmpMutex.h
	cafPdmDocument.cpp
	cafPdmDocument.h
	cafPdmField.cpp
	cafPdmField.h
	cafPdmField.inl
	cafPdmFieldImpl.h
	cafPdmObject.cpp
	cafPdmObject.h
	cafPdmObjectFactory.h
	cafPdmPointer.cpp
	cafPdmPointer.h
	cafPdmUiEditorHandle.cpp
	cafPdmUiEditorHandle.h
	cafPdmUiFieldEditorHandle.cpp
	cafPdmUiFieldEditorHandle.h
	cafPdmUiItem.cpp
	cafPdmUiItem.h
	cafPdmUiObjectEditorHandle.cpp
	cafPdmUiObjectEditorHandle.h
	cafPdmUiOrdering.cpp
	cafPdmUiOrdering.h
)
