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
        opm/utility/ECLCaseUtilities.cpp
        opm/utility/ECLEndPointScaling.cpp
        opm/utility/ECLFluxCalc.cpp
        opm/utility/ECLGraph.cpp
        opm/utility/ECLPropertyUnitConversion.cpp
        opm/utility/ECLPropTable.cpp
        opm/utility/ECLPvtCommon.cpp
        opm/utility/ECLPvtCurveCollection.cpp
        opm/utility/ECLPvtGas.cpp
        opm/utility/ECLPvtOil.cpp
        opm/utility/ECLPvtWater.cpp
        opm/utility/ECLRegionMapping.cpp
        opm/utility/ECLResultData.cpp
        opm/utility/ECLSaturationFunc.cpp
        opm/utility/ECLTableInterpolation1D.cpp
        opm/utility/ECLUnitHandling.cpp
        opm/utility/ECLWellSolution.cpp
        )

list (APPEND TEST_SOURCE_FILES
        tests/test_eclendpointscaling.cpp
        tests/test_eclpropertyunitconversion.cpp
        tests/test_eclproptable.cpp
        tests/test_eclpvtcommon.cpp
        tests/test_eclregionmapping.cpp
        tests/test_eclsimple1dinterpolant.cpp
        tests/test_eclunithandling.cpp
        )

list (APPEND EXAMPLE_SOURCE_FILES
        examples/computeFlowStorageCurve.cpp
        examples/computeLocalSolutions.cpp
        examples/computePhaseFluxes.cpp
        examples/computeToFandTracers.cpp
        examples/computeTracers.cpp
        examples/dynamicCellProperty.cpp
        examples/extractFromRestart.cpp
        examples/extractPropCurves.cpp
        tests/runAcceptanceTest.cpp
        tests/runLinearisedCellDataTest.cpp
        tests/runTransTest.cpp
        )

list (APPEND PUBLIC_HEADER_FILES
        opm/utility/ECLCaseUtilities.hpp
        opm/utility/ECLEndPointScaling.hpp
        opm/utility/ECLFluxCalc.hpp
        opm/utility/ECLGraph.hpp
        opm/utility/ECLPhaseIndex.hpp
        opm/utility/ECLPiecewiseLinearInterpolant.hpp
        opm/utility/ECLPropertyUnitConversion.hpp
        opm/utility/ECLPropTable.hpp
        opm/utility/ECLPvtCommon.hpp
        opm/utility/ECLPvtCurveCollection.hpp
        opm/utility/ECLPvtGas.hpp
        opm/utility/ECLPvtOil.hpp
        opm/utility/ECLPvtWater.hpp
        opm/utility/ECLRegionMapping.hpp
        opm/utility/ECLResultData.hpp
        opm/utility/ECLSaturationFunc.hpp
        opm/utility/ECLTableInterpolation1D.hpp
        opm/utility/ECLUnitHandling.hpp
        opm/utility/ECLWellSolution.hpp
        )
