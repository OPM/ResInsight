cmake_minimum_required (VERSION 2.8)

if (MSVC)
  ADD_DEFINITIONS("-DBOOST_ALL_NO_LIB")
endif()

project ( well_UnitTests )


include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../..

    ${Boost_INCLUDE_DIRS}

    ${NR_well_SOURCE_DIR}
)


set( PROJECT_FILES

	well_UnitTests.cpp
	../../gtest/gtest-all.cc

	wellBasicTest.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

if (MSVC)
  set (RI_BOOST_LIBRARIES
    debug     ${Boost_SYSTEM_LIBRARY_DEBUG}
    optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
    debug     ${Boost_FILESYSTEM_LIBRARY_DEBUG}
    optimized ${Boost_FILESYSTEM_LIBRARY_RELEASE}
  )
else()
  set (RI_BOOST_LIBRARIES ${Boost_LIBRARIES})
endif()

target_link_libraries ( ${PROJECT_NAME}
    NRLib
    ${RI_BOOST_LIBRARIES}
)

