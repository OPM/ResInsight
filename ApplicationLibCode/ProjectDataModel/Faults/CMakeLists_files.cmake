set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModelCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorPorePressure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorVoidRatio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorTemperature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorGeoMech.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStress.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStressGeoMech.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStressEclipse.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorWellLogExtraction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationEnums.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
