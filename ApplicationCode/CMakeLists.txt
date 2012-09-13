cmake_minimum_required (VERSION 2.8)

project (ApplicationCode)

# NB: The generated file is written to Cmake binary folder to avoid source tree pollution
# This folder is added to include_directories
CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RIVersionInfo.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/RIVersionInfo.h
)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Adm
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
    ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
    ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
    ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
    ${CMAKE_BINARY_DIR}/Generated
)


#-----------------------------------------------------------------
# Configuration of ERT depdendence
#-----------------------------------------------------------------

# By default the ERT_ROOT_PATH variable will point to the ERT distribution which
# is embedded in the ThirdParty/ directory, but by using the ERT_ROOT_PATH
# variable you can point to a different ERT distribution.

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(ERT_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (${CMAKE_SIZEOF_VOID_P} MATCHES "8" )
    set(ERT_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert-windows-x64")
  else()
    set(ERT_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert-windows")
  endif()
endif()

	
# The ERT binary distribution consists of four libraries, libwell, libecl,
# libutil and libgeometry and their accompanying header files. There has been a
# bit of mess of how this has been organized; either it has been on per
# directory basis as:
#
#    ${ERT_ROOT_PATH}/ecl/lib/libecl.so
#    ${ERT_ROOT_PATH}/ecl/include/ecl_xxx.h 
#
#    ${ERT_ROOT_PATH}/well/lib/libwell.so
#    ${ERT_ROOT_PATH}/well/include/well_xxx.h 
#    ....
#
# Or alternatively everything has been in common include/ and lib/ directories.
#
#    ${ERT_ROOT_PATH}/lib
#    ${ERT_ROOT_PATH}/include
#
# This is indirectly goverened by the ERT_XXX_PREFIX variables. If these are
# set to blank if everything will be collected from common include/ and lib/
# dierectories.
set( ERT_ECL_PREFIX  "ecl" CACHE STRING "Prefix path to use for ecl code in ert")
set( ERT_UTIL_PREFIX "util" CACHE STRING "Prefix path to use for util code in ert")
set( ERT_WELL_PREFIX "well" CACHE STRING "Prefix path to use for well code in ert")
set( ERT_GEO_PREFIX  "geometry" CACHE STRING "Prefix path to use for geometry code in ert")

mark_as_advanced( ERT_UTIL_PREFIX  ERT_ECL_PREFIX ERT_WELL_PREFIX ERT_GEO_PREFIX)

# Setting up include directories for ERT
set( ERT_INCLUDE_LIST 
     ${ERT_ROOT_PATH}/include 
     ${ERT_ROOT_PATH}/${ERT_ECL_PREFIX}/include 
     ${ERT_ROOT_PATH}/${ERT_UTIL_PREFIX}/include 
     ${ERT_ROOT_PATH}/${ERT_WELL_PREFIX}/include 
     ${ERT_ROOT_PATH}/${ERT_GEO_PREFIX}/include )

# Link to these ERT libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set( ERT_LIBRARY_LIST 
         ${ERT_ROOT_PATH}/lib/libecl.a
         ${ERT_ROOT_PATH}/lib/libert_util.a
         ${ERT_ROOT_PATH}/lib/libgeometry.a
         ${ERT_ROOT_PATH}/lib/libwell.a )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set( ERT_LIBRARY_LIST 
         ${ERT_ROOT_PATH}/${ERT_ECL_PREFIX}/lib/ecl.lib
         ${ERT_ROOT_PATH}/${ERT_UTIL_PREFIX}/lib/ert_util.lib
         ${ERT_ROOT_PATH}/${ERT_GEO_PREFIX}/lib/geometry.lib
         ${ERT_ROOT_PATH}/${ERT_WELL_PREFIX}/lib/well.lib )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   list(APPEND ERT_LIBRARY_LIST lapack z)  
endif()	

include_directories( ${ERT_INCLUDE_LIST} )
# Ert configuration complete
#-----------------------------------------------------------------




# Use all cpp and h files in the subdirectories
file( GLOB_RECURSE HEADER_FILES *.h )

list( APPEND CPP_SOURCES
    RIMain.cpp
    RIStdInclude.cpp
)

list( APPEND CPP_SOURCES
    Application/RIApplication.cpp
    Application/RIPreferences.cpp
)

list( APPEND CPP_SOURCES
    ModelVisualization/RivCellEdgeEffectGenerator.cpp
    ModelVisualization/RivGridPartMgr.cpp
    ModelVisualization/RivReservoirPartMgr.cpp
    ModelVisualization/RivReservoirViewPartMgr.cpp
    ModelVisualization/RivPipeGeometryGenerator.cpp
    ModelVisualization/RivReservoirPipesPartMgr.cpp
    ModelVisualization/RivWellPipesPartMgr.cpp
    ModelVisualization/RivWellHeadPartMgr.cpp
)

list( APPEND CPP_SOURCES
	FileInterface/RifEclipseInputFileTools.cpp
    FileInterface/RifEclipseOutputFileTools.cpp
    FileInterface/RifEclipseRestartFilesetAccess.cpp
    FileInterface/RifEclipseRestartDataAccess.cpp
    FileInterface/RifEclipseUnifiedRestartFileAccess.cpp
    FileInterface/RifReaderEclipseInput.cpp
    FileInterface/RifReaderEclipseOutput.cpp
    FileInterface/RifReaderMockModel.cpp
)

list( APPEND CPP_SOURCES
    SocketInterface/RiaSocketServer.cpp
)

list( APPEND CPP_SOURCES
    ProjectDataModel/RimCellFilter.cpp
    ProjectDataModel/RimCellPropertyFilter.cpp
    ProjectDataModel/RimCellPropertyFilterCollection.cpp
    ProjectDataModel/RimCellRangeFilter.cpp
    ProjectDataModel/RimCellRangeFilterCollection.cpp
    ProjectDataModel/RimDefines.cpp
    ProjectDataModel/RimLegendConfig.cpp
    ProjectDataModel/RimProject.cpp
    ProjectDataModel/RimReservoir.cpp
    ProjectDataModel/RimInputProperty.cpp
    ProjectDataModel/RimInputPropertyCollection.cpp
    ProjectDataModel/RimInputReservoir.cpp
    ProjectDataModel/RimResultReservoir.cpp
    ProjectDataModel/RimReservoirView.cpp
    ProjectDataModel/RimResultDefinition.cpp
    ProjectDataModel/RimResultSlot.cpp
    ProjectDataModel/RimCellEdgeResultSlot.cpp
    ProjectDataModel/RimWell.cpp
    ProjectDataModel/RimWellCollection.cpp
    ProjectDataModel/RimScriptCollection.cpp
    ProjectDataModel/RimCalcScript.cpp
    ProjectDataModel/RimExportInputPropertySettings.cpp
    ProjectDataModel/RimBinaryExportSettings.cpp
    ProjectDataModel/Rim3dOverlayInfoConfig.cpp
    ProjectDataModel/RimUiTreeModelPdm.cpp
    ProjectDataModel/RimUiTreeView.cpp
)

list( APPEND CPP_SOURCES
    ReservoirDataModel/RigCell.cpp
    ReservoirDataModel/RigGridBase.cpp
    ReservoirDataModel/RigReservoirCellResults.cpp 
    ReservoirDataModel/RigLocalGrid.cpp
    ReservoirDataModel/RigMainGrid.cpp
    ReservoirDataModel/RigReservoir.cpp
    ReservoirDataModel/RigReservoirBuilderMock.cpp
    ReservoirDataModel/RigWellResults.cpp
)

list( APPEND CPP_SOURCES
    UserInterface/RICursors.cpp
    UserInterface/RIMainWindow.cpp
    UserInterface/RIPreferencesDialog.cpp
    UserInterface/RIResultInfoPanel.cpp
    UserInterface/RIViewer.cpp
    UserInterface/RiuSimpleHistogramWidget.cpp

    UserInterface/RIProcessMonitor.cpp

)

# Define files for MOC-ing
set ( QT_MOC_HEADERS
    Application/RIApplication.h
    
    ProjectDataModel/RimUiTreeModelPdm.h
    ProjectDataModel/RimUiTreeView.h
    
    UserInterface/RIMainWindow.h
    UserInterface/RIPreferencesDialog.h
    UserInterface/RIResultInfoPanel.h
    UserInterface/RIViewer.h
    UserInterface/RIProcessMonitor.h
	SocketInterface/RiaSocketServer.h
)

qt4_wrap_cpp( MOC_FILES_CPP ${QT_MOC_HEADERS} )

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


#############################################################################
# creating PCH's for MSVC and GCC on Linux
#############################################################################

set( RAW_SOURCES ${CPP_SOURCES} )
list( REMOVE_ITEM RAW_SOURCES RIStdInclude.cpp)
list( REMOVE_ITEM RAW_SOURCES ReservoirDataModel/RigReaderInterfaceECL.cpp)
list( REMOVE_ITEM RAW_SOURCES ModelVisualization/RivCellEdgeEffectGenerator.cpp)
list( REMOVE_ITEM RAW_SOURCES ModelVisualization/RivPipeGeometryGenerator.cpp)
list( REMOVE_ITEM RAW_SOURCES ModelVisualization/RivWellPipesPartMgr.cpp)
list( REMOVE_ITEM RAW_SOURCES ModelVisualization/RivWellHeadPartMgr.cpp)


list( REMOVE_ITEM RAW_SOURCES     
    FileInterface/RifEclipseInputFileTools.cpp
    FileInterface/RifEclipseOutputFileTools.cpp
    FileInterface/RifEclipseRestartFilesetAccess.cpp
    FileInterface/RifEclipseRestartDataAccess.cpp
    FileInterface/RifEclipseUnifiedRestartFileAccess.cpp
    FileInterface/RifReaderEclipseInput.cpp
    FileInterface/RifReaderEclipseOutput.cpp
    UserInterface/RiuSimpleHistogramWidget.cpp

)

include( CustomPCH.cmake )
set( ALL_INCLUDES 
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibGuiQt_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}
    ${QT_INCLUDES}
)

set( PCH_NAME RIStdInclude )
set( GCC_PCH_TARGET gccPCH )
set( PCH_COMPILER_DEFINE EMPTY )
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set( PCH_COMPILER_DEFINE CVF_LINUX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

precompiled_header( RAW_SOURCES ALL_INCLUDES ${GCC_PCH_TARGET} ${PCH_NAME} ${PCH_COMPILER_DEFINE} )

################################################################################



add_executable(ResInsight
    ${CPP_SOURCES}
    ${MOC_FILES_CPP}
    ${QRC_FILES_CPP}
	${HEADER_FILES}
)


set( LINK_LIBRARIES
    cafPdmCvf
    cafUserInterface
    cafProjectDataModel
    cafViewer
    cafAnimControl
    CommonCode
    LibGuiQt
    LibViewing
    LibRender
    LibGeometry
    LibCore
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
)
set( EXTERNAL_LINK_LIBRARIES ${ERT_LIBRARY_LIST} )
target_link_libraries( ResInsight ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})

# Copy Dlls
if (MSVC)

    # Qt DLLs
    set (QTLIBLIST QtCore QtCored QtGui QtGuid QtOpenGl QtOpenGld QtNetwork QtNetworkd)
    foreach (qtlib ${QTLIBLIST})
        add_custom_command(TARGET ResInsight POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_BINARY_DIR}/${qtlib}4.dll"
                ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)
    endforeach( qtlib )


    # DLLs ERT depends on
#    add_custom_command(TARGET ResInsight POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#            "${PROJECT_SOURCE_DIR}/../ThirdParty/Ert-windows/bin/"
#            ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)


     # ERT DLLs
#     set (ERT_MODULES ecl geometry util well)
#     foreach (ert_module ${ERT_MODULES})
#         add_custom_command(TARGET ResInsight POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                 "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert-windows/${ert_module}/lib/lib${ert_module}.dll"
#                 ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)
#     endforeach()

endif(MSVC)

#############################################################################
# Install 
#############################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(ResInsight PROPERTIES INSTALL_RPATH "${RESINSIGHT_FINAL_INSTALL_PATH};.")

    # Find Qt libraries and sym links
    file (GLOB RESINSIGHT_FILES
            ${QT_LIBRARY_DIR}/libQtCore.so*
            ${QT_LIBRARY_DIR}/libQtGui.so*
            ${QT_LIBRARY_DIR}/libQtOpenGL.so*
            ${QT_LIBRARY_DIR}/libQtNetwork.so*
        )

endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (RESINSIGHT_FILES
            ${QT_LIBRARY_DIR}/QtCore4.dll
            ${QT_LIBRARY_DIR}/QtGui4.dll
            ${QT_LIBRARY_DIR}/QtOpenGL4.dll
            ${QT_LIBRARY_DIR}/QtNetwork4.dll
        )
endif()

set (RESINSIGHT_FILES ${RESINSIGHT_FILES} 
            ${CMAKE_CURRENT_SOURCE_DIR}/Adm/LicenseInformation.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/Adm/gplLicense.txt
    )
        

install(TARGETS ResInsight DESTINATION ${RESINSIGHT_FINAL_NAME})

install(FILES ${RESINSIGHT_FILES} DESTINATION ${RESINSIGHT_FINAL_NAME} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resinsight DESTINATION ${RESINSIGHT_FINAL_NAME} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )



IF(${CMAKE_SYSTEM_NAME} MATCHES "No_Linux")
install(CODE "
    set (INSTALLFILE_LIST
            ${CMAKE_CURRENT_BINARY_DIR}/ResInsight
			${CMAKE_CURRENT_SOURCE_DIR}/Adm/LicenseInformation.txt
			${CMAKE_CURRENT_SOURCE_DIR}/Adm/gplLicense.txt
            ${QT_LIBRARY_DIR}/libQtCore.so.4
            ${QT_LIBRARY_DIR}/libQtGui.so.4
            ${QT_LIBRARY_DIR}/libQtOpenGL.so.4
            /usr/lib64/libgfortran.so.1
            /usr/lib64/liblapack.so.3
            /usr/lib64/libblas.so.3
    )
    execute_process(COMMAND rm -r ${CMAKE_BINARY_DIR}/Install/ResInsight )
    execute_process(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/Install/ResInsight )
    foreach (installFile \${INSTALLFILE_LIST})
        execute_process(COMMAND cp -v \${installFile} ${CMAKE_BINARY_DIR}/Install/ResInsight)
    endforeach (installFile)

    execute_process(COMMAND bash -c \"cd ${CMAKE_BINARY_DIR}/Install/ ; tar cvzf ResInsight_${STRPRODUCTVER}-bin.tar.gz ResInsight/*\" )

")
endif()
