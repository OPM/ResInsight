#include "gtest/gtest.h"

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/utility/graph/AssembledConnections.hpp>
#include <opm/utility/ECLGraph.hpp>

TEST(opm_flowdiagnostics_test, basic_construction)
{
    auto g = Opm::AssembledConnections{};
    auto s = Opm::FlowDiagnostics::CellSet{};
    try {
    auto eg = Opm::ECLGraph::load("hei", "hopp");
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << '\n';
    }
}
