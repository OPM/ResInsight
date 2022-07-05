#define BOOST_TEST_MODULE UDQ_Data

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/TracerConfig.hpp>

#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/DoubHEAD.hpp>
#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/doubhead.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

Opm::Deck
first_sim(std::string fname)
{
    return Opm::Parser {}.parseFile(fname);
}
} // namespace



Opm::SummaryState
sum_state()
{
    auto state = Opm::SummaryState {Opm::TimeService::now()};
    state.update("FULPR", 460.);

    return state;
}


// int main(int argc, char* argv[])
struct SimulationCase {
    explicit SimulationCase(const Opm::Deck& deck)
        : es {deck}
        , grid {deck}
        , python {std::make_shared<Opm::Python>()}
        , sched {deck, es, python}
    {
    }

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule sched;
};

BOOST_AUTO_TEST_SUITE(LiftGasOptimization)



// test lift gas optimisation data
BOOST_AUTO_TEST_CASE(liftGasOptimzation_data)
{
    const auto simCase = SimulationCase {first_sim("2_WLIFT_MODEL5_NOINC.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::SummaryState st = sum_state();
    Opm::Schedule sched = simCase.sched;
    Opm::EclipseGrid grid = simCase.grid;
    // const auto& ioConfig = es.getIOConfig();
    Opm::Action::State action_state;

    // Report Step 1: 2020-01-01
    const auto rptStep = std::size_t {2};
    const auto simStep = std::size_t {1};

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, rptStep, rptStep, simStep);

    // set dummy value for next_step_size
    const double next_step_size = 0.1;
    const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, simStep, simStep+1, secs_elapsed, next_step_size);

    auto wellData = Opm::RestartIO::Helpers::AggregateWellData(ih);
    wellData.captureDeclaredWellData(sched, Opm::TracerConfig{}, simStep, action_state, {}, st, ih);

    // intehead data
    auto eachnc = Opm::RestartIO::Helpers::VectorItems::intehead::EACHNCITS;
    auto niwelz = Opm::RestartIO::Helpers::VectorItems::intehead::NIWELZ;
    auto nswelz = Opm::RestartIO::Helpers::VectorItems::intehead::NSWELZ;

    // doubhead data
    auto lomini = Opm::RestartIO::Helpers::VectorItems::doubhead::LOminInt;
    auto loincr = Opm::RestartIO::Helpers::VectorItems::doubhead::LOincrsz;
    auto lomineg = Opm::RestartIO::Helpers::VectorItems::doubhead::LOminEcGrad;

    // iwel data
    auto liftopt = static_cast<std::size_t>(Opm::RestartIO::Helpers::VectorItems::IWell::index::LiftOpt);

    // swel data
    auto lomaxrate = static_cast<std::size_t>(Opm::RestartIO::Helpers::VectorItems::SWell::index::LOmaxRate);
    auto lominrate = static_cast<std::size_t>(Opm::RestartIO::Helpers::VectorItems::SWell::index::LOminRate);
    auto loweightfac = static_cast<std::size_t>(Opm::RestartIO::Helpers::VectorItems::SWell::index::LOweightFac);



    {
        /*
        Check of InteHEAD and DoubHEAD data for LIFTOPT variables
        */

        BOOST_CHECK_EQUAL(ih[eachnc], 2);
        BOOST_CHECK_EQUAL(dh[lomini], 37.);
        BOOST_CHECK_EQUAL(dh[loincr], 12500);
        BOOST_CHECK_EQUAL(dh[lomineg], 5E-3);
    }


    {
        // IWEL

        const auto& iWel = wellData.getIWell();

        auto start = 0 * static_cast<std::size_t>(ih[niwelz]);
        BOOST_CHECK_EQUAL(iWel[start + liftopt], 1);

        start = 1 * static_cast<std::size_t>(ih[niwelz]);
        BOOST_CHECK_EQUAL(iWel[start + liftopt], 0);
    }

    {
        // SWEL

        const auto& sWel = wellData.getSWell();

        auto start = 0 * static_cast<std::size_t>(ih[nswelz]);
        BOOST_CHECK_CLOSE(sWel[start + lomaxrate], 150000.f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + lominrate], -1.0f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + loweightfac], 1.01f, 1.0e-7f);

        start = 1 * static_cast<std::size_t>(ih[nswelz]);
        BOOST_CHECK_CLOSE(sWel[start + lomaxrate], 150000.f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + lominrate], 0.0f, 1.0e-7f); // default value since item 2 for this well is 'NO'
        BOOST_CHECK_CLOSE(sWel[start + loweightfac], 1.0f, 1.0e-7f); // default value since item 2 for this well is 'NO'

        start = 2 * static_cast<std::size_t>(ih[nswelz]);
        BOOST_CHECK_CLOSE(sWel[start + lomaxrate], 150000.f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + lominrate], 3.0f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + loweightfac], 1.21f, 1.0e-7f);

        start = 3 * static_cast<std::size_t>(ih[nswelz]);
        BOOST_CHECK_CLOSE(sWel[start + lomaxrate], 150000.f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + lominrate], 0.f, 1.0e-7f);
        BOOST_CHECK_CLOSE(sWel[start + loweightfac], 1.01f, 1.0e-7f);
    }
}

BOOST_AUTO_TEST_SUITE_END()
