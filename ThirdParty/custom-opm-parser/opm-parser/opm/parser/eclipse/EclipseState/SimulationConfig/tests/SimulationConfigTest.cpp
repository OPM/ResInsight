/*
  Copyright 2015 Statoil ASA.

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



#define BOOST_TEST_MODULE SimulationConfigTests

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperties.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>


using namespace Opm;

const std::string& inputStr = "RUNSPEC\n"
                              "EQLOPTS\n"
                              "THPRES /\n"
                              "DIMENS\n"
                              "10 3 4 /\n"
                              "\n"
                              "GRID\n"
                              "REGIONS\n"
                              "EQLNUM\n"
                              "10*1 10*2 100*3 /\n "
                              "\n"

                              "SOLUTION\n"
                              "THPRES\n"
                              "1 2 12.0/\n"
                              "1 3 5.0/\n"
                              "2 3 7.0/\n"
                              "/\n"
                              "\n";

const std::string& inputStr_noTHPRES = "RUNSPEC\n"
                                       "EQLOPTS\n"
                                       "DIMENS\n"
                                       "10 3 4 /\n"
                                       "\n"
                                       "GRID\n"
                                       "REGIONS\n"
                                       "EQLNUM\n"
                                       "10*1 10*2 100*3 /\n "
                                       "\n"
                                       "SOLUTION\n"
                                       "\n";

const std::string& inputStr_cpr = "RUNSPEC\n"
    "CPR\n"
    "/\n"
    "SUMMARY\n";


const std::string& inputStr_INVALID = "RUNSPEC\n"
    "CPR\n"
    "WEll 10 10 17/"
    "/\n"
    "SUMMARY\n";



const std::string& inputStr_cpr_in_SUMMARY = "SUMMARY\n"
        "CPR\n"
        "well1 10 27 10/\n/\n";

const std::string& inputStr_cpr_BOTH = "RUNSPEC\n"
    "CPR\n"
    "/\n"
    "SUMMARY\n"
    "CPR\n"
    "well1 10 20 30/\n/\n";

const std::string& inputStr_vap_dis = "RUNSPEC\n"
                                      "VAPOIL\n"
                                      "DISGAS\n"
                                      "DIMENS\n"
                                      "10 3 4 /\n"
                                      "\n"
                                      "GRID\n"
                                      "REGIONS\n"
                                      "\n";

static DeckPtr createDeck(const ParseContext& parseContext , const std::string& input) {
    Opm::Parser parser;
    return parser.parseString(input, parseContext);
}


BOOST_AUTO_TEST_CASE(SimulationConfigGetThresholdPressureTableTest) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext , inputStr);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SimulationConfigConstPtr simulationConfigPtr;
    BOOST_CHECK_NO_THROW(simulationConfigPtr = std::make_shared
            <const SimulationConfig>(*deck, ep));
}


BOOST_AUTO_TEST_CASE(SimulationConfigNOTHPRES) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr_noTHPRES);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK( !simulationConfig.hasThresholdPressure() );
}

BOOST_AUTO_TEST_CASE(SimulationConfigCPRNotUsed) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr_noTHPRES);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK( ! simulationConfig.useCPR());
}

BOOST_AUTO_TEST_CASE(SimulationConfigCPRUsed) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr_cpr);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SUMMARYSection summary(*deck);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK(     simulationConfig.useCPR() );
    BOOST_CHECK(  !  summary.hasKeyword("CPR") );
}


BOOST_AUTO_TEST_CASE(SimulationConfigCPRInSUMMARYSection) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr_cpr_in_SUMMARY);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SUMMARYSection summary(*deck);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK( ! simulationConfig.useCPR());
    BOOST_CHECK(   summary.hasKeyword("CPR"));
}


BOOST_AUTO_TEST_CASE(SimulationConfigCPRBoth) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr_cpr_BOTH);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SUMMARYSection summary(*deck);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK(  simulationConfig.useCPR());
    BOOST_CHECK(  summary.hasKeyword("CPR"));

    const auto& cpr = summary.getKeyword<ParserKeywords::CPR>();
    const auto& record = cpr.getRecord(0);
    BOOST_CHECK_EQUAL( 1 , cpr.size());
    BOOST_CHECK_EQUAL( record.getItem<ParserKeywords::CPR::WELL>().get< std::string >(0) , "well1");
    BOOST_CHECK_EQUAL( record.getItem<ParserKeywords::CPR::I>().get< int >(0) , 10);
    BOOST_CHECK_EQUAL( record.getItem<ParserKeywords::CPR::J>().get< int >(0) , 20);
    BOOST_CHECK_EQUAL( record.getItem<ParserKeywords::CPR::K>().get< int >(0) , 30);
}


BOOST_AUTO_TEST_CASE(SimulationConfigCPRRUnspecWithData) {
    ParseContext parseContext;
    BOOST_CHECK_THROW( createDeck(parseContext , inputStr_INVALID) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(SimulationConfig_VAPOIL_DISGAS) {
    ParseContext parseContext;
    DeckPtr deck = createDeck(parseContext, inputStr);
    TableManager tm(*deck);
    EclipseGrid eg(10, 3, 4);
    Eclipse3DProperties ep(*deck, tm, eg);
    SimulationConfig simulationConfig(*deck, ep);
    BOOST_CHECK_EQUAL( false , simulationConfig.hasDISGAS());
    BOOST_CHECK_EQUAL( false , simulationConfig.hasVAPOIL());

    DeckPtr deck_vd = createDeck(parseContext, inputStr_vap_dis);
    TableManager tm_vd(*deck_vd);
    EclipseGrid eg_vd(10, 3, 4);
    Eclipse3DProperties ep_vd(*deck_vd, tm, eg);
    SimulationConfig simulationConfig_vd(*deck_vd, ep_vd);
    BOOST_CHECK_EQUAL( true , simulationConfig_vd.hasDISGAS());
    BOOST_CHECK_EQUAL( true , simulationConfig_vd.hasVAPOIL());
}
