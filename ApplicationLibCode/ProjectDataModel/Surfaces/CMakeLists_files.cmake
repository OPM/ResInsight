set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFileSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCaseSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceResultDefinition.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimDepthSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFractureSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimRegularSurface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimRegularFileSurface.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFileSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCaseSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSurfaceResultDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDepthSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFractureSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRegularSurface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRegularFileSurface.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
