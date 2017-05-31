# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

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
        opm/flowdiagnostics/CellSet.cpp
        opm/flowdiagnostics/ConnectionValues.cpp
        opm/flowdiagnostics/ConnectivityGraph.cpp
        opm/flowdiagnostics/DerivedQuantities.cpp
        opm/flowdiagnostics/Solution.cpp
        opm/flowdiagnostics/Toolbox.cpp
        opm/flowdiagnostics/TracerTofSolver.cpp
        opm/utility/graph/tarjan.c
        opm/utility/graph/AssembledConnections.cpp
        opm/utility/numeric/RandomVector.cpp
        )

list (APPEND TEST_SOURCE_FILES
        tests/test_assembledconnections.cpp
        tests/test_cellset.cpp
        tests/test_connectionvalues.cpp
        tests/test_connectivitygraph.cpp
        tests/test_derivedquantities.cpp
        tests/test_flowdiagnosticstool.cpp
        tests/test_tarjan.cpp
        )

list (APPEND PUBLIC_HEADER_FILES
        opm/flowdiagnostics/CellSet.hpp
        opm/flowdiagnostics/CellSetValues.hpp
        opm/flowdiagnostics/ConnectionValues.hpp
        opm/flowdiagnostics/ConnectivityGraph.hpp
        opm/flowdiagnostics/DerivedQuantities.hpp
        opm/flowdiagnostics/Solution.hpp
        opm/flowdiagnostics/Toolbox.hpp
        opm/flowdiagnostics/TracerTofSolver.hpp
        opm/utility/graph/AssembledConnections.hpp
        opm/utility/graph/AssembledConnectionsIteration.hpp
        opm/utility/numeric/RandomVector.hpp
        )
