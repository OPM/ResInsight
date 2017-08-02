cmake_minimum_required (VERSION 2.8)

project (cafPdmCvf)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui)
else()
  find_package ( Qt4 COMPONENTS QtCore QtGui QtMain )
  include (${QT_USE_FILE})
endif()

include_directories(
    ${LibCore_SOURCE_DIR}
    ${cafProjectDataModel_SOURCE_DIR}
    ${cafUserInterface_SOURCE_DIR}

    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
)


add_library( ${PROJECT_NAME}
    cafPdmCoreColor3f.h
    cafPdmUiCoreColor3f.cpp
    cafPdmUiCoreColor3f.h
    cafPdmXmlColor3f.cpp
    cafPdmXmlColor3f.h
    cafPdmFieldCvfColor.h

    cafPdmCoreVec3d.h
    cafPdmUiCoreVec3d.cpp
    cafPdmUiCoreVec3d.h
    cafPdmXmlVec3d.cpp
    cafPdmXmlVec3d.h
    cafPdmFieldCvfVec3d.h

    cafPdmCoreMat4d.h
    # cafPdmUiCoreVec3d.cpp no special editor for matrix is created yet
    cafPdmUiCoreMat4d.h
    cafPdmXmlMat4d.cpp
    cafPdmXmlMat4d.h
    cafPdmFieldCvfMat4d.h
)
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core Gui)
endif()

target_link_libraries ( ${PROJECT_NAME}
    cafPdmCore
    cafPdmUiCore
    cafUserInterface
    cafProjectDataModel
    LibCore
    ${QT_LIBRARIES}
)
