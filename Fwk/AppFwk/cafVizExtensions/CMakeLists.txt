cmake_minimum_required (VERSION 2.8)

project (cafVizExtensions)

# Qt
find_package(Qt4 COMPONENTS QtCore)
include(${QT_USE_FILE})

# Open GL
find_package( OpenGL )

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}
	${CommonCode_SOURCE_DIR}
)

add_library( ${PROJECT_NAME}
    cafDisplayCoordTransform.cpp
    cafDisplayCoordTransform.h
    cafBoxManipulatorPartManager.cpp
    cafBoxManipulatorPartManager.h
    cafBoxManipulatorGeometryGenerator.cpp
    cafBoxManipulatorGeometryGenerator.h
    cafCategoryLegend.cpp
    cafCategoryLegend.h
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
