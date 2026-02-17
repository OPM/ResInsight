project(ResultStatisticsCache)

add_library(
  ${PROJECT_NAME}
  RigStatisticsCalculator.h
  RigStatisticsCalculator.cpp
  RigStatisticsDataCache.h
  RigStatisticsDataCache.cpp
  RigStatisticsMath.h
  RigStatisticsMath.cpp
  RigStatisticsTools.h
  RigStatisticsTools.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} LibCore)
