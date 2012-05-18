cmake_minimum_required (VERSION 2.8)

project (ApplicationCode)

CONFIGURE_FILE(  ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RIVersionInfo.h.cmake  ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RIVersionInfo.h)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/ecl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/util/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/geometry/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/well/include
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
    FileInterface/RifReaderEclipseFileAccess.cpp
    FileInterface/RifReaderEclipseRestartFiles.cpp
    FileInterface/RifReaderEclipseResultAccess.cpp
    FileInterface/RifReaderEclipseUnifiedRestartFile.cpp
    FileInterface/RifReaderInterfaceEcl.cpp
    FileInterface/RifReaderInterfaceMock.cpp
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
    ProjectDataModel/RimReservoirView.cpp
    ProjectDataModel/RimResultDefinition.cpp
    ProjectDataModel/RimResultSlot.cpp
    ProjectDataModel/RimCellEdgeResultSlot.cpp
    ProjectDataModel/RimUiTreeModelPdm.cpp
    ProjectDataModel/RimWell.cpp
    ProjectDataModel/RimWellCollection.cpp
    ProjectDataModel/RimScriptCollection.cpp
    ProjectDataModel/RimCalcScript.cpp
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
    UserInterface/RIProcessMonitor.cpp

)

# Define files for MOC-ing
set ( QT_MOC_HEADERS
    Application/RIApplication.h
    ProjectDataModel/RimUiTreeModelPdm.h
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
    FileInterface/RifReaderEclipseFileAccess.cpp
    FileInterface/RifReaderEclipseRestartFiles.cpp
    FileInterface/RifReaderEclipseResultAccess.cpp
    FileInterface/RifReaderEclipseUnifiedRestartFile.cpp
    FileInterface/RifReaderInterfaceEcl.cpp
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
    cafProjectDataModel
    cafPdmCvf
    cafUserInterface
	cafViewer
	cafAnimControl
    CommonCode
    QtPropertyBrowser
    LibGuiQt
    LibViewing
    LibRender
    LibGeometry
    LibCore
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( LINUX_LINK_LIBRARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/ecl/lib/libecl.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/util/lib/libutil.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/geometry/lib/libgeometry.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Ert/well/lib/libwell.a
        lapack
    )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries( ResInsight ${LINK_LIBRARIES} ${LINUX_LINK_LIBRARIES})

# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore QtGui QtOpenGl QtNetwork)
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)

		# Release With debug info
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/${qtlib}4.dll)

    endforeach( qtlib )
endif(MSVC)

#############################################################################
# Install 
#############################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(ResInsight PROPERTIES INSTALL_RPATH "${RESINSIGHT_FINAL_INSTALL_PATH};.")
    set (RESINSIGHT_FILES
            ${QT_LIBRARY_DIR}/libQtCore.so.4.7.3
            ${QT_LIBRARY_DIR}/libQtGui.so.4.7.3
            ${QT_LIBRARY_DIR}/libQtOpenGL.so.4.7.3
            ${QT_LIBRARY_DIR}/libQtNetwork.so.4.7.3
            ${QT_LIBRARY_DIR}/libQtCore.so.4
            ${QT_LIBRARY_DIR}/libQtGui.so.4
            ${QT_LIBRARY_DIR}/libQtOpenGL.so.4
            ${QT_LIBRARY_DIR}/libQtNetwork.so.4           
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
