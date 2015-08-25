cmake_minimum_required (VERSION 2.8)

project (cafVizExtensions)

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
	cafTransparentWBRenderConfiguration.h
	cafTransparentWBRenderConfiguration.cpp
	TranspWB_CombinationFrag.glsl
	TranspWB_PartlyTranspPartsFrag.glsl
	TranspWB_TransparentPartsFrag.glsl
)
