cmake_minimum_required (VERSION 2.8.12)

project (cafVizExtensions)

# Open GL
find_package( OpenGL )

if (RESINSIGHT_BUILD_WITH_QT5)
	find_package(Qt5 COMPONENTS Core QUIET)
endif(RESINSIGHT_BUILD_WITH_QT5)

if (Qt5Core_FOUND)
	find_package(Qt5 CONFIG REQUIRED Core Gui OpenGL Widgets)
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
    include(${QT_USE_FILE})
endif(Qt5Core_FOUND)

add_library( ${PROJECT_NAME}
    cafDisplayCoordTransform.cpp
    cafDisplayCoordTransform.h
    cafBoxManipulatorPartManager.cpp
    cafBoxManipulatorPartManager.h
    cafBoxManipulatorGeometryGenerator.cpp
    cafBoxManipulatorGeometryGenerator.h
	cafTitledOverlayFrame.cpp
	cafTitledOverlayFrame.h
    cafCategoryLegend.cpp
    cafCategoryLegend.h
	cafOverlayScalarMapperLegend.h
	cafOverlayScalarMapperLegend.cpp
	cafOverlayScaleLegend.h
	cafOverlayScaleLegend.cpp
	cafInternalLegendRenderTools.h
 	cafInternalLegendRenderTools.cpp
    cafCategoryMapper.cpp
    cafCategoryMapper.h
    cafFixedAtlasFont.h
    cafFixedAtlasFont.cpp
	cafTransparentWBRenderConfiguration.h
	cafTransparentWBRenderConfiguration.cpp
	cafLine.h
	cafLine.inl
	TranspWB_CombinationFrag.glsl
	TranspWB_PartlyTranspPartsFrag.glsl
	TranspWB_TransparentPartsFrag.glsl
  
    cafHexGridIntersectionTools/cafHexGridIntersectionTools.h
    cafHexGridIntersectionTools/cafHexGridIntersectionTools.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


if (Qt5Core_FOUND)
	set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
endif()

target_link_libraries ( ${PROJECT_NAME}
    LibCore
    LibGeometry
    LibViewing
    LibRender
    LibGuiQt

    CommonCode

    ${QT_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})
