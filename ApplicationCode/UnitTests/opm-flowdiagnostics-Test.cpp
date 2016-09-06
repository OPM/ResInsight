#include "gtest/gtest.h"

#include <opm/flowdiagnostics/CellSet.hpp>
#include <opm/utility/graph/AssembledConnections.hpp>

TEST(opm_flowdiagnostics_test, basic_construction)
{
    auto g = Opm::AssembledConnections{};
    auto s = Opm::FlowDiagnostics::CellSet{};
}
