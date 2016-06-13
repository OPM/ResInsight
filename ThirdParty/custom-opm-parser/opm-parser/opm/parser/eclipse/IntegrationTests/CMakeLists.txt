add_definitions( -DKEYWORD_DIRECTORY="${PROJECT_SOURCE_DIR}/opm/parser/share/keywords/000_Eclipse100")

foreach(tapp TransMultIntegrationTests CheckDeckValidity IntegrationTests ParseWellProbe
             ParseTITLE ParseTOPS ParseWCONHIST ParseWellWithWildcards
             ParsePORO ParsePLYVISC ParsePLYDHFLF ParsePLYSHLOG ParseDATAWithDefault
             ParseTVDP ParseDENSITY ParseVFPPROD ScheduleCreateFromDeck
             CompletionsFromDeck ParseEND IncludeTest ParseEQUIL
             ParseRSVD ParsePVTG ParsePVTO ParseSWOF BoxTest
             ParseMULTREGT ParseSGOF ParseSLGOF EclipseGridCreateFromDeck NNCTests
             ResinsightTest IOConfigIntegrationTest
             Polymer ParsePLYADSS ParseMULTSEGWELL ParseMiscible)
  opm_add_test(run${tapp} SOURCES ${tapp}.cpp LIBRARIES opmparser ${Boost_LIBRARIES})
endforeach()


if (HAVE_OPM_DATA)
   foreach( test_deck ${OPM_DATA_ROOT}/norne/NORNE_ATW2013.DATA
                      ${OPM_DATA_ROOT}/solvent_test_suite/SPE1CASE2_SOLVENT.DATA
                      ${OPM_DATA_ROOT}/solvent_test_suite/SPE9_CP_SOLVENT_CO2.DATA
                      ${OPM_DATA_ROOT}/solvent_test_suite/SPE5CASE1.DATA
                      ${OPM_DATA_ROOT}/polymer_test_suite/simple2D/2D_THREEPHASE_POLY_HETER.DATA
                      ${OPM_DATA_ROOT}/spe1/SPE1CASE1.DATA
                      ${OPM_DATA_ROOT}/spe1/SPE1CASE2.DATA
                      ${OPM_DATA_ROOT}/spe1/SPE1CASE2_FAMII.DATA
                      ${OPM_DATA_ROOT}/spe1/SPE1CASE2_SLGOF.DATA
                      ${OPM_DATA_ROOT}/spe3/SPE3CASE1.DATA
                      ${OPM_DATA_ROOT}/spe3/SPE3CASE2.DATA
                      ${OPM_DATA_ROOT}/spe9/SPE9_CP.DATA
                      ${OPM_DATA_ROOT}/spe9/SPE9.DATA
                      ${OPM_DATA_ROOT}/spe10model1/SPE10_MODEL1.DATA
                      ${OPM_DATA_ROOT}/spe10model2/SPE10_MODEL2.DATA )
      get_filename_component( test_name ${test_deck} NAME_WE )
      add_test( NAME ${test_name} COMMAND opmi ${test_deck})
   endforeach()

   set_property(TEST NORNE_ATW2013 PROPERTY ENVIRONMENT "OPM_ERRORS_IGNORE=PARSE_RANDOM_SLASH")
endif()

