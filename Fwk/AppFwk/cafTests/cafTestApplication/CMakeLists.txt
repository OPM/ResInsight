cmake_minimum_required(VERSION 3.15)

project(cafTestApplication)

# Open GL
find_package(OpenGL)

# Qt

# Resource file
set(QRC_FILES ${QRC_FILES} textedit.qrc)
message("QRC_FILES: ${QRC_FILES}")

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL Svg
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::Svg)
qt_standard_project_setup()

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

if(USE_COMMAND_FRAMEWORK)
  include_directories(${cafCommand_SOURCE_DIR})
  add_definitions(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)

set(PROJECT_FILES
    Main.cpp
    MainWindow.cpp
    MainWindow.h
    WidgetLayoutTest.cpp
    WidgetLayoutTest.h
    ManyGroups.cpp
    ManyGroups.h
    CustomObjectEditor.cpp
    CustomObjectEditor.h
    MenuItemProducer.cpp
    MenuItemProducer.h
    TamComboBox.h
    TamComboBox.cpp
    LineEditAndPushButtons.h
    LineEditAndPushButtons.cpp
    LabelsAndHyperlinks.h
    LabelsAndHyperlinks.cpp
    ApplicationEnum.h
    ApplicationEnum.cpp
    OptionalFields.h
    OptionalFields.cpp
)

qt_add_executable(
  ${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES} ${QRC_FILES}
  $<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12. Remove
  # when we can use target_link_libraries with OBJECT libraries
)

set(TAP_LINK_LIBRARIES cafUserInterface)

if(USE_COMMAND_FRAMEWORK)
  set(TAP_LINK_LIBRARIES
      ${TAP_LINK_LIBRARIES} cafCommand
      # cafCommandFeatures # Not possible using cmake version < 3.12. Use when
      # we can use target_link_libraries with OBJECT libraries
  )
endif(USE_COMMAND_FRAMEWORK)

# According to ivarun this is needed on OpenSuse, and Fedora. See:
# https://github.com/OPM/ResInsight/pull/7
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND EXTERNAL_LINK_LIBRARIES rt pthread)
endif()

target_link_libraries(
  ${PROJECT_NAME} PRIVATE ${TAP_LINK_LIBRARIES} ${QT_LIBRARIES}
                          ${EXTERNAL_LINK_LIBRARIES}
)

foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endforeach(qtlib)

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CPACK_GENERATOR TGZ)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CPACK_GENERATOR ZIP)
endif()

include(CPack)
