cmake_minimum_required(VERSION 2.8)

project(cafTestCvfApplication)

# Qt MOC
set(MOC_HEADER_FILES MainWindow.h WidgetLayoutTest.h)

# Resource file
set(QRC_FILES textedit.qrc)

find_package(
  Qt5
  COMPONENTS
  REQUIRED Core Gui Widgets OpenGL)
set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADER_FILES})
qt5_add_resources(QRC_FILES_CPP ${QRC_FILES})

option(USE_COMMAND_FRAMEWORK "Use Caf Command Framework" ON)

include_directories(
  ${LibCore_SOURCE_DIR}
  ${LibGeometry_SOURCE_DIR}
  ${LibGuiQt_SOURCE_DIR}
  ${LibRender_SOURCE_DIR}
  ${LibViewing_SOURCE_DIR}
  ${cafProjectDataModel_SOURCE_DIR}
  ${cafUserInterface_SOURCE_DIR})

if(USE_COMMAND_FRAMEWORK)
  include_directories(${cafCommand_SOURCE_DIR})
  add_definitions(-DTAP_USE_COMMAND_FRAMEWORK)
endif(USE_COMMAND_FRAMEWORK)

include_directories(${cafPdmCore_SOURCE_DIR} ${cafPdmUiCore_SOURCE_DIR}
                    ${cafPdmXml_SOURCE_DIR} ${cafPdmCvf_SOURCE_DIR})

set(PROJECT_FILES Main.cpp MainWindow.cpp WidgetLayoutTest.cpp
                  TapCvfSpecialization.cpp TapProject.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES} ${MOC_SOURCE_FILES}
                               ${QRC_FILES_CPP})

set(TAP_LINK_LIBRARIES cafUserInterface cafPdmXml cafPdmCvf ${QT_LIBRARIES})

if(USE_COMMAND_FRAMEWORK)
  set(TAP_LINK_LIBRARIES ${TAP_LINK_LIBRARIES} cafCommand)
endif(USE_COMMAND_FRAMEWORK)

target_link_libraries(${PROJECT_NAME} ${TAP_LINK_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})

foreach(qtlib ${QT_LIBRARIES})
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach(qtlib)
