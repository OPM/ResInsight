foreach(tapp TableManagerTests
             TabdimsTests
             TableContainerTests
             SimpleTableTests
             ColumnSchemaTests
             TableSchemaTests
             TableColumnTests
             PvtxTableTests)

  opm_add_test(run${tapp} SOURCES ${tapp}.cpp
                          LIBRARIES opmparser ${Boost_LIBRARIES})

endforeach()
