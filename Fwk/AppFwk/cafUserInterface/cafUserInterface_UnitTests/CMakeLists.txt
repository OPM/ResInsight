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

set(PROJECT_FILES
    cafUserInterface_UnitTests.cpp
    cafPdmUiTreeViewModelTest.cpp
    cafPdmUiTreeViewEditorTest.cpp
    cafPdmUiTreeSelectionQModelTest.cpp
    cafPdmUiTableViewQModelTest.cpp
    cafPdmUiNumberFormatTest.cpp
    cafPdmUiAttributeToolsTest.cpp
    cafPdmUiEditorAttributesTest.cpp
    cafPdmUiCheckBoxAndTextEditorTest.cpp
    cafPdmUiPropertyViewDialogTest.cpp
    cafPdmUiEditorHandleTest.cpp
    cafPdmUiFormLayoutObjectEditorTest.cpp
    cafIconProviderTest.cpp
    cafProgressInfoTest.cpp
    gtest/gtest-all.cpp
)

qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafUserInterface ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

# Make the executable runnable from its own folder when built stand-alone
if(WIN32 AND TARGET Qt6::windeployqt)
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND Qt6::windeployqt --no-translations $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE_DIR:Qt6::Core>/../plugins/platforms/qoffscreen$<$<CONFIG:Debug>:d>.dll"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/"
  )
endif()

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
