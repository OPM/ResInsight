cmake_minimum_required(VERSION 3.15)

project(cafProjectDataModel_UnitTests)

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Widgets
)
qt_standard_project_setup()

if(MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code
  # related to TR1
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set(PROJECT_FILES
    cafPdmBasicTest.cpp
    cafProjectDataModel_UnitTests.cpp
    cafUiTreeManagerTest.cpp
    Child.cpp
    Parent.cpp
    TestObj.cpp
    AggregatedTypesInField.cpp
    gtest/gtest-all.cpp
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_FILES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    cafProjectDataModel_UnitTests PRIVATE -Wno-ambiguous-reversed-operator
                                          -Wno-invalid-source-encoding
  )
endif()

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafProjectDataModel ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

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
