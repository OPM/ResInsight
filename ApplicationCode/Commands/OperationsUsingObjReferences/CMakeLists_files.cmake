
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCopyReferencesToClipboardFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCutReferencesToClipboardFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteFeatureImpl.h

${CMAKE_CURRENT_LIST_DIR}/RicPasteEclipseCasesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteEclipseViewsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteGeoMechViewsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteIntersectionsFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCopyReferencesToClipboardFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCutReferencesToClipboardFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteFeatureImpl.cpp

${CMAKE_CURRENT_LIST_DIR}/RicPasteEclipseCasesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteEclipseViewsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteGeoMechViewsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteIntersectionsFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ObjReferences" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
