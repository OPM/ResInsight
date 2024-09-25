if(CMAKE_VERSION VERSION_LESS 3.8)
  message(WARNING "CMake version does not support c++17, guessing -std=c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

 if(MSVC)
  add_definitions( "/wd4996 /wd4244 /wd4267 /wd4101 /wd4477" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /permissive-")
endif(MSVC)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-switch -Wno-sign-compare -Wno-deprecated-copy -Wno-missing-field-initializers")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-missing-field-initializers -Wno-deprecated-copy")
endif()

project (custom-opm-common)

find_package(Boost)

include_directories(
    opm-common
    opm-common/external/cjson
    generated-opm-common 
    generated-opm-common/include
)

 add_definitions(-DFMT_HEADER_ONLY)
    include_directories(SYSTEM ../fmtlib/include)

# Set defines to include requred files
set(ENABLE_ECL_INPUT true)
#set(ENABLE_ECL_OUTPUT true)


include(opm-common/CMakeLists_files.cmake)

set(opm_parser_source_files_short_path 
	${MAIN_SOURCE_FILES}
)

foreach (file ${opm_parser_source_files_short_path} )
    list(APPEND opm_parser_source_files_long_path "opm-common/${file}" )
endforeach()

set(opm_parser_source_files
	${opm_parser_source_files_long_path}
    ${opm_parser_generated_source_files} 
) 

# Append generated sources
list(INSERT opm-common_SOURCES 0 generated-opm-common/ParserInit.cpp)
foreach (name A B C D E F G H I J K L M N O P Q R S T U V W X Y Z)
    list(INSERT opm-common_SOURCES 0 generated-opm-common/ParserKeywords/${name}.cpp)
endforeach()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    list(APPEND ADDITIONAL_LINK_LIBRARIES stdc++fs)
  endif()
endif()

 if(MSVC)
    list(APPEND opm-common_SOURCES opm-common/cross-platform/windows/Substitutes.cpp )
endif(MSVC)

add_library(${PROJECT_NAME} 
    STATIC
	${opm_parser_source_files_long_path}
	${opm-common_SOURCES}

    ## Missing files when only ENABLE_ECL_INPUT is defined
    opm-common/src/opm/io/eclipse/EclFile.cpp
    opm-common/src/opm/io/eclipse/EclOutput.cpp
    opm-common/src/opm/io/eclipse/EclUtil.cpp
    opm-common/src/opm/io/eclipse/EGrid.cpp
    opm-common/src/opm/io/eclipse/ESmry.cpp
    opm-common/src/opm/io/eclipse/ExtESmry.cpp
    opm-common/src/opm/io/eclipse/EInit.cpp
    opm-common/src/opm/io/eclipse/ERft.cpp

    # Required for use of static function RstConnection::inverse_peaceman
    opm-common/src/opm/io/eclipse/rst/connection.cpp

    # Required for use of RstHeader::restart_info
    opm-common/src/opm/io/eclipse/rst/action.cpp
    opm-common/src/opm/io/eclipse/rst/aquifer.cpp
    opm-common/src/opm/io/eclipse/rst/connection.cpp
    opm-common/src/opm/io/eclipse/rst/group.cpp
    opm-common/src/opm/io/eclipse/rst/header.cpp
    opm-common/src/opm/io/eclipse/rst/network.cpp
    opm-common/src/opm/io/eclipse/rst/segment.cpp
    opm-common/src/opm/io/eclipse/rst/state.cpp
    opm-common/src/opm/io/eclipse/rst/udq.cpp
    opm-common/src/opm/io/eclipse/rst/well.cpp
    opm-common/src/opm/output/eclipse/InteHEAD.cpp 
    opm-common/src/opm/output/eclipse/UDQDims.cpp
    opm-common/src/opm/output/eclipse/CreateActionRSTDims.cpp 

	# 2022.06 additional includes
    opm-common/src/opm/io/eclipse/ERst.cpp
    opm-common/src/opm/io/eclipse/RestartFileView.cpp
    opm-common/cross-platform/windows/Substitutes.cpp 
	
)

if(RESINSIGHT_ENABLE_UNITY_BUILD)

    set(UNITY_EXCLUDE_FILES
    opm-common/src/opm/parser/eclipse/Deck/UDAValue.cpp
    opm-common/src/opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.cpp
    opm-common/src/opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunction.cpp
    opm-common/src/opm/parser/eclipse/EclipseState/Schedule/Well/WListManager.cpp
    opm-common/src/opm/parser/eclipse/EclipseState/Schedule/MSW/SpiralICD.cpp
    opm-common/src/opm/io/eclipse/EclOutput.cpp
    opm-common/src/opm/io/eclipse/ESmry.cpp
    opm-common/src/opm/io/eclipse/EGrid.cpp
    )

    foreach(fileToExclude ${UNITY_EXCLUDE_FILES})
      set_source_files_properties(${fileToExclude}
                                PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
    endforeach(fileToExclude)
endif()

find_path(BOOST_SPIRIT_INCLUDE_DIRS "boost/spirit.hpp" HINTS ${Boost_INCLUDE_DIRS})

target_link_libraries(custom-opm-common 
  ${ADDITIONAL_LINK_LIBRARIES}
)

target_include_directories(custom-opm-common 
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/opm-common
    ${CMAKE_CURRENT_SOURCE_DIR}/generated-opm-common/include
  PRIVATE
    ${BOOST_SPIRIT_INCLUDE_DIRS}
    
)