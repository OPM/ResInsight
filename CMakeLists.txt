cmake_minimum_required (VERSION 2.8)

project (ResInsight)

set (VIZ_MODULES_FOLDER_NAME VisualizationModules)



# Setup the main platform defines
#-----------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DCVF_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DCVF_OSX)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        # Linux specific code
        set(CMAKE_CXX_FLAGS 		"-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat")
        set(CMAKE_CXX_FLAGS_DEBUG 	"-ggdb -g3 -O0 -DDEBUG -D_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
        #set(CMAKE_EXE_LINKER_FLAGS  "-Xlinker -rpath .")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
#        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g1")
ELSE()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
ENDIF() 


################################################################################
# Version number
################################################################################
include (ResInsightVersion.cmake)


################################################################################
# ERT
################################################################################

# Disable install of ERT libs and headers, as Ert code is compiled and linked directly
# into ResInsight
SET(INSTALL_ERT OFF CACHE BOOL "Build ERT without installing")

add_subdirectory(ThirdParty/Ert/devel)

include_directories(
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libecl/include/ert/ecl
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libert_util/include/ert/util
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libgeometry/include/ert/geometry
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libecl_well/include/ert/ecl_well
	${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libecl/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libert_util/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libgeometry/include
    ${CMAKE_SOURCE_DIR}/ThirdParty/Ert/devel/libecl_well/include

	${CMAKE_BINARY_DIR}/ThirdParty/Ert/devel/libert_util/include/ert/util
    ${CMAKE_BINARY_DIR}/ThirdParty/Ert/devel/libert_util/include
)


################################################################################
# Qt
################################################################################
set (QT_COMPONENTS_REQUIRED QtCore QtGui QtMain QtOpenGl QtNetwork)
find_package (Qt4 COMPONENTS ${QT_COMPONENTS_REQUIRED} REQUIRED)
include (${QT_USE_FILE})

# Open GL
find_package( OpenGL )

################################################################################
# CeeViz
################################################################################
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibCore)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGeometry)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibRender)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibViewing)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGuiQt)

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}
)


################################################################################
# Ceetron Application Framework
################################################################################

add_subdirectory(cafProjectDataModel)
add_subdirectory(CommonCode)
add_subdirectory(cafAnimControl)
add_subdirectory(cafViewer)
add_subdirectory(cafUserInterface)
add_subdirectory(cafPdmCvf)

include_directories(
    ${cafUserInterface_SOURCE_DIR}
    ${cafProjectDataModel_SOURCE_DIR}
    ${cafPdmCvf_SOURCE_DIR}
    ${CommonCode_SOURCE_DIR}
	${cafAnimControl_SOURCE_DIR}
	${cafViewer_SOURCE_DIR}
)

################################################################################
# Installation settings
################################################################################

set (RESINSIGHT_FINAL_NAME "ResInsight-${STRPRODUCTVER}")

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
string(REGEX MATCH "el[5,6]?" RESINSIGHT_PLATFORM ${CMAKE_SYSTEM})
if (NOT "${RESINSIGHT_PLATFORM}" STREQUAL "")
    set (RESINSIGHT_FINAL_NAME "${RESINSIGHT_FINAL_NAME}-${RESINSIGHT_PLATFORM}")
endif()

# override system install prefix if private installation chosen
option (RESINSIGHT_PRIVATE_INSTALL "Install in a private directory" ON)
if (RESINSIGHT_PRIVATE_INSTALL)
    set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/Install/)
    #set (CMAKE_INSTALL_PREFIX /usr/${RESINSIGHT_FINAL_NAME})
endif (RESINSIGHT_PRIVATE_INSTALL)

################################################################################
# Application 
################################################################################
add_subdirectory(ApplicationCode)
add_subdirectory(OctavePlugin)

#

################################################################################
# Unit tests
################################################################################
add_subdirectory(ApplicationCode/ReservoirDataModel/ReservoirDataModel_UnitTests)
add_subdirectory(ApplicationCode/FileInterface/FileInterface_UnitTests)
add_subdirectory(ApplicationCode/ModelVisualization/ModelVisualization_UnitTests)




################################################################################
# Code documentation using Doxygen
################################################################################
option(RESINSIGHT_BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
if(RESINSIGHT_BUILD_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR 
      "Doxygen is needed to build the documentation. Please install it correctly")
  endif()
  #-- Configure the Template Doxyfile for our specific project
  configure_file(Doxyfile.in 
                 ${PROJECT_BINARY_DIR}/Doxyfile  @ONLY IMMEDIATE)
  #-- Add a custom target to run Doxygen when ever the project is built
  add_custom_target (Docs ALL 
  					COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
  					SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
  # IF you do NOT want the documentation to be generated EVERY time you build the project
  # then leave out the 'ALL' keyword from the above command.
endif(RESINSIGHT_BUILD_DOCUMENTATION)

################################################################################
# Installation packaging
################################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR TGZ)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CPACK_GENERATOR ZIP)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PATCH_VERSION})

set(CPACK_PACKAGE_NAME "ResInsight-bin")

if (NOT "${RESINSIGHT_PLATFORM}" STREQUAL "")
    set (CPACK_SYSTEM_NAME "${RESINSIGHT_PLATFORM}")
endif()

include (CPack)
