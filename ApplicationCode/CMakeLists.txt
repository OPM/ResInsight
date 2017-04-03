cmake_minimum_required (VERSION 2.8)

project (ApplicationCode)

# NB: The generated file is written to Cmake binary folder to avoid source tree pollution
# This folder is added to include_directories
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/RiaVersionInfo.h
)


include_directories(
    ${cafPdmCore_SOURCE_DIR}
    ${cafPdmUiCore_SOURCE_DIR}
    ${cafPdmXml_SOURCE_DIR}
    ${cafProjectDataModel_SOURCE_DIR}

    ${cafCommand_SOURCE_DIR}
    ${cafViewer_SOURCE_DIR}
    ${cafAnimControl_SOURCE_DIR}
    ${cafUserInterface_SOURCE_DIR}
    ${cafPdmCvf_SOURCE_DIR}
    ${CommonCode_SOURCE_DIR}
    ${cafVizExtensions_SOURCE_DIR}
    ${cafTensor_SOURCE_DIR}

	${ResInsight_SOURCE_DIR}/ThirdParty
    ${ResInsight_SOURCE_DIR}/ThirdParty/NRLib/nrlib/well
    ${ResInsight_SOURCE_DIR}/ThirdParty/Qwt/src
	${ERT_INCLUDE_DIRS}

    ${Boost_INCLUDE_DIRS}
    ${ResInsight_SOURCE_DIR}/ThirdParty/custom-opm-common/opm-common/
    ${custom-opm-parser_SOURCE_DIR}/opm-parser/
    ${custom-opm-flowdiagnostics_SOURCE_DIR}/opm-flowdiagnostics/
    ${custom-opm-flowdiag-app_SOURCE_DIR}/opm-flowdiagnostics-applications/
    ${custom-opm-flowdiag-app_SOURCE_DIR}/opmCore

    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Adm
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/Commands
    ${CMAKE_CURRENT_SOURCE_DIR}/Commands/EclipseCommands
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/GridBox
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Intersections
    ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface

    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Summary
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Flow
    
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultStatisticsCache
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
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

set( APPLICATION_FILES 
    RiaMain.cpp
        
    Application/RiaApplication.cpp
    Application/RiaPreferences.cpp
    Application/RiaImageFileCompare.cpp
    Application/RiaImageCompareReporter.cpp
    Application/RiaProjectModifier.cpp
    Application/RiaRegressionTest.cpp
    Application/RiaColorTables.cpp
    Application/RiaLogging.h
    Application/RiaLogging.cpp
)

set( SOCKET_INTERFACE_FILES
    SocketInterface/RiaSocketServer.cpp
    SocketInterface/RiaProjectInfoCommands.cpp
    SocketInterface/RiaCaseInfoCommands.cpp
    SocketInterface/RiaGeometryCommands.cpp
    SocketInterface/RiaPropertyDataCommands.cpp
    SocketInterface/RiaWellDataCommands.cpp
    SocketInterface/RiaSocketTools.cpp
    SocketInterface/RiaSocketDataTransfer.cpp
)

list( APPEND CPP_SOURCES
     ${APPLICATION_FILES}
     ${SOCKET_INTERFACE_FILES}
     ${UNIT_TEST_FILES}
)

list( APPEND REFERENCED_CMAKE_FILES
    ReservoirDataModel/CMakeLists_files.cmake
    ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
    FileInterface/CMakeLists_files.cmake
    
    ProjectDataModel/CMakeLists_files.cmake
    ProjectDataModel/Summary/CMakeLists_files.cmake
    ProjectDataModel/Flow/CMakeLists_files.cmake
    
    GeoMech/GeoMechVisualization/CMakeLists_files.cmake
    
    ModelVisualization/CMakeLists_files.cmake
    ModelVisualization/GridBox/CMakeLists_files.cmake
    ModelVisualization/Intersections/CMakeLists_files.cmake
    
    UserInterface/CMakeLists_files.cmake
	
    Commands/CMakeLists_files.cmake
    Commands/ApplicationCommands/CMakeLists_files.cmake
    Commands/CrossSectionCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/CMakeLists_files.cmake
    Commands/EclipseCommands/EclipseWell/CMakeLists_files.cmake
    Commands/FlowCommands/CMakeLists_files.cmake
    Commands/IntersectionBoxCommands/CMakeLists_files.cmake
    Commands/OctaveScriptCommands/CMakeLists_files.cmake
	Commands/OperationsUsingObjReferences/CMakeLists_files.cmake
    Commands/SummaryPlotCommands/CMakeLists_files.cmake
    Commands/ToggleCommands/CMakeLists_files.cmake
    Commands/ViewLink/CMakeLists_files.cmake
    Commands/WellLogCommands/CMakeLists_files.cmake
    Commands/WellPathCommands/CMakeLists_files.cmake
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

#
# Odb api
#
set(RESINSIGHT_ODB_API_DIR "" CACHE PATH "Path to the ODB api from Simulia")
if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "") 
	add_definitions(-DUSE_ODB_API)
	add_subdirectory(GeoMech/OdbReader)
	SET(RESINSIGHT_USE_ODB_API 1)
	MESSAGE( STATUS "Using ODB-Api from : ${RESINSIGHT_ODB_API_DIR}" )
endif()

add_subdirectory(GeoMech/GeoMechDataModel)

#############################################################################
# Qt specifics: Moc, ui, resources
#############################################################################

set ( QT_MOC_HEADERS
    ${QT_MOC_HEADERS}
    Application/RiaApplication.h
    
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
source_group( "Application"         FILES ${APPLICATION_FILES} )
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
)

add_executable( ResInsight ${EXE_FILES} )


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

if (MSVC)
  set (RI_BOOST_LIBRARIES
    debug     ${Boost_SYSTEM_LIBRARY_DEBUG}
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
    debug     ${Boost_FILESYSTEM_LIBRARY_DEBUG}
    optimized ${Boost_FILESYSTEM_LIBRARY_RELEASE}
    debug     ${Boost_REGEX_LIBRARY_DEBUG}
    optimized ${Boost_REGEX_LIBRARY_RELEASE}
  )
else()
  set (RI_BOOST_LIBRARIES ${Boost_LIBRARIES})
endif()


set( LINK_LIBRARIES
    ${OPM_LIBRARIES}
        
    WellPathImportSsihub

	ResultStatisticsCache
	RigGeoMechDataModel

    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    
    ${ERT_LIBRARIES}
    
    ${THIRD_PARTY_LIBRARIES}

    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    
    ${RI_BOOST_LIBRARIES}
)
set( EXTERNAL_LINK_LIBRARIES ${ERT_LIBRARY_LIST} )

# According to ivarun this is needed on OpenSuse, and Fedora. See: https://github.com/OPM/ResInsight/pull/7
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( EXTERNAL_LINK_LIBRARIES
        ${EXTERNAL_LINK_LIBRARIES}
        rt
    )
endif()

# Add the odb reader if we are compiling with odb support
IF (${RESINSIGHT_USE_ODB_API})
	set ( LINK_LIBRARIES ${LINK_LIBRARIES} RifOdbReader)
ENDIF()

target_link_libraries( ResInsight ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})


#############################################################################
# cotire
#############################################################################
if(RESINSIGHT_ENABLE_COTIRE)

    # forever is used as variable name, and this symbol is defined by Qt and used in precompiled headers
    set_source_files_properties (${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc     PROPERTIES COTIRE_EXCLUDED TRUE)
  
    # multiple QRC files are not supported
    set_source_files_properties (qrc_cafAnimControl.cxx                                     PROPERTIES COTIRE_EXCLUDED TRUE)

    # TODO: Revisit why opm-parser is not working
    set_source_files_properties (UnitTests/opm-parser-Test.cpp                              PROPERTIES COTIRE_EXCLUDED TRUE)
 
    # variables at global file scope
    set_source_files_properties (ModelVisualization/Intersections/RivIntersectionGeometryGenerator.cpp    PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (ModelVisualization/Intersections/RivIntersectionPartMgr.cpp              PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (ModelVisualization/Intersections/RivIntersectionBoxPartMgr.cpp           PROPERTIES COTIRE_EXCLUDED TRUE)
    
    set_source_files_properties (ModelVisualization/GridBox/RivGridBoxGenerator.cpp         PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (Commands/WellLogCommands/RicWellLogPlotCurveFeatureImpl.cpp PROPERTIES COTIRE_EXCLUDED TRUE)

    # using namespace cvf
    set_source_files_properties (GeoMech/GeoMechVisualization/RivFemPartGeometryGenerator.cpp  PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (UnitTests/cvfGeometryTools-Test.cpp                        PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (UnitTests/opm-parser-Performance-Test.cpp                        PROPERTIES COTIRE_EXCLUDED TRUE)

    # opm parser
    set_source_files_properties (ProjectDataModel/RimProject.cpp                        PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (UnitTests/opm-parser-Performance-Test.cpp              PROPERTIES COTIRE_EXCLUDED TRUE)
    set_source_files_properties (UnitTests/opm-Test.cpp                                 PROPERTIES COTIRE_EXCLUDED TRUE)
    
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

# Copy Odb Dlls

if ((MSVC) AND (${RESINSIGHT_USE_ODB_API}))
	# Find all the dlls
	file (GLOB  RI_ALL_ODB_DLLS  ${RESINSIGHT_ODB_API_DIR}/lib/*.dll)
	
	# Strip off the path
	foreach (aDLL  ${RI_ALL_ODB_DLLS})
		 get_filename_component(filenameWithExt ${aDLL} NAME)
		 list(APPEND RI_ODB_DLLS ${filenameWithExt} )
	endforeach(aDLL)

	# Copy to target directory
	foreach (aDLL ${RI_ODB_DLLS})
        add_custom_command(TARGET ResInsight POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different  
						   "${RESINSIGHT_ODB_API_DIR}/lib/${aDLL}"  
						   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
        if (_unityTargetName)
            add_custom_command(TARGET ${_unityTargetName} POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different  
		    				   "${RESINSIGHT_ODB_API_DIR}/lib/${aDLL}"  
		    				   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
        endif()
    endforeach()

endif()

    # Qt DLLs

if (MSVC)

    set (QTLIBLIST QtCore QtCored QtGui QtGuid QtOpenGl QtOpenGld QtNetwork QtNetworkd QtScript QtScriptd QtScriptTools QtScriptToolsd)
    foreach (qtlib ${QTLIBLIST})
        add_custom_command(TARGET ResInsight POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_BINARY_DIR}/${qtlib}4.dll"
                "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
        if (_unityTargetName)
           add_custom_command(TARGET ResInsight POST_BUILD
               COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   "${QT_BINARY_DIR}/${qtlib}4.dll"
                   "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
        endif()
    endforeach( qtlib )

endif(MSVC)

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

        set (RESINSIGHT_FILES
                ${QT_BINARY_DIR}/QtCore4.dll
                ${QT_BINARY_DIR}/QtGui4.dll
                ${QT_BINARY_DIR}/QtOpenGL4.dll
                ${QT_BINARY_DIR}/QtNetwork4.dll
                ${QT_BINARY_DIR}/QtScript4.dll
                ${QT_BINARY_DIR}/QtScriptTools4.dll
            )

		if (${RESINSIGHT_USE_ODB_API})
			set (RESINSIGHT_FILES ${RESINSIGHT_FILES} ${RI_ALL_ODB_DLLS})
		endif()

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
