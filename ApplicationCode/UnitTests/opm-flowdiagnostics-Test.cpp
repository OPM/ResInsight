#include "gtest/gtest.h"

const std::string casePath = "\\\\csfiles\\Store\\ProjectData\\StatoilReservoir\\ReferenceCases\\simple_FlowDiag_Model\\";

#include "exampleSetup.hpp"

TEST(opm_flowdiagnostics_test, basic_construction)
{

    try
    {
        
        Opm::ECLGraph graph = Opm::ECLGraph::load(casePath + "SIMPLE.EGRID",
                                                  casePath + "SIMPLE.INIT");
        graph.assignFluxDataSource(casePath + "SIMPLE.UNRST");
        int step = 2;
        if ( ! graph.selectReportStep(step) )
        {
            std::ostringstream os;

            os << "Report Step " << step
                << " is Not Available in Result Set '"
                << casePath << '\'';

            throw std::domain_error(os.str());
        }

        Opm::FlowDiagnostics::Toolbox fdTool = example::initialiseFlowDiagnostics(graph);

        // Solve for time of flight.
        std::vector<Opm::FlowDiagnostics::CellSet> start;
        Opm::FlowDiagnostics::Toolbox::Forward sol = fdTool.computeInjectionDiagnostics(start);
        const std::vector<double>& tof = sol.fd.timeOfFlight();

        // Write it to standard out.
        std::cout.precision(16);
        for ( double t : tof )
        {
            // std::cout << t << '\n';
        }
    }
    catch ( const std::exception& e )
    {
        std::cerr << "Caught exception: " << e.what() << '\n';
    }
}
