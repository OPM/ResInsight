cmake_minimum_required (VERSION 2.8)

SET (ProjectName FileInterface_UnitTests)
project ( ${ProjectName} )


# Qt
find_package (Qt4 COMPONENTS QtCore QtGui QtMain QtOpenGl REQUIRED)
include (${QT_USE_FILE})

include_directories(
    ${ResInsight_SOURCE_DIR}/ApplicationCode
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel
    ${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ProjectDataModel
    ${ResInsight_SOURCE_DIR}/ThirdParty

    ${ResInsight_SOURCE_DIR}/Fwk/AppFwk/cafProjectDataModel

    ${ResInsight_SOURCE_DIR}/Fwk/AppFwk/CommonCode

	#Remove when RigStatistics is out
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ModelVisualization
)


# Populate variables from read from CMakeLists_files.cmake
set (CODE_SOURCE_FILES )
include ("${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface/CMakeLists_files.cmake")
set( CPP_SOURCES
    ${CPP_SOURCES}
    ${CODE_SOURCE_FILES}
)
source_group( "FileInterface"       FILES ${CODE_SOURCE_FILES} )


# Populate variables from read from CMakeLists_files.cmake
set (CODE_SOURCE_FILES )
include ("${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/CMakeLists_files.cmake")
set( CPP_SOURCES
    ${CPP_SOURCES}
    ${CODE_SOURCE_FILES}
)
source_group( "ReservoirDataModel"       FILES ${CODE_SOURCE_FILES} )


set( CPP_SOURCES
    ${CPP_SOURCES}

    ${ResInsight_SOURCE_DIR}/Fwk/AppFwk/cafUserInterface/cafProgressInfo.cpp
)

source_group( "Headers"             FILES ${CODE_HEADER_FILES} )

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

    ecl
    ert_util
    ert_geometry
    ecl_well

    ${QT_LIBRARIES}
)

add_executable( ${ProjectName}
    ${CODE_HEADER_FILES}

    ${CPP_SOURCES}
    ${UNIT_TEST_CPP_SOURCES}

    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc

    ${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface/CMakeLists_files.cmake
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel/CMakeLists_files.cmake
)

set( EXTERNAL_LINK_LIBRARIES ${ERT_LIBRARY_LIST} )
target_link_libraries( ${ProjectName} ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})


# Copy Dlls
if (MSVC)

    # Qt DLLs
    set (QTLIBLIST QtCore QtCored QtGui QtGuid QtOpenGl QtOpenGld QtNetwork QtNetworkd QtScript QtScriptd QtScriptTools QtScriptToolsd)
    foreach (qtlib ${QTLIBLIST})
        add_custom_command(TARGET ${ProjectName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_BINARY_DIR}/${qtlib}4.dll"
                ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>)
    endforeach( qtlib )

endif(MSVC)
