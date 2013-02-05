cmake_minimum_required (VERSION 2.8)

project (cafPdmCvf)

include_directories(
    ${cafProjectDataModel_SOURCE_DIR}
    ${cafUserInterface_SOURCE_DIR}
)


add_library( ${PROJECT_NAME}
    cafPdmFieldCvfColor.cpp
	cafPdmFieldCvfMat4d.cpp
)
