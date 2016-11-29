foreach(tapp FunctionalTests StringviewTests StringTests )

    opm_add_test(run${tapp}  SOURCES ${tapp}.cpp
                             LIBRARIES opmparser ${Boost_LIBRARIES})

endforeach()
