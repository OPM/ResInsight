cmake_minimum_required(VERSION 3.15)

project(cafPdmCore_UnitTests)

# Qt
find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Widgets Gui
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
    cafPdmCore_UnitTests.cpp
    gtest/gtest-all.cpp
    cafPdmCoreBasicTest.cpp
    cafPdmReferenceHelperTest.cpp
    cafPdmChildArrayFieldHandleTest.cpp
    cafSignalTest.cpp
    cafPdmLoggingTest.cpp
    Child.cpp
    Child.h
    Parent.cpp
    Parent.h
    TestObj.cpp
    TestObj.h
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_FILES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  target_compile_options(
    cafPdmCore_UnitTests PRIVATE -Wno-ambiguous-reversed-operator
                                 -Wno-invalid-source-encoding
  )
endif()

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafPdmCore ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

# Copy Qt Dlls
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
