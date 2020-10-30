cmake_minimum_required(VERSION 3.12)

project(RigGeoMechDataModel)

# Unity Build
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

add_library(
  ${PROJECT_NAME}
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
  RigFemClosestResultIndexCalculator.h
  RigFemClosestResultIndexCalculator.cpp
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
  RigHexGradientTools.h
  RigHexGradientTools.cpp
  RigFemPartResultCalculator.h
  RigFemPartResultCalculator.cpp
  RigFemPartResultCalculatorNormalSE.h
  RigFemPartResultCalculatorNormalSE.cpp
  RigFemPartResultCalculatorShearSE.h
  RigFemPartResultCalculatorShearSE.cpp
  RigFemPartResultCalculatorNormalST.h
  RigFemPartResultCalculatorNormalST.cpp
  RigFemPartResultCalculatorShearST.h
  RigFemPartResultCalculatorShearST.cpp
  RigFemPartResultCalculatorTimeLapse.h
  RigFemPartResultCalculatorTimeLapse.cpp
  RigFemPartResultCalculatorGamma.h
  RigFemPartResultCalculatorGamma.cpp
  RigFemPartResultCalculatorNormalized.h
  RigFemPartResultCalculatorNormalized.cpp
  RigFemPartResultCalculatorSurfaceAngles.h
  RigFemPartResultCalculatorSurfaceAngles.cpp
  RigFemPartResultCalculatorSurfaceAlignedStress.h
  RigFemPartResultCalculatorSurfaceAlignedStress.cpp
  RigFemPartResultCalculatorPrincipalStrain.h
  RigFemPartResultCalculatorPrincipalStrain.cpp
  RigFemPartResultCalculatorPrincipalStress.h
  RigFemPartResultCalculatorPrincipalStress.cpp
  RigFemPartResultCalculatorSFI.h
  RigFemPartResultCalculatorSFI.cpp
  RigFemPartResultCalculatorDSM.h
  RigFemPartResultCalculatorDSM.cpp
  RigFemPartResultCalculatorFOS.h
  RigFemPartResultCalculatorFOS.cpp
  RigFemPartResultCalculatorED.h
  RigFemPartResultCalculatorED.cpp
  RigFemPartResultCalculatorEV.h
  RigFemPartResultCalculatorEV.cpp
  RigFemPartResultCalculatorQ.h
  RigFemPartResultCalculatorQ.cpp
  RigFemPartResultCalculatorSM.h
  RigFemPartResultCalculatorSM.cpp
  RigFemPartResultCalculatorCompaction.h
  RigFemPartResultCalculatorCompaction.cpp
  RigFemPartResultCalculatorStressGradients.h
  RigFemPartResultCalculatorStressGradients.cpp
  RigFemPartResultCalculatorNE.h
  RigFemPartResultCalculatorNE.cpp
  RigFemPartResultCalculatorFormationIndices.h
  RigFemPartResultCalculatorFormationIndices.cpp
  RigFemPartResultCalculatorBarConverted.h
  RigFemPartResultCalculatorBarConverted.cpp
  RigFemPartResultCalculatorEnIpPorBar.h
  RigFemPartResultCalculatorEnIpPorBar.cpp
  RigFemPartResultCalculatorNodalGradients.h
  RigFemPartResultCalculatorNodalGradients.cpp
  RigFemPartResultCalculatorStressAnisotropy.h
  RigFemPartResultCalculatorStressAnisotropy.cpp
  RigFemPartResultCalculatorPoreCompressibility.h
  RigFemPartResultCalculatorPoreCompressibility.cpp
  RigFemPartResultCalculatorPorosityPermeability.h
  RigFemPartResultCalculatorPorosityPermeability.cpp
  RigFemPartResultCalculatorInitialPorosity.h
  RigFemPartResultCalculatorInitialPorosity.cpp
  RigFemPartResultCalculatorMudWeightWindow.h
  RigFemPartResultCalculatorMudWeightWindow.cpp
  RigFemPartResultCalculatorShearSlipIndicator.h
  RigFemPartResultCalculatorShearSlipIndicator.cpp
  RimGeoMechGeometrySelectionItem.h
  RimGeoMechGeometrySelectionItem.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(LINK_LIBRARIES LibCore cafPdmCvf cafTensor cafUserInterface CommonCode
                   ResultStatisticsCache)

target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})
