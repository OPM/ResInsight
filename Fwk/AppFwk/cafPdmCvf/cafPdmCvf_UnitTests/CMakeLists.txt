find_package(
  Qt6
  COMPONENTS
  REQUIRED Core
)
set(QT_LIBRARIES Qt6::Core)

project(cafPdmCvf_UnitTests)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} ${cafPdmCore_SOURCE_DIR} ${cafPdmXml_SOURCE_DIR}
  ${cafPdmCvf_SOURCE_DIR} ${LibCore_SOURCE_DIR}
)

set(PROJECT_FILES
    cafPdmCvf_UnitTests.cpp gtest/gtest-all.cpp cafPdmCoreVec3dTest.cpp
    cafPdmCoreColor3fTest.cpp cafPdmCoreMat4dTest.cpp
)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} cafPdmCore cafPdmXml LibCore cafPdmCvf ${QT_LIBRARIES}
)
