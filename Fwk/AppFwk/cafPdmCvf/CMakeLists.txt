cmake_minimum_required(VERSION 3.12)

project(cafPdmCvf)

# Unity Build
if(CAF_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

# Qt
find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})

add_definitions(-DCVF_USING_CMAKE)

add_library(
  ${PROJECT_NAME}
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
  cafPdmMat3d/cafPdmCoreMat3d.h
  cafPdmMat3d/cafPdmUiCoreMat3d.h
  cafPdmMat3d/cafPdmXmlMat3d.cpp
  cafPdmMat3d/cafPdmXmlMat3d.h
  cafPdmMat3d/cafPdmFieldCvfMat3d.h
  ${MOC_HEADER_FILES}
  ${MOC_SOURCE_FILES})

target_include_directories(
  ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                         ${CMAKE_CURRENT_SOURCE_DIR}/cafPdmMat3d)

target_link_libraries(${PROJECT_NAME} cafUserInterface LibCore ${QT_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})
