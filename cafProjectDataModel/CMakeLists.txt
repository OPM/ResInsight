cmake_minimum_required (VERSION 2.8)

project (cafProjectDataModel)

add_library( ${PROJECT_NAME}
    cafPdmObject.cpp
    cafPdmDocument.cpp
    cafPdmField.cpp
    cafPdmUiItem.cpp
	cafPdmPointer.cpp
)
