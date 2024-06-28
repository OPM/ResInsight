cmake_minimum_required(VERSION 3.15)

find_program(buildcache_program buildcache)
if(buildcache_program)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${buildcache_program}")
  message( STATUS "Found buildcache from : ${buildcache_program}" )
else()
  message( STATUS "No buildcache found" )
endif()

set(CMAKE_CXX_STANDARD 20)

project (TestCafAndVizFwk)

# ##############################################################################
# Setup the main platform defines
# ##############################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DCVF_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DCVF_OSX)
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(_HAS_STD_BYTE 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS
      "-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat -Wno-unused-parameter"
  )
  set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -g3 -O0 -DDEBUG -D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
endif()

if(CEE_USE_QT6)
  find_package(
    Qt6
    COMPONENTS
    REQUIRED Core Gui OpenGL Widgets
  )
  set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::Widgets)
  qt_standard_project_setup()
else()
  find_package(
    Qt5
    COMPONENTS
    REQUIRED Core Gui OpenGL Widgets
  )
  set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)
endif()

# Libraries
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(AppFwk/cafProjectDataModel)
add_subdirectory(AppFwk/cafCommand)
add_subdirectory(AppFwk/cafCommandFeatures)
add_subdirectory(AppFwk/cafUserInterface)
add_subdirectory(AppFwk/cafPdmScripting)
add_subdirectory(AppFwk/cafDataLoader)

# Unit Tests
add_subdirectory(AppFwk/cafProjectDataModel/cafProjectDataModel_UnitTests)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
add_subdirectory(AppFwk/cafPdmScripting/cafPdmScripting_UnitTests)
add_subdirectory(AppFwk/cafUserInterface/cafUserInterface_UnitTests)

# Executables
add_subdirectory(AppFwk/cafTests/cafTestApplication)


# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET cafPdmCore cafPdmUiCore cafPdmXml cafProjectDataModel PROPERTY FOLDER "ProjectDataModel")
set_property(TARGET cafPdmCore_UnitTests cafPdmXml_UnitTests cafProjectDataModel_UnitTests cafPdmScripting_UnitTests cafUserInterface_UnitTests PROPERTY FOLDER "UnitTests")

option (USE_CEE_VIZ "Enable visualization projects" ON)
if (USE_CEE_VIZ)
    add_subdirectory(VizFwk/LibCore)
    add_subdirectory(AppFwk/cafPdmCvf)

    add_subdirectory(AppFwk/cafPdmCvf/cafPdmCvf_UnitTests)

    add_subdirectory(AppFwk/cafTests/cafTestCvfApplication)

    set_property(TARGET LibCore cafPdmCvf cafPdmCvf_UnitTests cafTestCvfApplication PROPERTY FOLDER "CeeViz")
endif()
