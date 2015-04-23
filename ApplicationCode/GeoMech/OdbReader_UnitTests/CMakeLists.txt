cmake_minimum_required (VERSION 2.8)

project ( OdbReader_UnitTests )

set(RI_ODB_API_DIR C:/pfRoot/jjsOnJacobpcCsdep/User/Sigurd/OdbApiExperiments/OdbApi/x64 CACHE PATH "Path tho the ODB api from Simulia")
set(RI_VIZ_FWK_ROOT ../../../Fwk/main/VizFwk  CACHE PATH "Path to VizFwk")
set(RI_GTEST_ROOT ../../../ThirdParty  CACHE PATH "Path to forlder containing gtest folder")

include(${RI_VIZ_FWK_ROOT}/CMake/Utils/ceeDetermineCompilerFlags.cmake)

add_subdirectory(${RI_VIZ_FWK_ROOT}/LibCore buildVizFwk)
add_subdirectory(../GeoMechDataModel buildGeoMechDataModel)
add_subdirectory(../OdbReader buildOdbReader)

include_directories(${RI_ODB_API_DIR}/include)
include_directories(${RI_VIZ_FWK_ROOT}/LibCore)
include_directories(../GeoMechDataModel)
include_directories(../OdbReader)
include_directories(${RI_GTEST_ROOT})

set( UNIT_TEST_CPP_SOURCES
    main.cpp
    RifOdbReader-Test.cpp
	${RI_GTEST_ROOT}/gtest/gtest-all.cc
)


set( LINK_LIBRARIES
    LibCore
	RigGeoMechDataModel
	RifOdbReader
)

list(APPEND RI_ODB_LIBS
${RI_ODB_API_DIR}/lib/ABQSMAOdbDdbOdb.lib 
${RI_ODB_API_DIR}/lib/ABQSMAOdbApi.lib 
${RI_ODB_API_DIR}/lib/ABQSMAOdbCore.lib 
${RI_ODB_API_DIR}/lib/ABQSMAOdbCoreGeom.lib 
${RI_ODB_API_DIR}/lib/ABQSMAOdbAttrEO.lib 
${RI_ODB_API_DIR}/lib/ABQSMAAbuBasicUtils.lib 
${RI_ODB_API_DIR}/lib/ABQSMABasShared.lib 
${RI_ODB_API_DIR}/lib/ABQSMABasCoreUtils.lib 
${RI_ODB_API_DIR}/lib/ABQSMAStiCAE_StableTime.lib 
${RI_ODB_API_DIR}/lib/ABQSMABasMem.lib 
${RI_ODB_API_DIR}/lib/ABQSMAAbuGeom.lib 
${RI_ODB_API_DIR}/lib/ABQSMARomDiagEx.lib 
${RI_ODB_API_DIR}/lib/ABQSMASspUmaCore.lib 
${RI_ODB_API_DIR}/lib/ABQSMASimInterface.lib 
${RI_ODB_API_DIR}/lib/ABQSMAMtxCoreModule.lib
)

set( EXTERNAL_LINK_LIBRARIES  ${RI_ODB_LIBS})

add_executable( ${PROJECT_NAME} ${UNIT_TEST_CPP_SOURCES} )
target_link_libraries( ${PROJECT_NAME} ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES})

# Copy Dlls
if (MSVC)
	file (GLOB  ABA_DLLS_FULL  ${RI_ODB_API_DIR}/lib/*.dll)
	
	foreach (aDLL  ${ABA_DLLS_FULL})
		 get_filename_component(filenameWithExt ${aDLL} NAME)
		 list(APPEND ABA_DLLS ${filenameWithExt} )
	endforeach(aDLL)
 


	foreach (aDLL ${ABA_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different  "${RI_ODB_API_DIR}/lib/${aDLL}"  "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()

endif(MSVC)
