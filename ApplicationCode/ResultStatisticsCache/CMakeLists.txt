cmake_minimum_required (VERSION 2.8.12)

project (ResultStatisticsCache)

include_directories(
	${LibCore_SOURCE_DIR}
)

add_library( ${PROJECT_NAME}
	RigStatisticsCalculator.h
	RigStatisticsCalculator.cpp
	RigStatisticsDataCache.h
	RigStatisticsDataCache.cpp
	RigStatisticsMath.h
	RigStatisticsMath.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} LibCore)

source_group("" FILES ${PROJECT_FILES})
