/*
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE ParserIntegrationTests
#include <math.h>

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;



BOOST_AUTO_TEST_CASE( parse_VFPPROD_OK ) {
    ParserPtr parser(new Parser());
    std::string file("testdata/integration_tests/VFPPROD/VFPPROD1");
    BOOST_CHECK( parser->isRecognizedKeyword("VFPPROD"));

    DeckPtr deck =  parser->parseFile(file, ParseContext());

    const auto& VFPPROD1 = deck->getKeyword("VFPPROD" , 0);
    const auto& BPR = deck->getKeyword("BPR" , 0);
    const auto& VFPPROD2 = deck->getKeyword("VFPPROD" , 1);

    BOOST_CHECK_EQUAL( 573U  , VFPPROD1.size() );
    BOOST_CHECK_EQUAL( 1U    , BPR.size());
    BOOST_CHECK_EQUAL( 573U  , VFPPROD2.size());

    {
        const auto& record = VFPPROD1.getRecord(0);

        BOOST_CHECK_EQUAL( record.getItem("TABLE").get< int >(0) , 32 );
        BOOST_CHECK_EQUAL( record.getItem("DATUM_DEPTH").getSIDouble(0) , 394);
        BOOST_CHECK_EQUAL( record.getItem("RATE_TYPE").get< std::string >(0) , "LIQ");
        BOOST_CHECK_EQUAL( record.getItem("WFR").get< std::string >(0) , "WCT");
        BOOST_CHECK_EQUAL( record.getItem("GFR").get< std::string >(0) , "GOR");
    }

    {
        const auto& record = VFPPROD1.getRecord(1);
        const auto& item = record.getItem("FLOW_VALUES");

        BOOST_CHECK_EQUAL( item.size() , 12 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   100 );
        BOOST_CHECK_EQUAL( item.get< double >(11) , 20000 );
    }

    {
        const auto& record = VFPPROD1.getRecord(2);
        const auto& item = record.getItem("THP_VALUES");

        BOOST_CHECK_EQUAL( item.size() , 7 );
        BOOST_CHECK_CLOSE( item.get< double >(0)  , 16.01 , 0.0001 );
        BOOST_CHECK_CLOSE( item.get< double >(6) ,  61.01 , 0.0001 );
    }

    {
        const auto& record = VFPPROD1.getRecord(3);
        const auto& item = record.getItem("WFR_VALUES");

        BOOST_CHECK_EQUAL( item.size() , 9 );
        BOOST_CHECK_CLOSE( item.get< double >(1)  , 0.1 , 0.0001 );
        BOOST_CHECK_CLOSE( item.get< double >(7) ,  0.9 , 0.0001 );
    }

    {
        const auto& record = VFPPROD1.getRecord(4);
        const auto& item = record.getItem("GFR_VALUES");

        BOOST_CHECK_EQUAL( item.size() , 9 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   90 );
        BOOST_CHECK_EQUAL( item.get< double >(8) , 10000 );
    }

    {
        const auto& record = VFPPROD1.getRecord(5);
        const auto& item = record.getItem("ALQ_VALUES");

        BOOST_CHECK_EQUAL( item.size() , 1 );
        BOOST_CHECK_EQUAL( item.get< double >(0)  ,   0 );
    }

    {
        const auto& record = VFPPROD1.getRecord(6);

        {
            const auto& item = record.getItem("THP_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }

        {
            const auto& item = record.getItem("WFR_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("GFR_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("ALQ_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("VALUES");
            BOOST_CHECK_EQUAL( item.size() , 12 );
            BOOST_CHECK_EQUAL( item.get< double >(0) , 44.85 );
            BOOST_CHECK_EQUAL( item.get< double >(11) , 115.14 );
        }
    }

    {
        const auto& record = VFPPROD1.getRecord(572);
        {
            const auto& item = record.getItem("THP_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 7 );
        }
        {
            const auto& item = record.getItem("WFR_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 9 );
        }
        {
            const auto& item = record.getItem("GFR_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 9 );
        }
        {
            const auto& item = record.getItem("ALQ_INDEX");
            BOOST_CHECK_EQUAL( item.size() , 1 );
            BOOST_CHECK_EQUAL( item.get< int >(0) , 1 );
        }
        {
            const auto& item = record.getItem("VALUES");
            BOOST_CHECK_EQUAL( item.size() , 12 );
            BOOST_CHECK_EQUAL( item.get< double >(0) , 100.80 );
            BOOST_CHECK_EQUAL( item.get< double >(11) , 147.79 );
        }
    }
}
