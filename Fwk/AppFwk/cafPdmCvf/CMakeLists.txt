cmake_minimum_required (VERSION 2.8.12)

project (cafPdmCvf)

# Qt
if (CAF_USE_QT5)
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain REQUIRED)
    include(${QT_USE_FILE})
endif(CAF_USE_QT5)


# These headers need to go through Qt's MOC compiler
set (MOC_HEADER_FILES    
    cafPdmUiVec3dEditor.h
)

# Run MOC on the headers
add_definitions(-DCVF_USING_CMAKE)
if (CAF_USE_QT5)
	qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
else()
	qt4_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES} )
endif(CAF_USE_QT5)

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
	cafPdmUiVec3dEditor.cpp

	${MOC_HEADER_FILES}
	${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (CAF_USE_QT5)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif(CAF_USE_QT5)


target_link_libraries ( ${PROJECT_NAME}
    cafUserInterface
    LibCore
    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})
