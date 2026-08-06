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

# cafAssert.h is header-only, so the include directory is enough - no need to
# link cafPdmCore
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/Fwk/AppFwk/cafProjectDataModel/cafPdmCore
)

target_link_libraries(${PROJECT_NAME} LibCore)
