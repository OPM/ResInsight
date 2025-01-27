project(cafVizExtensions)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Open GL
find_package(OpenGL)

# Qt
find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL)
qt_standard_project_setup()
set_property(
  SOURCE cafTransparentWBRenderConfiguration.cpp PROPERTY SKIP_AUTOMOC ON
)

add_library(
  ${PROJECT_NAME}
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

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  LibCore
  LibGeometry
  LibViewing
  LibRender
  LibGuiQt
  CommonCode
  ${QT_LIBRARIES}
)


