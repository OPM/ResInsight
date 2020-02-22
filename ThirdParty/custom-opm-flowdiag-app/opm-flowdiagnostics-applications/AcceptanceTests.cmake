# Set absolute tolerance to be used for testing
Set (abs_tol 5.0e-8)
Set (rel_tol 1.0e-13)

# Input:
#   - casename: with or without extension
#
Macro (add_acceptance_test casename)

  String (REGEX REPLACE "\\.[^.]*$" "" basename "${casename}")

  # Note: non-default tolerances used for TOF tests (defaults too strict)

  Add_Test (NAME    ToF_accept_${casename}_all_steps
            COMMAND runAcceptanceTest
            "case=${OPM_DATA_ROOT}/flow_diagnostic_test/eclipse-simulation/${basename}"
            "ref-dir=${OPM_DATA_ROOT}/flow_diagnostic_test/fd-ref-data/${basename}"
            "atol=5e-6" "rtol=1e-13")

EndMacro (add_acceptance_test)

# Input
#  - casename: with or without extension
#
Macro (add_trans_acceptance_test casename)

  String (REGEX REPLACE "\\.[^.]*$" "" basename "${casename}")

  Add_Test (NAME    Trans_accept_${casename}
            COMMAND runTransTest
            "case=${OPM_DATA_ROOT}/flow_diagnostic_test/eclipse-simulation/${basename}"
            "ref-dir=${OPM_DATA_ROOT}/flow_diagnostic_test/fd-ref-data/${basename}"
            "atol=${abs_tol}" "rtol=${rel_tol}")

EndMacro (add_trans_acceptance_test)

# Input
#   - casename: with or without extension
#   - strings identifying which physical quantities to compare
Macro (add_celldata_acceptance_test casename)

  String (REGEX REPLACE "\\.[^.]*$" "" basename "${casename}")

  Add_Test (NAME    CellData_accept_${casename}
            COMMAND runLinearisedCellDataTest
            "case=${OPM_DATA_ROOT}/flow_diagnostic_test/eclipse-simulation/${basename}"
            "ref-dir=${OPM_DATA_ROOT}/flow_diagnostic_test/fd-ref-data/${basename}"
            "quant=${ARGN}" "atol=${abs_tol}" "rtol=${rel_tol}")

EndMacro (add_celldata_acceptance_test)

If (NOT TARGET test-suite)
  Add_Custom_Target (test-suite)
EndIf ()

# Acceptance tests

Add_Acceptance_Test (SIMPLE_2PH_W_FAULT_LGR)
Add_Trans_Acceptance_Test (SIMPLE_2PH_W_FAULT_LGR)
Add_CellData_Acceptance_Test (SIMPLE_2PH_W_FAULT_LGR "pressure")
