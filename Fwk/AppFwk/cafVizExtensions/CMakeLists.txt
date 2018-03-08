cmake_minimum_required (VERSION 2.8.12)

project (cafVizExtensions)

# Open GL
find_package( OpenGL )

add_library( ${PROJECT_NAME}
    cafDisplayCoordTransform.cpp
    cafDisplayCoordTransform.h
    cafBoxManipulatorPartManager.cpp
    cafBoxManipulatorPartManager.h
    cafBoxManipulatorGeometryGenerator.cpp
    cafBoxManipulatorGeometryGenerator.h
    cafCategoryLegend.cpp
    cafCategoryLegend.h
	cafOverlayScalarMapperLegend.h
	cafOverlayScalarMapperLegend.cpp
	cafInternalLegendRenderTools.h
 	cafInternalLegendRenderTools.cpp
    cafCategoryMapper.cpp
    cafCategoryMapper.h
    cafFixedAtlasFont.h
    cafFixedAtlasFont.cpp
	cafTransparentWBRenderConfiguration.h
	cafTransparentWBRenderConfiguration.cpp
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
