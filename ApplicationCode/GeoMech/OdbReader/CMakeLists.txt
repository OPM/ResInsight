cmake_minimum_required (VERSION 2.8)

project (RifOdbReader)

add_definitions(-DHKS_NT)
add_definitions(-DABQ_WIN86_64)


include_directories(
	${RI_ODB_API_DIR}/include
	${RigGeoMechDataModel_SOURCE_DIR}
	${LibCore_SOURCE_DIR}
)

add_library( ${PROJECT_NAME}
	RifOdbReader.h
	RifOdbReader.cpp
	RifGeoMechReaderInterface.h
	RifGeoMechReaderInterface.cpp
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

target_link_libraries(${PROJECT_NAME} ${RI_ODB_LIBS} RigGeoMechDataModel LibCore)
