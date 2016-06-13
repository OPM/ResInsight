opm_add_test(runjsonTests SOURCES jsonTests.cpp
                          LIBRARIES opmjson ${Boost_LIBRARIES})
if (NOT MSVC)
  set_source_files_properties( jsonTests.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
endif()
