foreach(tapp EclipseGridTests MULTREGTScannerTests GridPropertyTests
             FaceDirTests GridPropertiesTests BoxTests PORVTests
             BoxManagerTests TransMultTests FaultTests
             EqualRegTests MultiRegTests ADDREGTests CopyRegTests
             SatfuncPropertyInitializersTests)
  opm_add_test(run${tapp} SOURCES ${tapp}.cpp
                          LIBRARIES opmparser ${Boost_LIBRARIES})
endforeach()
