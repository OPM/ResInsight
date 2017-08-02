cmake_minimum_required (VERSION 2.8)

project (cafVizExtensions)

# Qt
option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core)
else()
  find_package(Qt4 COMPONENTS QtCore)
  include(${QT_USE_FILE})
endif()

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
if(USE_QT5)
  qt5_use_modules(${PROJECT_NAME} Core)
endif()
