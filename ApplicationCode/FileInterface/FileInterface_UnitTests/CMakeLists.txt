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

    ${ResInsight_SOURCE_DIR}/ThirdParty/Ert/ecl/include
    ${ResInsight_SOURCE_DIR}/ThirdParty/Ert/util/include
    ${ResInsight_SOURCE_DIR}/ThirdParty/Ert/well/include

)

set( FILEINTERFACE_CPP_SOURCES
    ../RifReaderEclipseFileAccess.cpp
    ../RifReaderEclipseRestartFiles.cpp
    ../RifReaderEclipseResultAccess.cpp
    ../RifReaderEclipseUnifiedRestartFile.cpp
    ../RifReaderInterfaceEcl.cpp
    ../RifReaderInterfaceMock.cpp
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
)

set( CPP_SOURCES
    ${FILEINTERFACE_CPP_SOURCES}
    ${RESERVOIRDATAMODEL_CPP_SOURCES}
)

source_group( "FileInterface"       FILES ${FILEINTERFACE_CPP_SOURCES} )
source_group( "ReservoirDataModel"  FILES ${RESERVOIRDATAMODEL_CPP_SOURCES} )

set( UNIT_TEST_CPP_SOURCES
    main.cpp
    RifReaderInterfaceEcl-Test.cpp
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



IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( LINUX_LINK_LIBRARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/Ert/ecl/lib/libecl.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/Ert/util/lib/libutil.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/Ert/well/lib/libwell.a
        lapack
    )

        # Linux specific code
        set(CMAKE_CXX_FLAGS 		"-DCVF_LINUX -DUSE_ECL_LIB -pipe -Wextra -Woverloaded-virtual -Wformat")
        set(CMAKE_CXX_FLAGS_DEBUG 	"-g -DDEBUG -D_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


target_link_libraries( ${ProjectName} ${LINK_LIBRARIES} ${LINUX_LINK_LIBRARIES})


# Copy Qt Dlls
if (MSVC)
    set (QTLIBLIST QtCore QtGui QtOpenGl)
    foreach (qtlib ${QTLIBLIST})
        
        # Debug
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}d4.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug/${qtlib}d4.dll)
        
        # Release
        execute_process(COMMAND cmake -E copy_if_different ${QT_BINARY_DIR}/${qtlib}4.dll ${CMAKE_CURRENT_BINARY_DIR}/Release/${qtlib}4.dll)
    endforeach( qtlib )
endif(MSVC)
