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
    ReservoirDataModel/RigGridScalarDataAccess.cpp
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
list( REMOVE_ITEM RAW_SOURCES ReservoirDataModel/RigGridScalarDataAccess.cpp)
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

    ecl
    ert_util
    ert_geometry
    ecl_well

    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
)
set( EXTERNAL_LINK_LIBRARIES ${ERT_LIBRARY_LIST} )

# According to ivarun this is needed on OpenSuse, and Fedora. See: https://github.com/OPM/ResInsight/pull/7
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( EXTERNAL_LINK_LIBRARIES
		${EXTERNAL_LINK_LIBRARIES}
		rt
    )
endif()

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
            ${QT_BINARY_DIR}/QtCore4.dll
            ${QT_BINARY_DIR}/QtGui4.dll
            ${QT_BINARY_DIR}/QtOpenGL4.dll
            ${QT_BINARY_DIR}/QtNetwork4.dll
        )
endif()

set (RESINSIGHT_FILES ${RESINSIGHT_FILES} 
            ${CMAKE_CURRENT_SOURCE_DIR}/Adm/LicenseInformation.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/Adm/gplLicense.txt
    )
        

install(TARGETS ResInsight DESTINATION ${RESINSIGHT_FINAL_NAME})

install(FILES ${RESINSIGHT_FILES} DESTINATION ${RESINSIGHT_FINAL_NAME} )

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resinsight DESTINATION ${RESINSIGHT_FINAL_NAME} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
endif()

