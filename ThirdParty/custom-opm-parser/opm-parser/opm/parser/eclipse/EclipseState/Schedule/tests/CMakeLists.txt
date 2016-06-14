foreach(tapp TimeMapTest ScheduleTests WellTests
             WellPropertiesTests WellSetTests
             GroupTests ScheduleEnumTests
             CompletionTests CompletionSetTests
             DynamicStateTests GroupTreeNodeTests
             GroupTreeTests TuningTests EventTests
             WellSolventTests DynamicVectorTests
             GeomodifierTests)
  opm_add_test(run${tapp} SOURCES ${tapp}.cpp
                          LIBRARIES opmparser ${Boost_LIBRARIES})
endforeach()
