cmake_minimum_required (VERSION 2.8)

project (RifOdbReader)

if (MSVC)
	add_definitions(-DHKS_NT)
	add_definitions(-DABQ_WIN86_64)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -w -Wno-deprecated")

	add_definitions(-DTYPENAME=typename)
	add_definitions(-D_LINUX_SOURCE)
	add_definitions(-DABQ_LINUX)
	add_definitions(-DABQ_LNX86_64)
	add_definitions(-DSMA_GNUC)
	add_definitions(-DFOR_TRAIL)
	add_definitions(-DHAS_BOOL)
	add_definitions(-DASSERT_ENABLED)
	add_definitions(-D_BSD_TYPES)
	add_definitions(-D_BSD_SOURCE)
	add_definitions(-D_GNU_SOURCE)
	add_definitions(-D_POSIX_SOURCE)
	add_definitions(-D_XOPEN_SOURCE_EXTENDED)
	add_definitions(-D_XOPEN_SOURCE)
	add_definitions(-DHAVE_OPENGL)
	add_definitions(-DHKS_OPEN_GL)
	add_definitions(-DGL_GLEXT_PROTOTYPES)
	add_definitions(-DMULTI_THREADING_ENABLED)
	add_definitions(-D_REENTRANT)
	add_definitions(-DABQ_MPI_SUPPORT)
	add_definitions(-DBIT64)
	add_definitions(-D_LARGEFILE64_SOURCE)
	add_definitions(-D_FILE_OFFSET_BITS=64)
	add_definitions(-DABQ_MPI_PMPI)
endif(MSVC)

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

if (MSVC)
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
else()
	list(APPEND RI_ODB_LIBS
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAOdbDdbOdb.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAOdbApi.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAOdbCore.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAOdbCoreGeom.so 
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAOdbAttrEO.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAAbuBasicUtils.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMABasShared.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMABasCoreUtils.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAStiCAE_StableTime.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMABasMem.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAAbuGeom.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMARomDiagEx.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMASspUmaCore.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMASimInterface.so
		${RESINSIGHT_ODB_API_DIR}/lib/libABQSMAMtxCoreModule.so
	)
endif(MSVC)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} ${RI_ODB_LIBS} RigGeoMechDataModel LibCore)

source_group("" FILES ${PROJECT_FILES})
