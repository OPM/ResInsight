opm_add_test(runjsonTests SOURCES jsonTests.cpp
                          LIBRARIES opmjson ${Boost_LIBRARIES})
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set_source_files_properties( jsonTests.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
endif()
