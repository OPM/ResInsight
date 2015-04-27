cmake_minimum_required (VERSION 2.8)

project (RigGeoMechDataModel)

include_directories(
	${LibCore_SOURCE_DIR}
)


add_library( ${PROJECT_NAME}
	RigFemPart.h
	RigFemPart.cpp
	RigFemTypes.h
	RigFemTypes.cpp
	RigGeoMechCaseData.cpp
	RigGeoMechCaseData.h
	RigFemPartCollection.cpp
	RigFemPartCollection.h

)

target_link_libraries( ${PROJECT_NAME} LibCore )