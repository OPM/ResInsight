opm_add_test(runIOConfigTests SOURCES IOConfigTest.cpp
                              LIBRARIES opmparser ${Boost_LIBRARIES})

opm_add_test(runRestartConfigTests SOURCES RestartConfigTests.cpp
                              LIBRARIES opmparser ${Boost_LIBRARIES})
