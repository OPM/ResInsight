cmake_minimum_required(VERSION 3.15)

project(cafPdmXml_UnitTests)

if(CEE_USE_QT6)
  find_package(
    Qt6
    COMPONENTS
    REQUIRED Core Xml
  )
  set(QT_LIBRARIES Qt6::Core Qt6::Xml)
  qt_standard_project_setup()
else()
  find_package(
    Qt5
    COMPONENTS
    REQUIRED Core Xml
  )
  set(QT_LIBRARIES Qt5::Core Qt5::Xml)
endif()

if(MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code
  # related to TR1
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set(PROJECT_FILES
    cafPdmXml_UnitTests.cpp gtest/gtest-all.cpp cafPdmXmlBasicTest.cpp
    cafPdmAdvancedTemplateTest.cpp cafPdmXmlNumberTest.cpp
    cafPdmPtrArrayTest.cpp
)

if(CEE_USE_QT6)
  qt_add_executable(${PROJECT_NAME} ${PROJECT_FILES})
else()
  add_executable(${PROJECT_NAME} ${PROJECT_FILES})
endif(CEE_USE_QT6)

source_group("" FILES ${PROJECT_FILES})

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafPdmXml ${QT_LIBRARIES} ${THREAD_LIBRARY}
)

# Copy Qt Dlls
if(Qt5Core_FOUND)
  foreach(qtlib ${QT_LIBRARIES})
    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}>
              $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
  endforeach(qtlib)
endif(Qt5Core_FOUND)

# Install
install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(CEE_USE_QT6)
  qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR NO_TRANSLATIONS
  )
  install(SCRIPT ${deploy_script})
endif(CEE_USE_QT6)
