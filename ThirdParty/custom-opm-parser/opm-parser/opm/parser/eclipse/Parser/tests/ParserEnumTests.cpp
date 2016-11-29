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


#define BOOST_TEST_MODULE ParserEnumTests
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Parser/ParserEnums.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(TestItemSizeEnum2String) {
    BOOST_CHECK_EQUAL( "ALL"    , ParserItemSizeEnum2String(ALL));
    BOOST_CHECK_EQUAL( "SINGLE" , ParserItemSizeEnum2String(SINGLE));
}


BOOST_AUTO_TEST_CASE(TestItemSizeEnumFromString) {
    BOOST_CHECK_THROW( ParserItemSizeEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( ALL , ParserItemSizeEnumFromString("ALL"));
    BOOST_CHECK_EQUAL( SINGLE , ParserItemSizeEnumFromString("SINGLE"));
}



BOOST_AUTO_TEST_CASE(TestItemSizeEnumLoop) {
    BOOST_CHECK_EQUAL( ALL    , ParserItemSizeEnumFromString( ParserItemSizeEnum2String( ALL ) ));
    BOOST_CHECK_EQUAL( SINGLE , ParserItemSizeEnumFromString( ParserItemSizeEnum2String( SINGLE ) ));

    BOOST_CHECK_EQUAL( "ALL"    , ParserItemSizeEnum2String(ParserItemSizeEnumFromString(  "ALL" ) ));
    BOOST_CHECK_EQUAL( "SINGLE" , ParserItemSizeEnum2String(ParserItemSizeEnumFromString(  "SINGLE" ) ));
}

/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestKeywordSizeEnum2String) {
    BOOST_CHECK_EQUAL( "SLASH_TERMINATED" , ParserKeywordSizeEnum2String(SLASH_TERMINATED));
    BOOST_CHECK_EQUAL( "FIXED"     , ParserKeywordSizeEnum2String(FIXED));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(OTHER_KEYWORD_IN_DECK));
    BOOST_CHECK_EQUAL( "UNKNOWN"     , ParserKeywordSizeEnum2String(UNKNOWN));
}


BOOST_AUTO_TEST_CASE(TestKeywordSizeEnumFromString) {
    BOOST_CHECK_THROW( ParserKeywordSizeEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( FIXED     , ParserKeywordSizeEnumFromString("FIXED"));
    BOOST_CHECK_EQUAL( SLASH_TERMINATED , ParserKeywordSizeEnumFromString("SLASH_TERMINATED"));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(OTHER_KEYWORD_IN_DECK));
    BOOST_CHECK_EQUAL( "UNKNOWN"     , ParserKeywordSizeEnum2String(UNKNOWN));
}



BOOST_AUTO_TEST_CASE(TestKeywordSizeEnumLoop) {
    BOOST_CHECK_EQUAL( FIXED     , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( FIXED ) ));
    BOOST_CHECK_EQUAL( SLASH_TERMINATED , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( SLASH_TERMINATED ) ));
    BOOST_CHECK_EQUAL( OTHER_KEYWORD_IN_DECK     , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( OTHER_KEYWORD_IN_DECK ) ));
    BOOST_CHECK_EQUAL( UNKNOWN , ParserKeywordSizeEnumFromString( ParserKeywordSizeEnum2String( UNKNOWN ) ));

    BOOST_CHECK_EQUAL( "FIXED"     , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "FIXED" ) ));
    BOOST_CHECK_EQUAL( "SLASH_TERMINATED" , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "SLASH_TERMINATED" ) ));
    BOOST_CHECK_EQUAL( "OTHER_KEYWORD_IN_DECK"     , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "OTHER_KEYWORD_IN_DECK" ) ));
    BOOST_CHECK_EQUAL( "UNKNOWN" , ParserKeywordSizeEnum2String(ParserKeywordSizeEnumFromString(  "UNKNOWN" ) ));
}


/*****************************************************************/


BOOST_AUTO_TEST_CASE(TestValueTypeEnum2String) {
    BOOST_CHECK_EQUAL( "INT"    , ParserValueTypeEnum2String(INT));
    BOOST_CHECK_EQUAL( "FLOAT" , ParserValueTypeEnum2String(FLOAT));
    BOOST_CHECK_EQUAL( "STRING"    , ParserValueTypeEnum2String(STRING));
}


BOOST_AUTO_TEST_CASE(TestValueTypeEnumFromString) {
    BOOST_CHECK_THROW( ParserValueTypeEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( INT , ParserValueTypeEnumFromString("INT"));
    BOOST_CHECK_EQUAL( STRING , ParserValueTypeEnumFromString("STRING"));
    BOOST_CHECK_EQUAL( FLOAT , ParserValueTypeEnumFromString("FLOAT"));
}



BOOST_AUTO_TEST_CASE(TestValueTypeEnumLoop) {
    BOOST_CHECK_EQUAL( INT    , ParserValueTypeEnumFromString( ParserValueTypeEnum2String( INT ) ));
    BOOST_CHECK_EQUAL( FLOAT , ParserValueTypeEnumFromString( ParserValueTypeEnum2String( FLOAT ) ));
    BOOST_CHECK_EQUAL( STRING    , ParserValueTypeEnumFromString( ParserValueTypeEnum2String( STRING ) ));

    BOOST_CHECK_EQUAL( "INT"    , ParserValueTypeEnum2String(ParserValueTypeEnumFromString(  "INT" ) ));
    BOOST_CHECK_EQUAL( "FLOAT" , ParserValueTypeEnum2String(ParserValueTypeEnumFromString(  "FLOAT" ) ));
    BOOST_CHECK_EQUAL( "STRING"    , ParserValueTypeEnum2String(ParserValueTypeEnumFromString(  "STRING" ) ));
}


/*****************************************************************/

BOOST_AUTO_TEST_CASE(TestKeywordActionEnum2String) {
    BOOST_CHECK_EQUAL( "INTERNALIZE"     , ParserKeywordActionEnum2String(INTERNALIZE));
    BOOST_CHECK_EQUAL( "IGNORE"          , ParserKeywordActionEnum2String(IGNORE));
    BOOST_CHECK_EQUAL( "IGNORE_WARNING"  , ParserKeywordActionEnum2String(IGNORE_WARNING));
    BOOST_CHECK_EQUAL( "THROW_EXCEPTION" , ParserKeywordActionEnum2String(THROW_EXCEPTION));
}


BOOST_AUTO_TEST_CASE(TestKeywordActionEnumFromString) {
    BOOST_CHECK_THROW( ParserKeywordActionEnumFromString("XXX") , std::invalid_argument );
    BOOST_CHECK_EQUAL( INTERNALIZE    , ParserKeywordActionEnumFromString("INTERNALIZE"));
    BOOST_CHECK_EQUAL( IGNORE_WARNING , ParserKeywordActionEnumFromString("IGNORE_WARNING"));
    BOOST_CHECK_EQUAL( IGNORE  , ParserKeywordActionEnumFromString("IGNORE"));
    BOOST_CHECK_EQUAL( THROW_EXCEPTION  , ParserKeywordActionEnumFromString("THROW_EXCEPTION"));
}



BOOST_AUTO_TEST_CASE(TestKeywordActionEnumLoop) {
    BOOST_CHECK_EQUAL( INTERNALIZE    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( INTERNALIZE ) ));
    BOOST_CHECK_EQUAL( IGNORE , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( IGNORE ) ));
    BOOST_CHECK_EQUAL( IGNORE_WARNING    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( IGNORE_WARNING ) ));
    BOOST_CHECK_EQUAL( THROW_EXCEPTION    , ParserKeywordActionEnumFromString( ParserKeywordActionEnum2String( THROW_EXCEPTION ) ));

    BOOST_CHECK_EQUAL( "INTERNALIZE"    , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "INTERNALIZE" ) ));
    BOOST_CHECK_EQUAL( "IGNORE" , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "IGNORE" ) ));
    BOOST_CHECK_EQUAL( "IGNORE_WARNING"    , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "IGNORE_WARNING" ) ));
    BOOST_CHECK_EQUAL( "THROW_EXCEPTION" , ParserKeywordActionEnum2String(ParserKeywordActionEnumFromString(  "THROW_EXCEPTION" ) ));
}


