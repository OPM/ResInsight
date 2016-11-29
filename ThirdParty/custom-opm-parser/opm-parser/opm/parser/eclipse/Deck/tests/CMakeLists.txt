foreach(tapp DeckRecordTests DeckIntItemTests DeckDoubleItemTests
             DeckStringItemTests DeckTests DeckKeywordTests SectionTests)
  opm_add_test(run${tapp} SOURCES ${tapp}.cpp
                          LIBRARIES opmparser ${Boost_LIBRARIES})
endforeach()
