
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimCellFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimCellRangeFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimCellRangeFilterCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilterCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilterCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilterCollection.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimCellFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCellRangeFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCellRangeFilterCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilterCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilterCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilterCollection.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\CellFilters" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
