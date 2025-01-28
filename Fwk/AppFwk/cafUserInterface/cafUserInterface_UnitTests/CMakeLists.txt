cmake_minimum_required(VERSION 3.15)

project(cafUserInterface_UnitTests)

# Qt
find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets Svg
)
qt_standard_project_setup()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set(PROJECT_FILES cafUserInterface_UnitTests.cpp cafPdmUiTreeViewModelTest.cpp
                  cafPdmUiTreeSelectionQModelTest.cpp gtest/gtest-all.cpp
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafUserInterface ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

# Install
install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
  TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR NO_TRANSLATIONS
)
install(SCRIPT ${deploy_script})
