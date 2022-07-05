/*
  Copyright 2018 Statoil IT

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#define BOOST_TEST_MODULE DoubHEAD_Vector

#include <boost/test/unit_test.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/output/eclipse/DoubHEAD.hpp>
#include <opm/output/eclipse/VectorItems/doubhead.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <chrono>
#include <ctime>
#include <initializer_list>
#include <numeric>              // partial_sum()
#include <ratio>
#include <vector>

namespace {

    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }
}

//int main(int argc, char* argv[])
struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python{ std::make_shared<Opm::Python>() }
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;

};


namespace {
    using Day = std::chrono::duration<double,
        std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>
    >;

    std::chrono::time_point<std::chrono::system_clock> startSimulation()
    {
        // 2015-04-09T00:00:00+0000
        auto timePoint = std::tm{};

        timePoint.tm_year = 115;     // 2015
        timePoint.tm_mon  =   4 - 1; // April
        timePoint.tm_mday =   9;     // 9th

        return Opm::TimeService::from_time_t( Opm::TimeService::makeUTCTime(timePoint) );
    }

    std::chrono::duration<double, std::chrono::seconds::period> tstep_123()
    {
        return Day{ 123 };
    }

    Opm::RestartIO::DoubHEAD::TimeStamp
    makeTStamp(std::chrono::time_point<std::chrono::system_clock>          start,
               std::chrono::duration<double, std::chrono::seconds::period> elapsed)
    {
        return { start, elapsed };
    }

    double getTimeConv(const ::Opm::UnitSystem& us)
    {
        switch (us.getType()) {
        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC:
            return static_cast<double>(Opm::Metric::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_FIELD:
            return static_cast<double>(Opm::Field::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_LAB:
            return static_cast<double>(Opm::Lab::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_PVT_M:
            return static_cast<double>(Opm::PVT_M::Time);

        case ::Opm::UnitSystem::UnitType::UNIT_TYPE_INPUT:
            throw std::invalid_argument {
                "Cannot Run Simulation With Non-Standard Units"
            };
        }

        throw std::invalid_argument("Unknown unit type specified");
    }
} // Anonymous

BOOST_AUTO_TEST_SUITE(Member_Functions)

BOOST_AUTO_TEST_CASE(Time_Stamp)
{
    const auto dh = Opm::RestartIO::DoubHEAD{}
        .timeStamp(makeTStamp(startSimulation(), tstep_123()));

    const auto& v = dh.data();

    // Start + elapsed = current (in days)
    BOOST_CHECK_CLOSE(v[1 - 1] + v[161 - 1], v[162 - 1], 1.0e-10);

    // Elapsed time in days.
    BOOST_CHECK_CLOSE(v[1 - 1], 123.0, 1.0e-10);

    // DateNum(startSimulation()) ==
    //     floor(365.25 * 2015) + day_of_year(=99)
    BOOST_CHECK_CLOSE(v[161 - 1], 736077.0, 1.0e-10);

    // Start + elapsed (days)
    BOOST_CHECK_CLOSE(v[162 - 1], 736200.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(Wsegiter)
{
    const auto simCase = SimulationCase{first_sim("0A4_GRCTRL_LRAT_LRAT_GGR_BASE_MODEL2_MSW_ALL.DATA")};

    Opm::EclipseState es    = simCase.es;
    Opm::Schedule     sched = simCase.sched;

    const auto& usys  = es.getDeckUnitSystem();
    const auto  tconv = getTimeConv(usys);

    const std::size_t lookup_step = 1;

    const auto dh = Opm::RestartIO::DoubHEAD{}
        .tuningParameters(sched[lookup_step].tuning(), tconv);

    const auto& v = dh.data();

    namespace VI = Opm::RestartIO::Helpers::VectorItems;

    BOOST_CHECK_EQUAL(v[VI::WsegRedFac], 0.3);
    BOOST_CHECK_EQUAL(v[VI::WsegIncFac], 2.0);

}

BOOST_AUTO_TEST_CASE(Netbalan)
{
    const auto simCase = SimulationCase{first_sim("5_NETWORK_MODEL5_STDW_NETBAL_PACK.DATA")};

    Opm::EclipseState es    = simCase.es;
    Opm::EclipseGrid  grid   = simCase.grid;

    Opm::Schedule     sched = simCase.sched;
    const auto& start_time = sched.getStartTime();

    double simTime = start_time + 2.E09;
    const double next_step_size = 0.2;

    const std::size_t report_step = 1;
    const std::size_t lookup_step = report_step - 1;

    const auto ih = Opm::RestartIO::Helpers::
            createInteHead(es, grid, sched, simTime,
                           report_step, // Should really be number of timesteps
                           report_step, lookup_step);
    const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, lookup_step, report_step,
                                                simTime, next_step_size);

    const auto& v = dh.data();

    namespace VI = Opm::RestartIO::Helpers::VectorItems;

    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbalint], 2.345);
    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbalnpre], 0.033);
    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbalthpc], 0.1);
    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbaltarerr], 1.E+19);
    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbalmaxerr], 1.E+18);
    BOOST_CHECK_EQUAL(v[VI::doubhead::Netbalstepsz], 0.15);

}


BOOST_AUTO_TEST_SUITE_END()
