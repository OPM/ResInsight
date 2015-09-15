cmake_minimum_required (VERSION 2.8)

SET (ProjectName RigReservoirDataModel_UnitTests)
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
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ResultStatisticsCache
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel
    ${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ProjectDataModel
    ${ResInsight_SOURCE_DIR}/ThirdParty
    ${ResInsight_SOURCE_DIR}/ThirdParty/NRLib/nrlib/well

    #${ResInsight_SOURCE_DIR}/Fwk/AppFwk/cafProjectDataModel

	${cafProjectDataModel_SOURCE_DIR}
	${cafPdmCore_SOURCE_DIR}

    ${ResInsight_SOURCE_DIR}/Fwk/AppFwk/CommonCode

	#Remove when RigStatistics is out
    #${ResInsight_SOURCE_DIR}/ApplicationCode/ModelVisualization
)

# Populate the filenames into variable lists
include ("${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists_files.cmake")


set( UNIT_TEST_CPP_SOURCES

    ${ResInsight_SOURCE_DIR}/Fwk/AppFwk/cafUserInterface/cafProgressInfo.cpp

    main.cpp
    RigActiveCellInfo-Test.cpp
    RigReservoir-Test.cpp
	RigStatisticsMath-Test.cpp
	cvfGeometryTools-Test.cpp
)


#############################################################################
# Adds folders for Visual Studio solution explorer (and for Xcode explorer)
#############################################################################
source_group( "UnitTests"       FILES ${UNIT_TEST_CPP_SOURCES} )



set( LINK_LIBRARIES
    CommonCode

	ResultStatisticsCache

    LibViewing
    LibRender
    LibGeometry
    LibCore

    ecl
    ecl_well
    ert_util

	NRLib

    ${QT_LIBRARIES}
)


add_executable( ${ProjectName}
    ${CODE_SOURCE_FILES}
    ${UNIT_TEST_CPP_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists_files.cmake

    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
)



IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( LINUX_LINK_LIBRARIES
        pthread
    )

        # Linux specific code
        set(CMAKE_CXX_FLAGS 		"-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat")
        set(CMAKE_CXX_FLAGS_DEBUG 	"-g -DDEBUG -D_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS         "-DGTEST_USE_OWN_TR1_TUPLE=1")
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
