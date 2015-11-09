cmake_minimum_required (VERSION 2.8)

SET (ProjectName ModelVisualization_UnitTests)
project ( ${ProjectName} )

include_directories(
    ${LibCore_SOURCE_DIR}
    ${LibGeometry_SOURCE_DIR}
    ${LibRender_SOURCE_DIR}
    ${LibViewing_SOURCE_DIR}

    ${ResInsight_SOURCE_DIR}/ThirdParty

    ${ResInsight_SOURCE_DIR}/ApplicationCode/ModelVisualization
    
    ${CommonCode_SOURCE_DIR}
)

set( MODEL_VISUALIZATION_CPP_SOURCES
    ../RivPipeGeometryGenerator.cpp
    ../RivTernaryScalarMapper.cpp
)


set( CPP_SOURCES
    ${MODEL_VISUALIZATION_CPP_SOURCES}
)

set( UNIT_TEST_CPP_SOURCES
    main.cpp
    RivPipeGeometryGenerator-Test.cpp
	RivTernaryScalarMapper-Test.cpp
)


set( LINK_LIBRARIES
    LibViewing
    LibRender
    LibGeometry
    LibCore
    LibGuiQt

    CommonCode

    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}

)


add_executable( ${ProjectName}
    ${CPP_SOURCES}
    ${UNIT_TEST_CPP_SOURCES}

    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set( EXTERNAL_LINK_LIBRARIES
        pthread
    )
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set( CMAKE_CXX_FLAGS
        "-DGTEST_USE_OWN_TR1_TUPLE=1"
    )
ENDIF()

target_link_libraries( ${ProjectName} ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})


