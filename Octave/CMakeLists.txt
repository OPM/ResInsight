cmake_minimum_required(VERSION 3.15)

project(OctavePlugins)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Octave)

if(CEE_USE_QT6)
  find_package(
    Qt6
    COMPONENTS
    REQUIRED Core Gui OpenGL Widgets Network
  )
  set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::Widgets Qt6::Network)
  qt_standard_project_setup()
else()
  find_package(
    Qt5
    COMPONENTS
    REQUIRED Core Gui OpenGL Widgets Network
  )
  set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets Qt5::Network)
endif()

add_subdirectory(OctavePlugin)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CPACK_GENERATOR TGZ)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CPACK_GENERATOR ZIP)
endif()

include(CPack)
