
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimSurface.h
${CMAKE_CURRENT_LIST_DIR}/RimFileSurface.h
${CMAKE_CURRENT_LIST_DIR}/RimGridCaseSurface.h
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInView.h
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInViewCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceResultDefinition.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurface.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurfaceInView.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsSurface.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimSurface.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFileSurface.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGridCaseSurface.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInView.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceInViewCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSurfaceResultDefinition.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurface.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSurfaceInView.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsSurface.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Surface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
