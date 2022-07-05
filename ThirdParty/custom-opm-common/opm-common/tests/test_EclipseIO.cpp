/*
  Copyright 2014 Andreas Lauser

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
#include "config.h"

#define BOOST_TEST_MODULE EclipseIO
#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/output/data/Cells.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include <opm/common/utility/TimeService.hpp>

#include <algorithm>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <time.h>

#include <tests/WorkArea.hpp>

using namespace Opm;

namespace {

bool keywordExists(const std::vector<EclIO::EclFile::EclEntry>& knownVec,
                   const std::string&                           arrayname)
{
    return std::any_of(knownVec.begin(), knownVec.end(),
        [&arrayname](const EclIO::EclFile::EclEntry& entry) -> bool
    {
        return std::get<0>(entry) == arrayname;
    });
}

template <typename T>
T sum(const std::vector<T>& array)
{
    return std::accumulate(array.begin(), array.end(), T(0));
}

data::Solution createBlackoilState( int timeStepIdx, int numCells ) {

    std::vector< double > pressure( numCells );
    std::vector< double > swat( numCells );
    std::vector< double > sgas( numCells );
    std::vector< double > rs( numCells );
    std::vector< double > rv( numCells );

    for( int cellIdx = 0; cellIdx < numCells; ++cellIdx) {

        pressure[cellIdx] = timeStepIdx*1e5 + 1e4 + cellIdx;
        sgas[cellIdx] = timeStepIdx*1e5 +2.2e4 + cellIdx;
        swat[cellIdx] = timeStepIdx*1e5 +2.3e4 + cellIdx;

        // oil vaporization factor
        rv[cellIdx] = timeStepIdx*1e5 +3e4 + cellIdx;
        // gas dissolution factor
        rs[cellIdx] = timeStepIdx*1e5 + 4e4 + cellIdx;
    }

    data::Solution solution;

    solution.insert( "PRESSURE" , UnitSystem::measure::pressure , pressure, data::TargetType::RESTART_SOLUTION );
    solution.insert( "SWAT" , UnitSystem::measure::identity , swat, data::TargetType::RESTART_SOLUTION );
    solution.insert( "SGAS" , UnitSystem::measure::identity , sgas, data::TargetType::RESTART_SOLUTION );
    solution.insert( "RS" , UnitSystem::measure::identity , rs, data::TargetType::RESTART_SOLUTION );
    solution.insert( "RV" , UnitSystem::measure::identity , rv, data::TargetType::RESTART_SOLUTION );

    return solution;
}

template< typename T, typename U >
void compareErtData(const std::vector< T > &src,
                    const std::vector< U > &dst,
                    double tolerance ) {
    BOOST_REQUIRE_EQUAL(src.size(), dst.size());

    for (size_t i = 0; i < src.size(); ++i)
        BOOST_CHECK_CLOSE(src[i], dst[i], tolerance);
}

void compareErtData(const std::vector<int> &src, const std::vector<int> &dst)
{
    BOOST_CHECK_EQUAL_COLLECTIONS( src.begin(), src.end(),
                                   dst.begin(), dst.end() );
}

void checkEgridFile( const EclipseGrid& eclGrid ) {
    auto egridFile = EclIO::EGrid("FOO.EGRID");

    const auto numCells = eclGrid.getNX() * eclGrid.getNY() * eclGrid.getNZ();

    {
        const auto& coord  = egridFile.get<float>("COORD");
        const auto& expect = eclGrid.getCOORD();
        compareErtData(expect, coord, 1e-6);
    }

    {
        const auto& zcorn  = egridFile.get<float>("ZCORN");
        const auto& expect = eclGrid.getZCORN();
        compareErtData(expect, zcorn, 1e-6);
    }

    if (egridFile.hasKey("ACTNUM")) {
        const auto& actnum = egridFile.get<int>("ACTNUM");
        auto expect = eclGrid.getACTNUM();

        if (expect.empty())
            expect.assign(numCells, 1);

        compareErtData(expect, actnum);
    }
}

void checkInitFile( const Deck& deck, const data::Solution& simProps) {
    EclIO::EclFile initFile { "FOO.INIT" };

    if (initFile.hasKey("PORO")) {
        const auto& poro   = initFile.get<float>("PORO");
        const auto& expect = deck["PORO"].back().getSIDoubleData();

        compareErtData(expect, poro, 1e-4);
    }

    if (initFile.hasKey("PERMX")) {
        const auto& expect = deck["PERMX"].back().getSIDoubleData();
        auto        permx  = initFile.get<float>("PERMX");

        for (auto& kx : permx) {
            kx *= 9.869233e-16;
        }

        compareErtData(expect, permx, 1e-4);
    }

    /*
      These keyword should always be in the INIT file, irrespective of
      whether they appear in the inut deck or not.
    */
    BOOST_CHECK_MESSAGE( initFile.hasKey("NTG"), R"(INIT file must have "NTG" array)" );
    BOOST_CHECK_MESSAGE( initFile.hasKey("FIPNUM"), R"(INIT file must have "FIPNUM" array)");
    BOOST_CHECK_MESSAGE( initFile.hasKey("SATNUM"), R"(INIT file must have "SATNUM" array)");

    for (const auto& prop : simProps) {
        BOOST_CHECK_MESSAGE( initFile.hasKey(prop.first), R"(INIT file must have ")" + prop.first + R"(" array)" );
    }
}

void checkRestartFile( int timeStepIdx ) {
    EclIO::ERst rstFile{ "FOO.UNRST" };

    for (int i = 1; i <= timeStepIdx; ++i) {
        if (! rstFile.hasReportStepNumber(i))
            continue;

        auto sol = createBlackoilState( i, 3 * 3 * 3 );

        rstFile.loadReportStepNumber(i);

        const auto& knownVec = rstFile.listOfRstArrays(i);

        if (keywordExists(knownVec, "PRESSURE")) {
            const auto& press = rstFile.getRestartData<float>("PRESSURE", i, 0);
            for( auto& x : sol.data("PRESSURE") )
                x /= Metric::Pressure;

            compareErtData( sol.data("PRESSURE"), press, 1e-4 );
        }

        if (keywordExists(knownVec, "SWAT")) {
            const auto& swat = rstFile.getRestartData<float>("SWAT", i, 0);
            compareErtData( sol.data("SWAT"), swat, 1e-4 );
        }

        if (keywordExists(knownVec, "SGAS")) {
            const auto& sgas = rstFile.getRestartData<float>("SGAS", i, 0);
            compareErtData( sol.data("SGAS"), sgas, 1e-4 );
        }

        if (keywordExists(knownVec, "KRO")) {
            const auto& kro = rstFile.getRestartData<float>("KRO", i, 0);
            BOOST_CHECK_CLOSE(1.0 * i * kro.size(), sum(kro), 1.0e-8);
        }

        if (keywordExists(knownVec, "KRG")) {
            const auto& krg = rstFile.getRestartData<float>("KRG", i, 0);
            BOOST_CHECK_CLOSE(10.0 * i * krg.size(), sum(krg), 1.0e-8);
        }
    }
}

time_t ecl_util_make_date( const int day, const int month, const int year )
{
    const auto ymd = Opm::TimeStampUTC::YMD{ year, month, day };
    return static_cast<time_t>(asTimeT(Opm::TimeStampUTC{ymd}));
}

} // Anonymous namespace

BOOST_AUTO_TEST_CASE(EclipseIOIntegration) {
    const char *deckString =
        "RUNSPEC\n"
        "UNIFOUT\n"
        "OIL\n"
        "GAS\n"
        "WATER\n"
        "METRIC\n"
        "DIMENS\n"
        "3 3 3/\n"
        "GRID\n"
        "PORO\n"
        "27*0.3 /\n"
        "PERMX\n"
        "27*1 /\n"
        "INIT\n"
        "DXV\n"
        "1.0 2.0 3.0 /\n"
        "DYV\n"
        "4.0 5.0 6.0 /\n"
        "DZV\n"
        "7.0 8.0 9.0 /\n"
        "TOPS\n"
        "9*100 /\n"
        "PORO \n"
        "  27*0.15 /\n"
        "PROPS\n"
        "REGIONS\n"
        "SATNUM\n"
        "27*2 /\n"
        "FIPNUM\n"
        "27*3 /\n"
        "SOLUTION\n"
        "RPTRST\n"
        "BASIC=2\n"
        "/\n"
        "SCHEDULE\n"
        "TSTEP\n"
        "1.0 2.0 3.0 4.0 5.0 6.0 7.0 /\n"
        "WELSPECS\n"
        "'INJ' 'G' 1 1 2000 'GAS' /\n"
        "'PROD' 'G' 3 3 1000 'OIL' /\n"
        "/\n";

    auto write_and_check = [&]( int first = 1, int last = 5 ) {
        auto deck = Parser().parseString( deckString);
        auto es = EclipseState( deck );
        auto& eclGrid = es.getInputGrid();
        auto python = std::make_shared<Python>();
        Schedule schedule(deck, es, python);
        SummaryConfig summary_config( deck, schedule, es.fieldProps(), es.aquifer());
        SummaryState st(TimeService::now());
        es.getIOConfig().setBaseName( "FOO" );

        EclipseIO eclWriter( es, eclGrid , schedule, summary_config);

        using measure = UnitSystem::measure;
        using TargetType = data::TargetType;
        auto start_time = ecl_util_make_date( 10, 10, 2008 );
        std::vector<double> tranx(3*3*3);
        std::vector<double> trany(3*3*3);
        std::vector<double> tranz(3*3*3);
        data::Solution eGridProps {
            { "TRANX", { measure::transmissibility, tranx, TargetType::INIT } },
            { "TRANY", { measure::transmissibility, trany, TargetType::INIT } },
            { "TRANZ", { measure::transmissibility, tranz, TargetType::INIT } },
        };

        std::map<std::string, std::vector<int>> int_data =  {{"STR_ULONGNAME" , {1,1,1,1,1,1,1,1} } };

        std::vector<int> v(27); v[2] = 67; v[26] = 89;
        int_data["STR_V"] = v;

        eclWriter.writeInitial( );

        BOOST_CHECK_THROW( eclWriter.writeInitial( eGridProps , int_data) , std::invalid_argument);

        int_data.erase("STR_ULONGNAME");
        eclWriter.writeInitial( eGridProps , int_data );

        data::Wells wells;
        data::GroupAndNetworkValues grp_nwrk;

        for( int i = first; i < last; ++i ) {
            data::Solution sol = createBlackoilState( i, 3 * 3 * 3 );
            sol.insert("KRO", measure::identity , std::vector<double>(3*3*3 , i), TargetType::RESTART_AUXILIARY);
            sol.insert("KRG", measure::identity , std::vector<double>(3*3*3 , i*10), TargetType::RESTART_AUXILIARY);

            Action::State action_state;
            WellTestState wtest_state;
            UDQState udq_state(1);
            RestartValue restart_value(sol, wells, grp_nwrk, {});
            auto first_step = ecl_util_make_date( 10 + i, 11, 2008 );
            eclWriter.writeTimeStep( action_state,
                                     wtest_state,
                                     st,
                                     udq_state,
                                     i,
                                     false,
                                     first_step - start_time,
                                     std::move(restart_value));

            checkRestartFile( i );
        }

        checkInitFile( deck , eGridProps);
        checkEgridFile( eclGrid );

        EclIO::EclFile initFile("FOO.INIT");
        BOOST_CHECK_MESSAGE( initFile.hasKey("STR_V"), R"(INIT file must have "STR_V" array)" );
        const auto& kw = initFile.get<int>("STR_V");
        BOOST_CHECK_EQUAL(67, kw[ 2]);
        BOOST_CHECK_EQUAL(89, kw[26]);

        std::ifstream file( "FOO.UNRST", std::ios::binary );
        std::streampos file_size = 0;

        file_size = file.tellg();
        file.seekg( 0, std::ios::end );
        file_size = file.tellg() - file_size;

        return file_size;
    };

    /*
     * write the file and calculate the file size. FOO.UNRST should be
     * overwitten for every time step, i.e. the file size should not change
     * between runs.  This is to verify that UNRST files are properly
     * overwritten, which they used not to.
     *
     * * https://github.com/OPM/opm-simulators/issues/753
     * * https://github.com/OPM/opm-output/pull/61
     */

    WorkArea work_area("test_ecl_writer");
    const auto file_size = write_and_check();

    for( int i = 0; i < 3; ++i )
        BOOST_CHECK_EQUAL( file_size, write_and_check() );

    /*
     * check that "restarting" and writing over previous timesteps does not
     * change the file size, if the total amount of steps are the same
     */
    BOOST_CHECK_EQUAL( file_size, write_and_check( 3, 5 ) );

    /* verify that adding steps from restart also increases file size */
    BOOST_CHECK( file_size < write_and_check( 3, 7 ) );

    /*
     * verify that restarting a simulation, then writing fewer steps truncates
     * the file
     */
    BOOST_CHECK_EQUAL( file_size, write_and_check( 3, 5 ) );
}
