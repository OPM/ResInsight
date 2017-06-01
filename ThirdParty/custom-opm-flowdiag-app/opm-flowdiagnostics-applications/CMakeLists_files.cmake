# This file sets up five lists:
#       MAIN_SOURCE_FILES     List of compilation units which will be included in
#                             the library. If it isn't on this list, it won't be
#                             part of the library. Please try to keep it sorted to
#                             maintain sanity.
#
#       TEST_SOURCE_FILES     List of programs that will be run as unit tests.
#
#       TEST_DATA_FILES       Files from the source three that should be made
#                             available in the corresponding location in the build
#                             tree in order to run tests there.
#
#       EXAMPLE_SOURCE_FILES  Other programs that will be compiled as part of the
#                             build, but which is not part of the library nor is
#                             run as tests.
#
#       PUBLIC_HEADER_FILES   List of public header files that should be
#                             distributed together with the library. The source
#                             files can of course include other files than these;
#                             you should only add to this list if the *user* of
#                             the library needs it.

list (APPEND MAIN_SOURCE_FILES
        opm/utility/ECLEndPointScaling.cpp
        opm/utility/ECLFluxCalc.cpp
        opm/utility/ECLGraph.cpp
        opm/utility/ECLPropTable.cpp
        opm/utility/ECLResultData.cpp
        opm/utility/ECLSaturationFunc.cpp
        opm/utility/ECLUnitHandling.cpp
        opm/utility/ECLWellSolution.cpp
        )

list (APPEND TEST_SOURCE_FILES
        tests/test_eclendpointscaling.cpp
        tests/test_eclproptable.cpp
        tests/test_eclunithandling.cpp
        )

list (APPEND EXAMPLE_SOURCE_FILES
        examples/computeFlowStorageCurve.cpp
        examples/computeLocalSolutions.cpp
        examples/computeToFandTracers.cpp
        examples/computeTracers.cpp
        examples/extractFromRestart.cpp
        tests/runAcceptanceTest.cpp
        tests/runLinearisedCellDataTest.cpp
        tests/runTransTest.cpp
        )

list (APPEND PUBLIC_HEADER_FILES
        opm/utility/ECLEndPointScaling.hpp
        opm/utility/ECLFluxCalc.hpp
        opm/utility/ECLGraph.hpp
        opm/utility/ECLPhaseIndex.hpp
        opm/utility/ECLPropTable.hpp
        opm/utility/ECLResultData.hpp
        opm/utility/ECLSaturationFunc.hpp
        opm/utility/ECLUnitHandling.hpp
        opm/utility/ECLWellSolution.hpp
        )
