cmake_minimum_required (VERSION 2.8)

project (TestCafAndVizFwk)

option(USE_QT5 "Use version 5.x of the Qt library" OFF)
if(USE_QT5)
  find_package(Qt5 COMPONENTS Core Gui OpenGL REQUIRED)
else()
  find_package (Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
  include (${QT_USE_FILE})
endif()

# Libraries
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(AppFwk/cafProjectDataModel)
add_subdirectory(AppFwk/cafCommand)
add_subdirectory(AppFwk/cafUserInterface)

# Unit Tests
add_subdirectory(AppFwk/cafProjectDataModel/cafProjectDataModel_UnitTests)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests)
add_subdirectory(AppFwk/cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)

# Executables
add_subdirectory(AppFwk/cafTests/cafTestApplication)


# Organize sub-projects into folders on Visual Studio
# Turn on using solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_property(TARGET cafPdmCore cafPdmUiCore cafPdmXml cafProjectDataModel PROPERTY FOLDER "ProjectDataModel")
set_property(TARGET cafPdmCore_UnitTests cafPdmXml_UnitTests cafProjectDataModel_UnitTests PROPERTY FOLDER "UnitTests")

option (USE_CEE_VIZ "Enable visualization projects" ON)
if (USE_CEE_VIZ)
    add_subdirectory(VizFwk/LibCore)
    add_subdirectory(AppFwk/cafPdmCvf)

    add_subdirectory(AppFwk/cafPdmCvf/cafPdmCvf_UnitTests)

    add_subdirectory(AppFwk/cafTests/cafTestCvfApplication)

    set_property(TARGET LibCore cafPdmCvf cafPdmCvf_UnitTests cafTestCvfApplication PROPERTY FOLDER "CeeViz")
endif()
