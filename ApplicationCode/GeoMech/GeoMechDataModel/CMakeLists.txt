cmake_minimum_required (VERSION 2.8)

project (RigGeoMechDataModel)
find_package(Qt4 COMPONENTS QtCore)
include(${QT_USE_FILE})

include_directories(
	${LibCore_SOURCE_DIR}
        ${cafTensor_SOURCE_DIR}
	../../ResultStatisticsCache
	../OdbReader
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
	RigFemPartResults.h
	RigFemPartResults.cpp
	RigFemPartResultsCollection.h
	RigFemPartResultsCollection.cpp
	RigFemScalarResultFrames.h
	RigFemScalarResultFrames.cpp
	RigFemNativeStatCalc.h
	RigFemNativeStatCalc.cpp
	RigFemNativeVisibleCellsStatCalc.h
	RigFemNativeVisibleCellsStatCalc.cpp
	RigFemFaceComparator.h
	RigFemPartGrid.h
	RigFemPartGrid.cpp
	RigFemResultAddress.h
	RigFemResultPosEnum.h
    RimGeoMechGeometrySelectionItem.h
    RimGeoMechGeometrySelectionItem.cpp
)

target_link_libraries( ${PROJECT_NAME} LibCore cafTensor ResultStatisticsCache)
