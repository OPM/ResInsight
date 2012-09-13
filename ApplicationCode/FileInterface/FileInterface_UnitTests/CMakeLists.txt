cmake_minimum_required (VERSION 2.8)

SET (ProjectName FileInterface_UnitTests)
project ( ${ProjectName} )

# Qt
find_package (Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
include (${QT_USE_FILE})

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}

    ${ResInsight_SOURCE_DIR}/ApplicationCode
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel
    ${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ProjectDataModel
    ${ResInsight_SOURCE_DIR}/ThirdParty

    ${ResInsight_SOURCE_DIR}/cafProjectDataModel

    ${ResInsight_SOURCE_DIR}/CommonCode
)

#-----------------------------------------------------------------
# Ert configuration
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(ERT_ROOT_PATH "${ResInsight_SOURCE_DIR}/ThirdParty/Ert")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (${CMAKE_SIZEOF_VOID_P} MATCHES "8" )
    set(ERT_ROOT_PATH "${ResInsight_SOURCE_DIR}/ThirdParty/Ert-windows-x64")
  else()
    set(ERT_ROOT_PATH "${ResInsight_SOURCE_DIR}/ThirdParty/Ert-windows")
  endif()
endif()

set( ERT_ECL_PREFIX  "ecl" CACHE STRING "Prefix path to use for ecl code in ert")
set( ERT_UTIL_PREFIX "util" CACHE STRING "Prefix path to use for util code in ert")
set( ERT_WELL_PREFIX "well" CACHE STRING "Prefix path to use for well code in ert")

set( ERT_INCLUDE_LIST 
     ${ERT_ROOT_PATH}/include 
     ${ERT_ROOT_PATH}/${ERT_ECL_PREFIX}/include 
     ${ERT_ROOT_PATH}/${ERT_UTIL_PREFIX}/include 
     ${ERT_ROOT_PATH}/${ERT_WELL_PREFIX}/include )

# Link to these ERT libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set( ERT_LIBRARY_LIST 
         ${ERT_ROOT_PATH}/lib/libecl.a
         ${ERT_ROOT_PATH}/lib/libert_util.a
         ${ERT_ROOT_PATH}/lib/libgeometry.a
         ${ERT_ROOT_PATH}/lib/libwell.a )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if (${CMAKE_SIZEOF_VOID_P} MATCHES "8" )
    set( ERT_LIBRARY_LIST 
         ${ERT_ROOT_PATH}/${ERT_ECL_PREFIX}/lib/ecl.lib
         ${ERT_ROOT_PATH}/${ERT_UTIL_PREFIX}/lib/ert_util.lib
         ${ERT_ROOT_PATH}/${ERT_GEO_PREFIX}/lib/geometry.lib
         ${ERT_ROOT_PATH}/${ERT_WELL_PREFIX}/lib/well.lib )
  else()
    set( ERT_LIBRARY_LIST 
         ${ERT_ROOT_PATH}/${ERT_ECL_PREFIX}/lib/libecl.lib
         ${ERT_ROOT_PATH}/${ERT_UTIL_PREFIX}/lib/libutil.lib
         ${ERT_ROOT_PATH}/${ERT_GEO_PREFIX}/lib/libgeometry.lib
         ${ERT_ROOT_PATH}/${ERT_WELL_PREFIX}/lib/libwell.lib )
  endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   list(APPEND ERT_LIBRARY_LIST lapack z)  
endif()	
#-----------------------------------------------------------------

include_directories( ${ERT_INCLUDE_LIST} )

set( FILEINTERFACE_CPP_SOURCES
    ../RifEclipseInputFileTools.cpp
    ../RifEclipseOutputFileTools.cpp
    ../RifEclipseRestartFilesetAccess.cpp
    ../RifEclipseRestartDataAccess.cpp
    ../RifEclipseUnifiedRestartFileAccess.cpp
    ../RifReaderEclipseOutput.cpp
    ../RifReaderEclipseInput.cpp
    ../RifReaderMockModel.cpp
)

set( RESERVOIRDATAMODEL_CPP_SOURCES
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigCell.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigGridBase.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigLocalGrid.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigMainGrid.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigReservoir.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigReservoirBuilderMock.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigReservoirCellResults.cpp
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/RigWellResults.cpp
    ${ResInsight_SOURCE_DIR}/cafUserInterface/cafProgressInfo.cpp
)

set( CPP_SOURCES
    ${FILEINTERFACE_CPP_SOURCES}
    ${RESERVOIRDATAMODEL_CPP_SOURCES}
)

source_group( "FileInterface"       FILES ${FILEINTERFACE_CPP_SOURCES} )
source_group( "ReservoirDataModel"  FILES ${RESERVOIRDATAMODEL_CPP_SOURCES} )

set( UNIT_TEST_CPP_SOURCES
    main.cpp
    RifReaderEclipseOutput-Test.cpp
    Ert-Test.cpp
)


set( LINK_LIBRARIES
    CommonCode

    LibViewing
    LibRender
    LibGeometry
    LibCore

    ${QT_LIBRARIES}
)



add_executable( ${ProjectName}
    ${CPP_SOURCES}
    ${UNIT_TEST_CPP_SOURCES}

    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
)

set( EXTERNAL_LINK_LIBRARIES ${ERT_LIBRARY_LIST} )
target_link_libraries( ${ProjectName} ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})



# Copy Dlls
if (MSVC)

    # Qt DLLs
    set (QTLIBLIST QtCore QtCored QtGui QtGuid QtOpenGl QtOpenGld QtNetwork QtNetworkd)
    foreach (qtlib ${QTLIBLIST})
        add_custom_command(TARGET ${ProjectName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_BINARY_DIR}/${qtlib}4.dll"
                ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)
    endforeach( qtlib )


    # DLLs ERT depends on
    add_custom_command(TARGET ${ProjectName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/../../../ThirdParty/Ert-windows/bin/"
            ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)


     # ERT DLLs
     set (ERT_MODULES ecl geometry util well)
     foreach (ert_module ${ERT_MODULES})
         add_custom_command(TARGET ${ProjectName} POST_BUILD
             COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 "${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/Ert-windows/${ert_module}/lib/lib${ert_module}.dll"
                 ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)
     endforeach()

endif(MSVC)
