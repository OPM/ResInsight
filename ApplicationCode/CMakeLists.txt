cmake_minimum_required (VERSION 2.8.12)

project (ApplicationCode)


# set packaging dir
if(NOT CPACK_PACKAGE_DIRECTORY)
    set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
endif()

if(RESINSIGHT_ENABLE_UNITY_BUILD)
    message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
    set(CMAKE_UNITY_BUILD true)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

# Open GL
find_package( OpenGL )

option(RESINSIGHT_ENABLE_GRPC "Enable the gRPC scripting framework" OFF)
option(RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE "Bundle the gRPC python modules into the install folder" OFF)

find_package(Qt5 COMPONENTS Core QUIET)

if (Qt5Core_FOUND)
    find_package(Qt5 CONFIG REQUIRED Core Gui OpenGL Network Script Widgets Xml Concurrent PrintSupport)
    set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Network Qt5::OpenGL Qt5::Script Qt5::Widgets Qt5::Xml Qt5::Concurrent Qt5::PrintSupport)
endif(Qt5Core_FOUND)

# NB: The generated file is written to Cmake binary folder to avoid source tree pollution
# This folder is added to include_directories
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/RiaVersionInfo.h
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(STATUS "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

include_directories(

    # required for compilation of gtest, should be refactored
    ${ResInsight_SOURCE_DIR}/ThirdParty
    
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Adm
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools/WellPathTools

    ${CMAKE_CURRENT_SOURCE_DIR}/Commands
    ${CMAKE_CURRENT_SOURCE_DIR}/Commands/EclipseCommands
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/Measurement
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/GridBox
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Intersections
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Surfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
	${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/AnalysisPlots
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface/Core

    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/AnalysisPlots
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/CorrelationPlots
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Annotations
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Completions
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Flow
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/GridCrossPlots
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Measurement
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Summary
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Surfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModelCommands
    
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultStatisticsCache
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/Completions
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/OdbReader
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechVisualization

    ${CMAKE_BINARY_DIR}/Generated
    ${CMAKE_CURRENT_BINARY_DIR}
)

################################################################################
# Eigen
################################################################################
include_directories(SYSTEM ${ResInsight_SOURCE_DIR}/ThirdParty/Eigen-Subset)

#############################################################################
# Defining all the source (and header) files
#############################################################################

# Use all h files in the subdirectories to make them available in the project
file( GLOB_RECURSE HEADER_FILES *.h )

set( SOCKET_INTERFACE_FILES
    SocketInterface/RiaSocketServer.cpp
    SocketInterface/RiaProjectInfoCommands.cpp
    SocketInterface/RiaCaseInfoCommands.cpp
    SocketInterface/RiaGeometryCommands.cpp
    SocketInterface/RiaNNCCommands.cpp
    SocketInterface/RiaPropertyDataCommands.cpp
    SocketInterface/RiaWellDataCommands.cpp
    SocketInterface/RiaSocketTools.cpp
    SocketInterface/RiaSocketDataTransfer.cpp
)

list( APPEND CPP_SOURCES
     ${SOCKET_INTERFACE_FILES}
     ${UNIT_TEST_FILES}
)

if (RESINSIGHT_ENABLE_GRPC)
    list( APPEND REFERENCED_CMAKE_FILES
        GrpcInterface/CMakeLists.cmake
    )
endif(RESINSIGHT_ENABLE_GRPC)

list( APPEND REFERENCED_CMAKE_FILES
    Application/CMakeLists_files.cmake
    Application/Tools/CMakeLists_files.cmake 
    Application/Tools/WellPathTools/CMakeLists_files.cmake

    ReservoirDataModel/CMakeLists_files.cmake
    ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
    ReservoirDataModel/Completions/CMakeLists_files.cmake
    FileInterface/CMakeLists_files.cmake
    
    ProjectDataModel/CMakeLists_files.cmake
    ProjectDataModel/AnalysisPlots/CMakeLists_files.cmake
	ProjectDataModel/CorrelationPlots/CMakeLists_files.cmake
    ProjectDataModel/GridCrossPlots/CMakeLists_files.cmake
    ProjectDataModel/GridCrossPlots/CellFilters/CMakeLists_files.cmake
    ProjectDataModel/Summary/CMakeLists_files.cmake
    ProjectDataModel/Flow/CMakeLists_files.cmake
    ProjectDataModel/Annotations/CMakeLists_files.cmake
    ProjectDataModel/Completions/CMakeLists_files.cmake
    ProjectDataModel/Measurement/CMakeLists_files.cmake
    ProjectDataModel/PlotTemplates/CMakeLists_files.cmake
    ProjectDataModel/Surfaces/CMakeLists_files.cmake
    ProjectDataModelCommands/CMakeLists_files.cmake
    GeoMech/GeoMechVisualization/CMakeLists_files.cmake
    
    ModelVisualization/CMakeLists_files.cmake
    ModelVisualization/GridBox/CMakeLists_files.cmake
    ModelVisualization/Intersections/CMakeLists_files.cmake
	ModelVisualization/Surfaces/CMakeLists_files.cmake
    ModelVisualization/WindowEdgeAxesOverlayItem/CMakeLists_files.cmake
    
    UserInterface/CMakeLists_files.cmake
    UserInterface/AnalysisPlots/CMakeLists_files.cmake
    
    Commands/CMakeLists_files.cmake
	Commands/AnalysisPlotCommands/CMakeLists_files.cmake
    Commands/ApplicationCommands/CMakeLists_files.cmake
    Commands/AnnotationCommands/CMakeLists_files.cmake
	Commands/ColorLegendCommands/CMakeLists_files.cmake
    Commands/CompletionCommands/CMakeLists_files.cmake
    Commands/CompletionExportCommands/CMakeLists_files.cmake
	Commands/CorrelationPlotCommands/CMakeLists_files.cmake
    Commands/CrossSectionCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/EclipseWell/CMakeLists_files.cmake
    Commands/ExportCommands/CMakeLists_files.cmake
    Commands/FlowCommands/CMakeLists_files.cmake
    Commands/GridCrossPlotCommands/CMakeLists_files.cmake
    Commands/HoloLensCommands/CMakeLists_files.cmake
    Commands/IntersectionBoxCommands/CMakeLists_files.cmake
    Commands/IntersectionViewCommands/CMakeLists_files.cmake
    Commands/MeasurementCommands/CMakeLists_files.cmake
    Commands/OctaveScriptCommands/CMakeLists_files.cmake
    Commands/OperationsUsingObjReferences/CMakeLists_files.cmake
    Commands/SummaryPlotCommands/CMakeLists_files.cmake
    Commands/SsiHubImportCommands/CMakeLists_files.cmake
    Commands/SurfaceCommands/CMakeLists_files.cmake
    Commands/ToggleCommands/CMakeLists_files.cmake
    Commands/ViewLink/CMakeLists_files.cmake
    Commands/WellLogCommands/CMakeLists_files.cmake
    Commands/WellPathCommands/CMakeLists_files.cmake
    Commands/PlotTemplateCommands/CMakeLists_files.cmake

    CommandFileInterface/CMakeLists_files.cmake
    CommandFileInterface/Core/CMakeLists_files.cmake
    Commands/FractureCommands/CMakeLists_files.cmake
)

option (RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS "Include ApplicationCode Unit Tests" OFF)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
if (RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
    add_definitions(-DUSE_UNIT_TESTS)

    list( APPEND REFERENCED_CMAKE_FILES
        UnitTests/CMakeLists_files.cmake
        )
    
    list( APPEND CPP_SOURCES 
        ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc    
        )
    
endif()


# Include source file lists from *.cmake files
foreach (referencedfile ${REFERENCED_CMAKE_FILES})
    include (${referencedfile})
endforeach (referencedfile)

list( APPEND CPP_SOURCES
    ${CODE_SOURCE_FILES}
)

#############################################################################
# Sub-directory projects
#############################################################################

add_subdirectory(ResultStatisticsCache)

set( RI_LIBRARIES
    ResultStatisticsCache
)

#
# Odb api
#
set(RESINSIGHT_ODB_API_DIR "" CACHE PATH "Optional path to the ABAQUS ODB API from Simulia. Needed for support of geomechanical models")
if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "") 
    add_definitions(-DUSE_ODB_API)
    add_subdirectory(GeoMech/OdbReader)
    SET(RESINSIGHT_USE_ODB_API 1)
    MESSAGE( STATUS "Using ODB-Api from : ${RESINSIGHT_ODB_API_DIR}" )
endif()

add_subdirectory(GeoMech/GeoMechDataModel)
list( APPEND RI_LIBRARIES
    RigGeoMechDataModel
)

if (RESINSIGHT_USE_ODB_API)
    list( APPEND RI_LIBRARIES
        RifOdbReader
    )
endif()


# 
# HDF5  
#
if (RESINSIGHT_FOUND_HDF5)
    list( APPEND CPP_SOURCES
        FileInterface/RifHdf5Reader.h
        FileInterface/RifHdf5Reader.cpp
    )

    add_definitions(-DUSE_HDF5)

    if(MSVC)
	    add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

        include_directories(${RESINSIGHT_HDF5_DIR}/include)
    else()
	    add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
		add_definitions(${HDF5_DEFINITIONS})

		include_directories(${HDF5_INCLUDE_DIRS})
    endif() # MSVC

    source_group( "FileInterface" FILES FileInterface/RifHdf5Reader.h FileInterface/RifHdf5Reader.cpp )
endif()

#############################################################################
# Qt specifics: Moc, ui, resources
#############################################################################

set ( QT_MOC_HEADERS
    ${QT_MOC_HEADERS}
    
    ProjectDataModel/RimMimeData.h
    ProjectDataModel/RimBoxIntersection.h
    
    SocketInterface/RiaSocketServer.h
)

qt5_wrap_cpp(MOC_SOURCE_FILES ${QT_MOC_HEADERS} )
qt5_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )


# NOTE! Resources in subfolders must append to QRC_FILES using the following statement
#   set( QRC_FILES
#       ${QRC_FILES}
#       ${CMAKE_CURRENT_SOURCE_DIR}/Resources/myLibrary.qrc
#       PARENT_SCOPE
#   )

set( QRC_FILES
    ${QRC_FILES} 
    Resources/ResInsight.qrc
)

# Runs RCC on specified files
qt5_add_resources( QRC_FILES_CPP
    ${QRC_FILES}
)

# Adding resource (RC) files for Windows
if ( MSVC )
    set( WIN_RESOURCE Resources/ResInsight.rc )
endif()


################################################################################
# Create source groups - see also included CMakeLists_files.cmake
################################################################################
source_group( "ModelVisualization"  FILES ${MODEL_VISUALIZATION_FILES} )
source_group( "SocketInterface"     FILES ${SOCKET_INTERFACE_FILES} )
source_group( "UnitTests"           FILES ${UNIT_TEST_FILES} )


#############################################################################
# Set up the main executable with its source files
#############################################################################

# Default behaviour for a Qt application is a console application, resulting in a console window always being launced at startup
# The following statement is used to control this behaviour
#   set_target_properties( MY_TARGET PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
# This setting i not honored by Cmake - http://public.kitware.com/Bug/view.php?id=14326 
# We use the following workaround described in the bug report
#   ADD_EXECUTABLE(${TARGET_NAME} WIN32 ${SRC})
# See CMake symbol WIN32_EXECUTABLE for details

if (MSVC)
    set( EXE_FILES WIN32)
    if (RESINSIGHT_ENABLE_GRPC)
        # GRPC generates a lot of harmless warnings on MSVC
        set_source_files_properties(${GRPC_CPP_SOURCES} ${GRPC_UNIT_TEST_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/wd4251 /wd4702 /wd4005 /wd4244 /wd4125 /wd4267")
    endif(RESINSIGHT_ENABLE_GRPC)
elseif (APPLE)
    set( EXE_FILES MACOSX_BUNDLE)
else()
	set_source_files_properties(${GRPC_CPP_SOURCES} ${GRPC_UNIT_TESTS_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual")
endif()
set( EXE_FILES
    ${EXE_FILES}
    ${CPP_SOURCES}
    ${MOC_SOURCE_FILES}
    ${FORM_FILES_CPP}
    ${QRC_FILES_CPP}
    ${WIN_RESOURCE}
    ${HEADER_FILES}
    ${REFERENCED_CMAKE_FILES}
    ../ResInsightVersion.cmake
    .clang-format
    .clang-tidy
    Adm/RiaVersionInfo.h.cmake
    $<TARGET_OBJECTS:cafCommandFeatures> # Needed for cmake version < 3.12. Remove when we can use target_link_libraries with OBJECT libraries
)

if (RESINSIGHT_ENABLE_GRPC)
    list(APPEND EXE_FILES
         ${GRPC_CPP_SOURCES}
         ${GRPC_HEADER_FILES}
         ${GRPC_PYTHON_SOURCES_FULL_PATH}
		 ${GRPC_PROTO_FILES_FULL_PATH}
         Adm/RiaVersionInfo.py.cmake
    )
    if (DEFINED GRPC_LIBRARY_DIRS)
        message(STATUS "Using GRPC Library Dir: ${GRPC_LIBRARY_DIRS}")
        link_directories(${GRPC_LIBRARY_DIRS})
    endif(DEFINED GRPC_LIBRARY_DIRS)
endif(RESINSIGHT_ENABLE_GRPC)

add_executable( ResInsight ${EXE_FILES})

option(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS "Use Precompiled Headers" OFF)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
if(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
    message("Precompiled Headers is enabled on : ${PROJECT_NAME}")
    target_precompile_headers(ResInsight PRIVATE pch.h)

    set_source_files_properties(${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch")
endif()


if (MSVC) 
    # The following warnings are supposed to be used in ResInsight, but temporarily disabled to avoid too much noise
    #  warning C4245: 'return': conversion from 'int' to 'size_t', signed/unsigned mismatch
	#  warning C4005: Macro redefinition for math constants (M_PI, M_SQRT2 etc)

    # If possible, the following command is supposed to be the final target
    # set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "/W3 /wd4190 /wd4100 /wd4127")    
    
    set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "/W3 /wd4190 /wd4100 /wd4127 /wd4245 /wd4005")
    if (CMAKE_CXX_COMPILER_VERSION LESS_EQUAL 19.14)
        # The following warning is generated over 800 times from a qwt header only using VS2015
        # Disabling temporarily
        # warning C4505 'function' : unreferenced local function has been removed 
        set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "/wd4505")
    endif()

endif() 

#############################################################################
# Application icon for MacOS X bundle
#############################################################################

if (APPLE)
    add_custom_command (OUTPUT Resources/ResInsight.icns
        COMMAND sips -s format icns ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppLogo48x48.png --out ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns
        COMMENT Converting application icon
    )
    add_custom_target (ResInsight-icns
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns)
    add_dependencies (ResInsight ResInsight-icns)
    set_target_properties (ResInsight PROPERTIES
        MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_BINARY_DIR}/Resources/ResInsight.icns)
endif ()

#############################################################################
# Set up libraries and dependent projects to link with
#############################################################################

set( LINK_LIBRARIES
    ${OPM_LIBRARIES}
        
    ${RI_LIBRARIES}

    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    
    ${THIRD_PARTY_LIBRARIES}

    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
)

if (RESINSIGHT_ENABLE_GRPC)
	if (UNIX)
		foreach(GRPC_LIBRARY ${GRPC_LIBRARIES})
			list(APPEND LINK_LIBRARIES "${GRPC_LIBRARY_DIRS}/lib${GRPC_LIBRARY}.a")
		endforeach()
		list(APPEND LINK_LIBRARIES ${GRPC_PACKAGE_LIBRARIES})
	else()		
		list(APPEND LINK_LIBRARIES ${GRPC_LIBRARIES})
		set_target_properties(ResInsight PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:MSVCRTD.lib")
	endif()
endif()

# According to ivarun this is needed on OpenSuse, and Fedora. See: https://github.com/OPM/ResInsight/pull/7
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND EXTERNAL_LINK_LIBRARIES
        rt
    )
endif()

target_link_libraries( ResInsight ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})


#############################################################################
# cotire and unity builds
#############################################################################

set( UNITY_EXCLUDE_FILES
    # forever is used as variable name, and this symbol is defined by Qt and used in precompiled headers
    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
    
    # multiple QRC files are not supported
    qrc_cafAnimControl.cxx
    qrc_ResInsight.cxx

    # mix of cvf and Qt namespaces
    ModelVisualization/GridBox/RivGridBoxGenerator.cpp
    ModelVisualization/Intersections/RivIntersectionGeometryGenerator.cpp

    # exclude file using Eigen
    ReservoirDataModel/RigCellGeometryTools.cpp
    ReservoirDataModel/Completions/RigTransmissibilityCondenser.cpp
    ReservoirDataModel/Completions/RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
    ReservoirDataModel/Completions/RigEclipseToStimPlanCalculator.cpp

    # exclude file using SolveSpace
    Application/Tools/WellPathTools/RiaSCurveCalculator.cpp

    # QT 5
    qrc_cafAnimControl.cpp
    qrc_ResInsight.cpp
    qrc_cafCommandFeatures.cpp
    ProjectDataModel/RimContourMapView.cpp
    Commands/CompletionExportCommands/RicExportFractureCompletionsImpl.cpp
)

if(RESINSIGHT_ENABLE_COTIRE)

    foreach (fileToExclude ${UNITY_EXCLUDE_FILES})
        set_source_files_properties (${fileToExclude} PROPERTIES COTIRE_EXCLUDED TRUE)
    endforeach(fileToExclude)

    foreach (cppFile ${CAF_COTIRE_START_NEW_UNITY_SOURCES})
        set_source_files_properties (${cppFile} PROPERTIES COTIRE_START_NEW_UNITY_SOURCE TRUE)
    endforeach(cppFile ${CAF_COTIRE_START_NEW_UNITY_SOURCES})

    # disable precompiled headers
    set_target_properties(ResInsight PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
   
    cotire(ResInsight)
    
    # make sure the unity target is included in the active builds to trigger rebuild before debug
    get_target_property(_unityTargetName ResInsight COTIRE_UNITY_TARGET_NAME)
    set_target_properties(${_unityTargetName} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)
    set_target_properties(ResInsight PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

endif()

if(RESINSIGHT_ENABLE_UNITY_BUILD)
    foreach (fileToExclude ${UNITY_EXCLUDE_FILES})
        set_source_files_properties (${fileToExclude} PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
    endforeach(fileToExclude)
endif()

#############################################################################
# Copy Dlls on MSVC
#############################################################################
if (MSVC)

    # Qt DLLs
    message(STATUS "Creating post build step for copying Qt DLLs")
		
	foreach (qtlib ${QT_LIBRARIES})
		add_custom_command(TARGET ResInsight POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:ResInsight>
		)
	endforeach(qtlib)
			
	if (_unityTargetName)
		foreach (qtlib ${QT_LIBRARIES})
			add_custom_command(TARGET ${_unityTargetName} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${qtlib}> $<TARGET_FILE_DIR:${_unityTargetName}>
			)
		endforeach(qtlib)
	endif(_unityTargetName)

    # Odb Dlls
    if (RESINSIGHT_USE_ODB_API)
        # Find all the dlls
        file (GLOB  RI_ALL_ODB_DLLS  ${RESINSIGHT_ODB_API_DIR}/lib/*.dll)
    
        # Strip off the path
        foreach (aDLL  ${RI_ALL_ODB_DLLS})
             get_filename_component(filenameWithExt ${aDLL} NAME)
             list(APPEND RI_ODB_DLLS ${filenameWithExt} )
        endforeach(aDLL)

        foreach (aDLL ${RI_ODB_DLLS})
            list(APPEND RI_DLL_FILENAMES ${RESINSIGHT_ODB_API_DIR}/lib/${aDLL})
        endforeach()
    endif()

    # HDF5 Dlls
    if (RESINSIGHT_FOUND_HDF5)
        set (HDF5_DLL_NAMES hdf5 hdf5_cpp szip zlib)
        foreach (HDF5_DLL_NAME ${HDF5_DLL_NAMES})
            list(APPEND RI_DLL_FILENAMES ${RESINSIGHT_HDF5_DIR}/bin/${HDF5_DLL_NAME}.dll)
        endforeach( HDF5_DLL_NAME )
    endif()

endif(MSVC)

# Copy to target directory
foreach (FILE_TO_COPY ${RI_DLL_FILENAMES})
    add_custom_command(TARGET ResInsight POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different  
                        ${FILE_TO_COPY}
                        $<TARGET_FILE_DIR:ResInsight>)
    if (_unityTargetName)
        add_custom_command(TARGET ${_unityTargetName} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different  
                            ${FILE_TO_COPY}
                            $<TARGET_FILE_DIR:ResInsight>)
    endif()
endforeach()

# Generate Python code in a target that is part of ALL_BUILD and depends on ResInsight
if (RESINSIGHT_ENABLE_GRPC)	
    set(RESINSIGHT_GRPC_PYTHON_EXECUTABLE "" CACHE FILEPATH "gRPC : Path to Python 3 executable, required to build the Python client library")
    if (RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
        add_custom_command(OUTPUT ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/pdm_objects.py
				    COMMAND ResInsight ARGS --console --generate ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/pdm_objects.py
                    DEPENDS ResInsight
                    COMMENT "Generating ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/pdm_objects.py"
	    )   
        list(APPEND GRPC_GENERATED_PYTHON_SOURCES ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/pdm_objects.py)
		add_custom_target(GeneratedPythonSources ALL DEPENDS ${GRPC_GENERATED_PYTHON_SOURCES})
		add_dependencies(GeneratedPythonSources ResInsight)
	else()
		message(STATUS "RESINSIGHT_GRPC_PYTHON_EXECUTABLE not specified. Will not install GRPC Python code.")
	endif(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)	
endif(RESINSIGHT_ENABLE_GRPC)

#############################################################################
# Install 
#############################################################################

set (RESINSIGHT_LICENSE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Adm/LicenseInformation.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/Adm/gplLicense.txt
  )

# bundle libraries together with private installation
if (RESINSIGHT_PRIVATE_INSTALL)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      # tell binary to first attempt to load libraries from its own directory
      set(RESINSIGHT_INSTALL_RPATH "\$ORIGIN")

      if (${RESINSIGHT_USE_ODB_API})
        # This is a "hack" to make ResInsight runtime find the ODB so files used when compiling.
        # statoil wanted it this way, but we should probbly make a different installoptions that does things this way,
        # and really do copy them when doing PRIVATE_INSTALL
        set(RESINSIGHT_INSTALL_RPATH ${RESINSIGHT_INSTALL_RPATH} ${RESINSIGHT_ODB_API_DIR}/lib)
      endif()
      
      if (EXISTS ${RESINSIGHT_ERT_EXTERNAL_LIB_ROOT}) 
        # append the external ERT lib root
        set(RESINSIGHT_INSTALL_RPATH ${RESINSIGHT_INSTALL_RPATH} ${RESINSIGHT_ERT_EXTERNAL_LIB_ROOT})
      endif()
      
      #message("RESINSIGHT_INSTALL_RPATH : " "${RESINSIGHT_INSTALL_RPATH}")

      set_target_properties (ResInsight PROPERTIES INSTALL_RPATH "${RESINSIGHT_INSTALL_RPATH}")
      
      # bundle Qt5 libraries
      option (RESINSIGHT_QT5_BUNDLE_LIBRARIES "Bundle Qt5 libraries" OFF)
      mark_as_advanced(FORCE RESINSIGHT_QT5_BUNDLE_LIBRARIES) 
      if (RESINSIGHT_QT5_BUNDLE_LIBRARIES)
        message( STATUS "Bundling of Qt5 libraries is enabled" )
        
        foreach (qtlib ${QT_LIBRARIES})
          get_target_property(FILE_NAME_FULL_PATH ${qtlib} LOCATION)
          message (STATUS "${qtlib} location on disk - ${FILE_NAME_FULL_PATH}")
          get_filename_component(FILE_NAME_WE ${FILE_NAME_FULL_PATH} NAME_WE)
          set(FILE_NAME_FOR_INSTALL "${FILE_NAME_WE}.so.5")
          message (STATUS "${qtlib} filename for install - ${FILE_NAME_FOR_INSTALL}")
          
          install(FILES ${FILE_NAME_FULL_PATH} DESTINATION ${RESINSIGHT_INSTALL_FOLDER} RENAME ${FILE_NAME_FOR_INSTALL} )
        endforeach()
      endif(RESINSIGHT_QT5_BUNDLE_LIBRARIES)

    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # put a .exe.local file in the target directory to pick up DLLs from there
        install (CODE "exec_program (\"${CMAKE_COMMAND}\" ARGS -E touch \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}ResInsight${CMAKE_EXECUTABLE_SUFFIX}.local)")

        set (RESINSIGHT_FILES ${RI_DLL_FILENAMES})

        if(WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
          get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

          execute_process(
            COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt5_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )

          set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

          if(EXISTS ${imported_location})
            add_executable(Qt5::windeployqt IMPORTED)

            set_target_properties(Qt5::windeployqt PROPERTIES
              IMPORTED_LOCATION ${imported_location}
            )
          endif()
        endif()

        # TODO(wjwwood): find a way to make this optional or to run without "deploying" the
        #                necessary dlls and stuff to the bin folder.
        #                see:
        # https://stackoverflow.com/questions/41193584/deploy-all-qt-dependencies-when-building#41199492
        if(TARGET Qt5::windeployqt)
          # execute windeployqt in a tmp directory after build
          add_custom_command(TARGET ResInsight
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
            COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
            COMMAND
            Qt5::windeployqt
            --no-compiler-runtime
            --no-system-d3d-compiler
            --no-quick-import
            --no-translations
            --verbose 0
            --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
            "$<TARGET_FILE_DIR:ResInsight>/$<TARGET_FILE_NAME:ResInsight>"
          )

          # copy deployment directory during installation
          install(
            DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
            DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
          )
        endif()

        # install GRPC-related DLLs
        if (RESINSIGHT_ENABLE_GRPC)
            set (GRPC_DLL_NAMES libprotobuf cares zlib1)
            foreach (dllname ${GRPC_DLL_NAMES})		  
              list(APPEND RESINSIGHT_FILES $<TARGET_FILE_DIR:ResInsight>/${dllname}.dll)
            endforeach(dllname ${GRPC_DLL_NAMES})
	    endif()

        # OpenSSL
        option(RESINSIGHT_BUNDLE_OPENSSL "Bundle the OpenSSL binary DLLs" OFF)
        mark_as_advanced(FORCE RESINSIGHT_BUNDLE_OPENSSL)

        if (RESINSIGHT_BUNDLE_OPENSSL)
          find_package(OpenSSL)
          if (OPENSSL_FOUND)
            message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
            SET(OPENSSL_BIN_DIR ${OPENSSL_INCLUDE_DIR})
            string(REPLACE "include" "bin" OPENSSL_BIN_DIR ${OPENSSL_BIN_DIR})
            message("OpenSSL binary dir: ${OPENSSL_BIN_DIR}")
  
            install(FILES ${OPENSSL_BIN_DIR}/libeay32.dll DESTINATION ${RESINSIGHT_INSTALL_FOLDER})
            install(FILES ${OPENSSL_BIN_DIR}/ssleay32.dll DESTINATION ${RESINSIGHT_INSTALL_FOLDER})
          endif(OPENSSL_FOUND)
        endif(RESINSIGHT_BUNDLE_OPENSSL)

        # CRT
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
        set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
        include(InstallRequiredSystemLibraries)
        install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${RESINSIGHT_INSTALL_FOLDER})

    endif()

    # install gRPC Python files
    if (RESINSIGHT_ENABLE_GRPC)
		message(STATUS "GRPC enabled")
		if (RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
			message(STATUS "Python found")
			if (RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE)	
				message(STATUS "Bundling Python GRPC modules")
				add_custom_command(
					TARGET ResInsight
					POST_BUILD
					COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} -m pip install --target=${GRPC_PYTHON_SOURCE_PATH} grpcio-tools
				)
			endif()
			install(DIRECTORY ${GRPC_PYTHON_SOURCE_PATH}/ DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/Python)
            install(FILES ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/pdm_objects.py DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/Python/rips/generated)
		endif()
	endif()

    set (RESINSIGHT_FILES ${RESINSIGHT_FILES} ${RESINSIGHT_LICENSE_FILES})
        

    install(TARGETS ResInsight DESTINATION ${RESINSIGHT_INSTALL_FOLDER})

    install(FILES ${RESINSIGHT_FILES} DESTINATION ${RESINSIGHT_INSTALL_FOLDER} )

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resinsight DESTINATION ${RESINSIGHT_INSTALL_FOLDER} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
    endif()

else (RESINSIGHT_PRIVATE_INSTALL)
  # binaries go in /usr/bin
  install (TARGETS ResInsight
    DESTINATION bin
    )
  # license go in /usr/share/doc
  install (FILES ${RESINSIGHT_LICENSE_FILES}
    DESTINATION share/doc/ResInsight
    )
  # no bundled libraries for system install
  # application icon
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppLogo48x48.png
    DESTINATION share/icons/hicolor/48x48/apps
    RENAME ResInsight.png
    )
  # desktop environment icon; remember to call `update-desktop-database`
  # in package post-install scripts
  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/resinsight.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/resinsight.desktop
    @ONLY
    )
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/resinsight.desktop
    DESTINATION share/applications
    )
endif (RESINSIGHT_PRIVATE_INSTALL)


################################################################################
# Installation packaging
################################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CPACK_GENERATOR TGZ)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CPACK_GENERATOR ZIP)
endif()


# Handling of system name on Windows
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_SYSTEM_NAME win64)
  else()
    set(CPACK_SYSTEM_NAME win32)
  endif()
endif()

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
string(REGEX MATCH "el[6,7,8]?" RESINSIGHT_RHEL_SYSTEM_NAME ${CMAKE_SYSTEM})


set(RESINSIGHT_PACKAGE_NAME "ResInsight")

set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}-${STRPRODUCTVER}")

if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_odb")
endif()

if(NOT ${OCTAVE_VERSION_STRING} EQUAL "")
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_oct-${OCTAVE_VERSION_STRING}")
endif()

if(RESINSIGHT_BUNDLE_OPENSSL AND OPENSSL_FOUND)
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_openssl")
endif()

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
if (NOT "${RESINSIGHT_RHEL_SYSTEM_NAME}" STREQUAL "")
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_${RESINSIGHT_RHEL_SYSTEM_NAME}")
else()
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_${CPACK_SYSTEM_NAME}")
endif()

#message("RESINSIGHT_PACKAGE_NAME : " ${RESINSIGHT_PACKAGE_NAME})
set(CPACK_PACKAGE_FILE_NAME ${RESINSIGHT_PACKAGE_NAME})


include (CPack)
