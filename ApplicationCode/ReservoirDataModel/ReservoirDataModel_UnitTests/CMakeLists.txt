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
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ReservoirDataModel
    ${ResInsight_SOURCE_DIR}/ApplicationCode/FileInterface
    ${ResInsight_SOURCE_DIR}/ApplicationCode/ProjectDataModel
    ${ResInsight_SOURCE_DIR}/ThirdParty

    ${ResInsight_SOURCE_DIR}/cafProjectDataModel

    ${ResInsight_SOURCE_DIR}/CommonCode
)

file( GLOB CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp )
file( GLOB UNIT_TEST_CPP_SOURCES *.cpp )


set( LINK_LIBRARIES
    CommonCode

    LibViewing
    LibRender
    LibGeometry
    LibCore

    ecl
    well
    ert_util

    ${QT_LIBRARIES}
)


add_executable( ${ProjectName}
    ${CPP_SOURCES}
    ${UNIT_TEST_CPP_SOURCES}

    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
)



IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set ( LINUX_LINK_LIBRARIES
        lapack
        pthread
    )

        # Linux specific code
        set(CMAKE_CXX_FLAGS 		"-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat")
        set(CMAKE_CXX_FLAGS_DEBUG 	"-g -DDEBUG -D_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

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
