cmake_minimum_required (VERSION 2.8)

project (RifOdbReader)

add_definitions(-DHKS_NT)
add_definitions(-DABQ_WIN86_64)


include_directories(
	${RESINSIGHT_ODB_API_DIR}/include
	${RigGeoMechDataModel_SOURCE_DIR}
	${LibCore_SOURCE_DIR}
)

add_library( ${PROJECT_NAME}
	RifOdbReader.h
	RifOdbReader.cpp
	RifGeoMechReaderInterface.h
	RifGeoMechReaderInterface.cpp
	OdbSetup.cmake
)

list(APPEND RI_ODB_LIBS
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAOdbDdbOdb.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAOdbApi.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAOdbCore.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAOdbCoreGeom.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAOdbAttrEO.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAAbuBasicUtils.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMABasShared.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMABasCoreUtils.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAStiCAE_StableTime.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMABasMem.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAAbuGeom.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMARomDiagEx.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMASspUmaCore.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMASimInterface.lib 
${RESINSIGHT_ODB_API_DIR}/lib/ABQSMAMtxCoreModule.lib
)

target_link_libraries(${PROJECT_NAME} ${RI_ODB_LIBS} RigGeoMechDataModel LibCore)
