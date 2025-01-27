set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModel.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationModelCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccess.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessor.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorPorePressure.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorVoidRatio.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorTemperature.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorGeoMech.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStress.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStressGeoMech.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorStressEclipse.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationDataAccessorWellLogExtraction.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultReactivationEnums.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
