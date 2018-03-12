cmake_minimum_required (VERSION 2.8.12)

project (ApplicationCode)

# NB: The generated file is written to Cmake binary folder to avoid source tree pollution
# This folder is added to include_directories
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/RiaVersionInfo.h
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message("Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

include_directories(

    # required for compilation of gtest, should be refactored
    ${ResInsight_SOURCE_DIR}/ThirdParty
    
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Adm
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools
    ${CMAKE_CURRENT_SOURCE_DIR}/Commands
    ${CMAKE_CURRENT_SOURCE_DIR}/Commands/EclipseCommands
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/GridBox
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Intersections
    ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface/Core

    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Completions
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Flow
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Summary
    
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultStatisticsCache
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/Completions
    ${CMAKE_CURRENT_SOURCE_DIR}/WellPathImportSsihub
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/OdbReader
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechVisualization

    ${CMAKE_BINARY_DIR}/Generated
    ${CMAKE_CURRENT_BINARY_DIR}
)

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

list( APPEND REFERENCED_CMAKE_FILES
    Application/CMakeLists_files.cmake
    Application/Tools/CMakeLists_files.cmake

    ReservoirDataModel/CMakeLists_files.cmake
    ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
    ReservoirDataModel/Completions/CMakeLists_files.cmake
    FileInterface/CMakeLists_files.cmake
    
    ProjectDataModel/CMakeLists_files.cmake
    ProjectDataModel/Summary/CMakeLists_files.cmake
    ProjectDataModel/Flow/CMakeLists_files.cmake
    ProjectDataModel/Completions/CMakeLists_files.cmake
    
    GeoMech/GeoMechVisualization/CMakeLists_files.cmake
    
    ModelVisualization/CMakeLists_files.cmake
    ModelVisualization/GridBox/CMakeLists_files.cmake
    ModelVisualization/Intersections/CMakeLists_files.cmake
    ModelVisualization/WindowEdgeAxesOverlayItem/CMakeLists_files.cmake
    
    UserInterface/CMakeLists_files.cmake
	
    Commands/CMakeLists_files.cmake
    Commands/ApplicationCommands/CMakeLists_files.cmake
    Commands/CompletionCommands/CMakeLists_files.cmake
    Commands/CompletionExportCommands/CMakeLists_files.cmake
    Commands/CrossSectionCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/EclipseWell/CMakeLists_files.cmake
    Commands/ExportCommands/CMakeLists_files.cmake
    Commands/FlowCommands/CMakeLists_files.cmake
    Commands/IntersectionBoxCommands/CMakeLists_files.cmake
    Commands/IntersectionViewCommands/CMakeLists_files.cmake
    Commands/OctaveScriptCommands/CMakeLists_files.cmake
    Commands/OperationsUsingObjReferences/CMakeLists_files.cmake
    Commands/SummaryPlotCommands/CMakeLists_files.cmake
    Commands/ToggleCommands/CMakeLists_files.cmake
    Commands/ViewLink/CMakeLists_files.cmake
    Commands/WellLogCommands/CMakeLists_files.cmake
    Commands/WellPathCommands/CMakeLists_files.cmake

    CommandFileInterface/CMakeLists_files.cmake
    CommandFileInterface/Core/CMakeLists_files.cmake
    Commands/FractureCommands/CMakeLists_files.cmake
)

option (RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS "Include ApplicationCode Unit Tests" OFF)
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

add_subdirectory(WellPathImportSsihub)
add_subdirectory(ResultStatisticsCache)

set( RI_LIBRARIES
    WellPathImportSsihub
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

    source_group( "FileInterface" FILES FileInterface/RifHdf5Reader.h FileInterface/RifHdf5Reader.cpp )

	add_definitions(-DUSE_HDF5)
endif()

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_SOURING)
    add_definitions(-DENABLE_SOURING)
endif()


#############################################################################
# Qt specifics: Moc, ui, resources
#############################################################################

set ( QT_MOC_HEADERS
    ${QT_MOC_HEADERS}
    
    ProjectDataModel/RimMimeData.h
    ProjectDataModel/RimIntersectionBox.h
    
    SocketInterface/RiaSocketServer.h
)

qt4_wrap_cpp( MOC_FILES_CPP ${QT_MOC_HEADERS} )

qt4_wrap_ui( FORM_FILES_CPP ${QT_UI_FILES} )

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
qt4_add_resources( QRC_FILES_CPP ${QRC_FILES} )

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
elseif (APPLE)
    set( EXE_FILES MACOSX_BUNDLE)
endif()
set( EXE_FILES
    ${EXE_FILES}
    ${CPP_SOURCES}
    ${MOC_FILES_CPP}
    ${FORM_FILES_CPP}
    ${QRC_FILES_CPP}
    ${WIN_RESOURCE}
    ${HEADER_FILES}
	${REFERENCED_CMAKE_FILES}
    ../ResInsightVersion.cmake
    ../.clang-format
)

add_executable( ResInsight ${EXE_FILES} )

if (MSVC) 
    set_target_properties(ResInsight PROPERTIES COMPILE_FLAGS "/wd4190") 
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

# According to ivarun this is needed on OpenSuse, and Fedora. See: https://github.com/OPM/ResInsight/pull/7
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    list(APPEND EXTERNAL_LINK_LIBRARIES
        rt
    )
endif()

target_link_libraries( ResInsight ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})


#############################################################################
# cotire
#############################################################################
if(RESINSIGHT_ENABLE_COTIRE)

    set( COTIRE_EXCLUDE_FILES
        # forever is used as variable name, and this symbol is defined by Qt and used in precompiled headers
        ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
    
        # multiple QRC files are not supported
        qrc_cafAnimControl.cxx
        qrc_ResInsight.cxx

        # mix of cvf and Qt namespaces
        ModelVisualization/GridBox/RivGridBoxGenerator.cpp
        ModelVisualization/Intersections/RivIntersectionGeometryGenerator.cpp

        # exclude file using Eigen
        ReservoirDataModel/RigTransmissibilityCondenser.cpp
        ReservoirDataModel/RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
        ReservoirDataModel/RigCellGeometryTools.cpp
    )

	foreach (fileToExclude ${COTIRE_EXCLUDE_FILES})
        set_source_files_properties (${fileToExclude} PROPERTIES COTIRE_EXCLUDED TRUE)
	endforeach(fileToExclude)

    # disable precompiled headers
    set_target_properties(ResInsight PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
   
    cotire(ResInsight)
    
    # make sure the unity target is included in the active builds to trigger rebuild before debug
    get_target_property(_unityTargetName ResInsight COTIRE_UNITY_TARGET_NAME)
    set_target_properties(${_unityTargetName} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD FALSE)
    set_target_properties(ResInsight PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

endif()



#############################################################################
# Copy Dlls on MSVC
#############################################################################


if (MSVC)

    # Qt DLLs
    set (QTLIBLIST QtCore QtCored QtGui QtGuid QtOpenGl QtOpenGld QtNetwork QtNetworkd QtScript QtScriptd QtScriptTools QtScriptToolsd)
    foreach (qtlib ${QTLIBLIST})
        list(APPEND RI_DLL_FILENAMES ${QT_BINARY_DIR}/${qtlib}4.dll)
    endforeach( qtlib )

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
						"${FILE_TO_COPY}"  
						"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
    if (_unityTargetName)
        add_custom_command(TARGET ${_unityTargetName} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		    				"${FILE_TO_COPY}"  
		    				"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
    endif()
endforeach()


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
      
    endif()

    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # put a .exe.local file in the target directory to pick up DLLs from there
        install (CODE "exec_program (\"${CMAKE_COMMAND}\" ARGS -E touch \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}ResInsight${CMAKE_EXECUTABLE_SUFFIX}.local)")

        set (RESINSIGHT_FILES ${RI_DLL_FILENAMES})

        # CRT
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
        set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
        include(InstallRequiredSystemLibraries)
        install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${RESINSIGHT_INSTALL_FOLDER})

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

if(RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE) 
    get_filename_component(RESINSIGHT_OCTAVE_DIRECTORY ${RESINSIGHT_OCTAVE_PLUGIN_MKOCTFILE} DIRECTORY)
    
    execute_process (COMMAND octave-config -v
        WORKING_DIRECTORY ${RESINSIGHT_OCTAVE_DIRECTORY}
                    OUTPUT_VARIABLE OCTAVE_VERSION_STRING
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

# Handling of system name on Windows
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_SYSTEM_NAME win64)
  else()
    set(CPACK_SYSTEM_NAME win32)
  endif()
endif()

# Append el5 when compiled on RHEL5 and el6 if compiled on RHEL6
string(REGEX MATCH "el[5,6]?" RESINSIGHT_RHEL_SYSTEM_NAME ${CMAKE_SYSTEM})


set(RESINSIGHT_PACKAGE_NAME "ResInsight")

set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}-${STRPRODUCTVER}")

if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_odb")
endif()

if(NOT ${OCTAVE_VERSION_STRING} EQUAL "")
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_oct-${OCTAVE_VERSION_STRING}")
endif()

if(RESINSIGHT_FOUND_HDF5)
    set (RESINSIGHT_PACKAGE_NAME "${RESINSIGHT_PACKAGE_NAME}_souring")
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
