# Libs to link tests against
set(TEST_LIBS opmcommon Boost::unit_test_framework)
set(EXTRA_TESTS)

if (Boost_UNIT_TEST_FRAMEWORK_FOUND)

# Generated source, needs to be here
opm_add_test(InlineKeywordTest
             EXE_NAME TestKeywords
             SOURCES ${PROJECT_BINARY_DIR}/TestKeywords.cpp
             LIBRARIES ${TEST_LIBS})
list(APPEND EXTRA_TESTS TestKeywords)

# Extra compile definitions and extra parameters
include(cmake/Modules/CheckCaseSensitiveFileSystem.cmake)
set(_testdir ${PROJECT_SOURCE_DIR}/tests/parser/data)

opm_add_test(ParserTests
             SOURCES tests/parser/ParserTests.cpp
             LIBRARIES ${TEST_LIBS}
             TEST_ARGS -- ${_testdir}/)
list(APPEND EXTRA_TESTS ParserTests)

opm_add_test(ParserIncludeTests
             SOURCES tests/parser/ParserIncludeTests.cpp
             LIBRARIES ${TEST_LIBS}
             TEST_ARGS -- ${_testdir}/parser/)
target_compile_definitions(ParserIncludeTests PRIVATE
                           -DHAVE_CASE_SENSITIVE_FILESYSTEM=${HAVE_CASE_SENSITIVE_FILESYSTEM})
list(APPEND EXTRA_TESTS ParserIncludeTests)

opm_add_test(PvtxTableTests
             SOURCES tests/parser/PvtxTableTests.cpp
             LIBRARIES ${TEST_LIBS}
             TEST_ARGS -- ${_testdir}/integration_tests/)
list(APPEND EXTRA_TESTS PvtxTableTests)

opm_add_test(EclipseStateTests
             SOURCES tests/parser/EclipseStateTests.cpp
             LIBRARIES ${TEST_LIBS}
             TEST_ARGS -- ${_testdir}/integration_tests/)
list(APPEND EXTRA_TESTS EclipseStateTests)

foreach (test BoxTest
              CheckDeckValidity
              EclipseGridCreateFromDeck
              IncludeTest
              IntegrationTests
              IOConfigIntegrationTest
              ParseKEYWORD
              Polymer
              ScheduleCreateFromDeck
              TransMultIntegrationTests)

  opm_add_test(${test}
               SOURCES tests/parser/integration/${test}.cpp
               LIBRARIES ${TEST_LIBS}
               TEST_ARGS -- ${_testdir}/integration_tests/)
  list(APPEND EXTRA_TESTS ${test})
endforeach ()

opm_add_test( rst_spe1
              SOURCES tests/rst_test.cpp
              LIBRARIES ${TEST_LIBS}
              TEST_ARGS tests/SPE1CASE2.DATA tests/SPE1CASE2_RESTART.DATA )

opm_add_test( rst_msw
              SOURCES tests/rst_test.cpp
              LIBRARIES ${TEST_LIBS}
              TEST_ARGS tests/MSW.DATA tests/MSW_RESTART.DATA )

add_test( NAME rst_deck_test
          COMMAND ${PROJECT_SOURCE_DIR}/tests/rst_test_driver.sh ${PROJECT_BINARY_DIR}/bin/rst_deck ${PROJECT_BINARY_DIR}/bin/opmhash
          ${PROJECT_SOURCE_DIR}/tests/SPE1CASE2_INCLUDE.DATA)


add_test( NAME rst_deck_test2
          COMMAND ${PROJECT_SOURCE_DIR}/tests/rst_test_driver2.sh
                  ${PROJECT_BINARY_DIR}/bin/rst_deck
                  ${PROJECT_BINARY_DIR}/bin/opmi
                  ${PROJECT_SOURCE_DIR}/tests/ACTIONX_M1.DATA
                  ${PROJECT_SOURCE_DIR}/tests/ACTIONX_M1_MULTIPLE.DATA
                  ${PROJECT_SOURCE_DIR}/tests/ACTIONX_M1.UNRST
                  ${PROJECT_SOURCE_DIR}/tests/ACTIONX_M1.X0010 )

# opm-tests dependent tests
if(HAVE_OPM_TESTS)
  opm_add_test(parse_write ONLY_COMPILE
               SOURCES tests/parser/integration/parse_write.cpp
               LIBRARIES ${TEST_LIBS})
  list(APPEND EXTRA_TESTS parse_write)
  foreach (deck ${OPM_TESTS_ROOT}/norne/NORNE_ATW2013.DATA
                ${OPM_TESTS_ROOT}/spe1_solvent/SPE1CASE2_SOLVENT.DATA
                ${OPM_TESTS_ROOT}/spe9_solvent/SPE9_CP_SOLVENT_CO2.DATA
                ${OPM_TESTS_ROOT}/spe5/SPE5CASE1.DATA
                ${OPM_TESTS_ROOT}/polymer_simple2D/2D_THREEPHASE_POLY_HETER.DATA
                ${OPM_TESTS_ROOT}/spe1/SPE1CASE1.DATA
                ${OPM_TESTS_ROOT}/spe1/SPE1CASE2.DATA
                ${OPM_TESTS_ROOT}/spe1/SPE1CASE2_FAMII.DATA
                ${OPM_TESTS_ROOT}/spe1/SPE1CASE2_SLGOF.DATA
                ${OPM_TESTS_ROOT}/spe3/SPE3CASE1.DATA
                ${OPM_TESTS_ROOT}/spe3/SPE3CASE2.DATA
                ${OPM_TESTS_ROOT}/spe9/SPE9_CP.DATA
                ${OPM_TESTS_ROOT}/spe9/SPE9_CP_GROUP.DATA
                ${OPM_TESTS_ROOT}/spe9/SPE9_CP_SHORT.DATA
                ${OPM_TESTS_ROOT}/spe9/SPE9.DATA
                ${OPM_TESTS_ROOT}/spe10model1/SPE10_MODEL1.DATA
                ${OPM_TESTS_ROOT}/spe10model2/SPE10_MODEL2.DATA
                ${OPM_TESTS_ROOT}/msw_2d_h/2D_H__.DATA
                ${OPM_TESTS_ROOT}/model2/0_BASE_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/1_MULTREGT_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/2_MULTXYZ_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/3_MULTFLT_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/4_MINPVV_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/5_SWATINIT_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/6_ENDSCALE_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/7_HYSTERESIS_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/8_MULTIPLY_TRANXYZ_MODEL2.DATA
                ${OPM_TESTS_ROOT}/model2/9_EDITNNC_MODEL2.DATA)

      get_filename_component(test_name ${deck} NAME_WE)
      opm_add_test(${test_name} NO_COMPILE
                   EXE_NAME parse_write
                   TEST_ARGS ${deck})
  endforeach()
  opm_add_test("SPE9_CP_GROUP2" NO_COMPILE EXE_NAME parse_write TEST_ARGS "${OPM_TESTS_ROOT}/spe9group/SPE9_CP_GROUP.DATA")
  set_property(TEST NORNE_ATW2013
               PROPERTY ENVIRONMENT "OPM_ERRORS_IGNORE=PARSE_RANDOM_SLASH")

  add_test( NAME rst_deck_test_norne
             COMMAND ${PROJECT_SOURCE_DIR}/tests/rst_test_driver.sh ${PROJECT_BINARY_DIR}/bin/rst_deck ${PROJECT_BINARY_DIR}/bin/opmhash
             ${OPM_TESTS_ROOT}/norne/NORNE_ATW2013.DATA)

  set_property(TEST rst_deck_test_norne
               PROPERTY ENVIRONMENT "OPM_ERRORS_IGNORE=PARSE_RANDOM_SLASH")

endif()


# JSON tests
opm_add_test(jsonTests
             SOURCES tests/json/jsonTests.cpp
             LIBRARIES ${TEST_LIBS}
             TEST_ARGS ${PROJECT_SOURCE_DIR}/tests/json/example1.json)
list(APPEND EXTRA_TESTS jsonTests)

endif()
