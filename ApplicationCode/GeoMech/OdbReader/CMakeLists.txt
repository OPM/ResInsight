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
