foreach(tapp OrderedMapTests ValueTests RecordVectorTests)
  opm_add_test(run${tapp} SOURCES ${tapp}.cpp
                          LIBRARIES opmparser ${Boost_LIBRARIES})
endforeach()
