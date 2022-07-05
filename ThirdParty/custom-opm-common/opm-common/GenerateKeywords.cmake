set(genkw_SOURCES src/opm/json/JsonObject.cpp
                  src/opm/input/eclipse/Parser/createDefaultKeywordList.cpp
                  src/opm/input/eclipse/Deck/UDAValue.cpp
                  src/opm/input/eclipse/Deck/DeckTree.cpp
                  src/opm/input/eclipse/Deck/DeckValue.cpp
                  src/opm/input/eclipse/Deck/Deck.cpp
                  src/opm/input/eclipse/Deck/DeckView.cpp
                  src/opm/input/eclipse/Deck/DeckItem.cpp
                  src/opm/input/eclipse/Deck/DeckKeyword.cpp
                  src/opm/input/eclipse/Deck/DeckRecord.cpp
                  src/opm/input/eclipse/Deck/DeckOutput.cpp
                  src/opm/input/eclipse/Generator/KeywordGenerator.cpp
                  src/opm/input/eclipse/Generator/KeywordLoader.cpp
                  src/opm/input/eclipse/Schedule/UDQ/UDQEnums.cpp
                  src/opm/input/eclipse/Parser/ErrorGuard.cpp
                  src/opm/input/eclipse/Parser/ParseContext.cpp
                  src/opm/input/eclipse/Parser/ParserEnums.cpp
                  src/opm/input/eclipse/Parser/ParserItem.cpp
                  src/opm/input/eclipse/Parser/ParserKeyword.cpp
                  src/opm/input/eclipse/Parser/ParserRecord.cpp
                  src/opm/input/eclipse/Parser/raw/RawKeyword.cpp
                  src/opm/input/eclipse/Parser/raw/RawRecord.cpp
                  src/opm/input/eclipse/Parser/raw/StarToken.cpp
                  src/opm/input/eclipse/Units/Dimension.cpp
                  src/opm/input/eclipse/Units/UnitSystem.cpp
                  src/opm/common/utility/OpmInputError.cpp
                  src/opm/common/utility/shmatch.cpp
                  src/opm/common/OpmLog/OpmLog.cpp
                  src/opm/common/OpmLog/Logger.cpp
                  src/opm/common/OpmLog/StreamLog.cpp
                  src/opm/common/OpmLog/LogBackend.cpp
                  src/opm/common/OpmLog/LogUtil.cpp
)
if(NOT cjson_FOUND)
  list(APPEND genkw_SOURCES external/cjson/cJSON.c)
endif()
add_executable(genkw ${genkw_SOURCES})

target_link_libraries(genkw ${opm-common_LIBRARIES})

# Generate keyword list
include(src/opm/input/eclipse/share/keywords/keyword_list.cmake)
string(REGEX REPLACE "([^;]+)" "${PROJECT_SOURCE_DIR}/src/opm/input/eclipse/share/keywords/\\1" keyword_files "${keywords}")
configure_file(src/opm/input/eclipse/keyword_list.argv.in keyword_list.argv)

# Generate keyword source

set( genkw_argv keyword_list.argv
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserInit.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/include/
  opm/input/eclipse/Parser/ParserKeywords
  ${PROJECT_BINARY_DIR}/tmp_gen/TestKeywords.cpp)

set( _tmp_output
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/A.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/B.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/C.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/D.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/E.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/F.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/G.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/H.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/I.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/J.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/K.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/L.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/M.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/N.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/O.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/P.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/Q.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/R.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/S.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/T.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/U.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/V.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/W.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/X.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/Y.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/Z.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/ParserInit.cpp
  ${PROJECT_BINARY_DIR}/tmp_gen/TestKeywords.cpp)


set( _target_output
  ${PROJECT_BINARY_DIR}/ParserKeywords/A.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/B.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/C.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/D.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/E.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/F.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/G.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/H.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/I.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/J.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/K.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/L.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/M.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/N.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/O.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/P.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/Q.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/R.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/S.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/T.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/U.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/V.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/W.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/X.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/Y.cpp
  ${PROJECT_BINARY_DIR}/ParserKeywords/Z.cpp
  ${PROJECT_BINARY_DIR}/TestKeywords.cpp
  ${PROJECT_BINARY_DIR}/ParserInit.cpp)


if (OPM_ENABLE_PYTHON)
  list(APPEND genkw_argv ${PROJECT_BINARY_DIR}/tmp_gen/builtin_pybind11.cpp)
  list(APPEND _tmp_output ${PROJECT_BINARY_DIR}/tmp_gen/builtin_pybind11.cpp)
  list(APPEND _target_output ${PROJECT_BINARY_DIR}/python/cxx/builtin_pybind11.cpp)
endif()



add_custom_command( OUTPUT
  ${_tmp_output}
  COMMAND genkw ${genkw_argv}
  DEPENDS genkw ${keyword_files} src/opm/input/eclipse/share/keywords/keyword_list.cmake)

# To avoid some rebuilds
add_custom_command(OUTPUT
  ${_target_output}
  DEPENDS ${PROJECT_BINARY_DIR}/tmp_gen/ParserKeywords/A.cpp
  COMMAND ${CMAKE_COMMAND} -DBASE_DIR=${PROJECT_BINARY_DIR} -P ${PROJECT_SOURCE_DIR}/CopyHeaders.cmake)
