/*
  Copyright 2014 Statoil ASA.

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

#define BOOST_TEST_MODULE WellPropertiesTest
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellInjectionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/WellProductionProperties.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <string>

namespace {
    namespace WCONHIST {
        std::string all_specified_CMODE_BHP() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'BHP' 1 2 3/\n/\n";

            return input;
        }



        std::string all_specified() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'ORAT' 1 2 3/\n/\n";

            return input;
        }

        std::string orat_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'WRAT' 1* 2 3/\n/\n";

            return input;
        }

        std::string owrat_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'GRAT' 1* 1* 3/\n/\n";

            return input;
        }

        std::string all_defaulted() {
            const std::string input =
                "WCONHIST\n"
                "'P' 'OPEN' 'LRAT'/\n/\n";

            return input;
        }

        std::string all_defaulted_with_bhp() {
            const std::string input =
                "WCONHIST\n"
                "-- 1    2     3      4-9 10\n"
                "   'P' 'OPEN' 'RESV' 6*  500/\n/\n";

            return input;
        }

        Opm::WellProductionProperties properties(const std::string& input) {
            Opm::Parser parser;

            Opm::DeckPtr deck = parser.parseString(input, Opm::ParseContext());
            const auto& record = deck->getKeyword("WCONHIST").getRecord(0);
            Opm::WellProductionProperties hist = Opm::WellProductionProperties::history( 100 , record);;

            return hist;
        }
    } // namespace WCONHIST

    namespace WCONPROD {
        std::string
        all_specified_CMODE_BHP()
        {
            const std::string input =
                "WCONPROD\n"
                "'P' 'OPEN' 'BHP' 1 2 3/\n/\n";

            return input;
        }


        Opm::WellProductionProperties
        properties(const std::string& input)
        {
            Opm::Parser parser;

            Opm::DeckPtr             deck   = parser.parseString(input, Opm::ParseContext());
            const auto& kwd     = deck->getKeyword("WCONHIST");
            const auto&  record = kwd.getRecord(0);
            Opm::WellProductionProperties pred = Opm::WellProductionProperties::prediction( record, false );

            return pred;
        }
    }

} // namespace anonymous

BOOST_AUTO_TEST_CASE(WCH_All_Specified_BHP_Defaulted)
{
    const Opm::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_specified());

    // WCONHIST always supports {O,W,G}RAT, LRAT, and
    // RESV--irrespective of actual specification.
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::RESV));

    BOOST_CHECK_EQUAL(p.controlMode , Opm::WellProducer::ORAT);

    // BHP must be explicitly provided/specified
    BOOST_CHECK(! p.hasProductionControl(Opm::WellProducer::BHP));
}

BOOST_AUTO_TEST_CASE(WCH_ORAT_Defaulted_BHP_Defaulted)
{
    const Opm::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::orat_defaulted());

    // WCONHIST always supports {O,W,G}RAT, LRAT, and
    // RESV--irrespective of actual specification.
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::RESV));
    BOOST_CHECK_EQUAL(p.controlMode , Opm::WellProducer::WRAT);

    // BHP must be explicitly provided/specified
    BOOST_CHECK(! p.hasProductionControl(Opm::WellProducer::BHP));
}

BOOST_AUTO_TEST_CASE(WCH_OWRAT_Defaulted_BHP_Defaulted)
{
    const Opm::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::owrat_defaulted());

    // WCONHIST always supports {O,W,G}RAT, LRAT, and
    // RESV--irrespective of actual specification.
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::RESV));
    BOOST_CHECK_EQUAL(p.controlMode , Opm::WellProducer::GRAT);

    // BHP must be explicitly provided/specified
    BOOST_CHECK(! p.hasProductionControl(Opm::WellProducer::BHP));
}

BOOST_AUTO_TEST_CASE(WCH_Rates_Defaulted_BHP_Defaulted)
{
    const Opm::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted());

    // WCONHIST always supports {O,W,G}RAT, LRAT, and
    // RESV--irrespective of actual specification.
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::RESV));
    BOOST_CHECK_EQUAL(p.controlMode , Opm::WellProducer::LRAT);

    // BHP must be explicitly provided/specified
    BOOST_CHECK(! p.hasProductionControl(Opm::WellProducer::BHP));
}

BOOST_AUTO_TEST_CASE(WCH_Rates_Defaulted_BHP_Specified)
{
    const Opm::WellProductionProperties& p =
        WCONHIST::properties(WCONHIST::all_defaulted_with_bhp());

    // WCONHIST always supports {O,W,G}RAT, LRAT, and
    // RESV--irrespective of actual specification.
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::ORAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::WRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::GRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::LRAT));
    BOOST_CHECK(p.hasProductionControl(Opm::WellProducer::RESV));

    BOOST_CHECK_EQUAL(p.controlMode , Opm::WellProducer::RESV);

    /*
      BHP in WCONHIST is not an available control; just information
      about the historical BHP.
    */
    BOOST_CHECK_EQUAL(false , p.hasProductionControl(Opm::WellProducer::BHP));
}


BOOST_AUTO_TEST_CASE(BHP_CMODE)
{
    BOOST_CHECK_THROW( WCONHIST::properties(WCONHIST::all_specified_CMODE_BHP()) , std::invalid_argument);
    BOOST_CHECK_THROW( WCONPROD::properties(WCONPROD::all_specified_CMODE_BHP()) , std::invalid_argument);
}




BOOST_AUTO_TEST_CASE(CMODE_DEFAULT) {
    const Opm::WellProductionProperties Pproperties;
    const Opm::WellInjectionProperties Iproperties;

    BOOST_CHECK_EQUAL( Pproperties.controlMode , Opm::WellProducer::CMODE_UNDEFINED );
    BOOST_CHECK_EQUAL( Iproperties.controlMode , Opm::WellInjector::CMODE_UNDEFINED );
}
