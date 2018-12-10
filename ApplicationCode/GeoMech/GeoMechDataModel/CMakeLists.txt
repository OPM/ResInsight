cmake_minimum_required (VERSION 2.8.12)

project (RigGeoMechDataModel)

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
        RimFemResultObserver.h
        RimFemResultObserver.cpp
        RimGeoMechGeometrySelectionItem.h
        RimGeoMechGeometrySelectionItem.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set( LINK_LIBRARIES
  LibCore 
  
  cafPdmCvf
  cafTensor 
  cafUserInterface
  CommonCode

  ResultStatisticsCache 
)

target_link_libraries( ${PROJECT_NAME}
  ${LINK_LIBRARIES}
)

source_group("" FILES ${PROJECT_FILES})
