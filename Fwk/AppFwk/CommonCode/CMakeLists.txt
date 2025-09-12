project(CommonCode)

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

add_library(
  ${PROJECT_NAME}
  cafEffectCache.cpp
  cafEffectCache.h
  cafEffectGenerator.cpp
  cafEffectGenerator.h
  cafLog.cpp
  cafLog.h
  cafMessagePanel.cpp
  cafMessagePanel.h
  cafMouseState.cpp
  cafMouseState.h
  cafUtils.cpp
  cafUtils.h
  cvfStructGrid.cpp
  cvfStructGrid.h
  cvfCellRange.cpp
  cvfCellRange.h
  cafColorTable.cpp
  cafColorTable.h
  cafContourLines.cpp
  cafContourLines.h
  cvfStructGridGeometryGenerator.cpp
  cvfStructGridGeometryGenerator.h
  cvfGeometryGeneratorInterface.cpp
  cvfGeometryGeneratorInterface.h
  cvfCylindricalGeometryGenerator.cpp
  cvfCylindricalGeometryGenerator.h
  cvfSingleCellMeshFactory.cpp
  cvfSingleCellMeshFactory.h
  cvfStructGridTools.cpp
  cvfStructGridTools.h
  cvfGeometryGeneratorFactory.cpp
  cvfGeometryGeneratorFactory.h
  cvfStructGridScalarDataAccess.h
  cafVecIjk.cpp
  cafVecIjk.h
  ${MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  LibCore
  LibGeometry
  LibViewing
  LibRender
  LibGuiQt
  cafPdmCore
  ${QT_LIBRARIES}
)

if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W4 /wd4100 /wd4127"
  )
endif()
