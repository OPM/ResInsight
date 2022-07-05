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


#include <stdexcept>
#include <sstream>

#define BOOST_TEST_MODULE DeckTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Deck/DeckTree.hpp>
#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckView.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

#include "src/opm/input/eclipse/Parser/raw/RawRecord.hpp"

using namespace Opm;

BOOST_AUTO_TEST_CASE(hasKeyword_empty_returnFalse) {
    Deck deck;
    BOOST_CHECK_EQUAL(false, deck.hasKeyword("Bjarne"));
    BOOST_CHECK_THROW( deck["Bjarne"].back() , std::exception);
}

std::pair<std::vector<Dimension>, std::vector<Dimension>> make_dims() {
    UnitSystem metric(UnitSystem::UnitType::UNIT_TYPE_METRIC);
    return std::make_pair<std::vector<Dimension>, std::vector<Dimension>>({metric.getDimension("Length")}, {metric.getDimension("Length")});
}



BOOST_AUTO_TEST_CASE(getKeywordList_empty_list) {
    Deck deck;
    auto kw_list = deck.getKeywordList("TRULS");
    BOOST_CHECK_EQUAL( kw_list.size() , 0U );
}

BOOST_AUTO_TEST_CASE(getKeyword_singlekeyword_outRange_throws) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    BOOST_CHECK_THROW(deck["GRID"][10], std::exception);
}


BOOST_AUTO_TEST_CASE(getKeywordList_returnOK) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    BOOST_CHECK_NO_THROW( deck.getKeywordList("GRID") );
}


BOOST_AUTO_TEST_CASE(getKeyword_indexok_returnskeyword) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    BOOST_CHECK_NO_THROW(deck[0]);
}

BOOST_AUTO_TEST_CASE(numKeyword_singlekeyword_return1) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    BOOST_CHECK_EQUAL(1U , deck.count("GRID"));
}


BOOST_AUTO_TEST_CASE(numKeyword_twokeyword_return2) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    BOOST_CHECK_EQUAL(2U , deck.count("GRID"));
    BOOST_CHECK_EQUAL(0U , deck.count("GRID_BUG"));
}


BOOST_AUTO_TEST_CASE(size_twokeyword_return2) {
    Deck deck;
    Parser parser;
    DeckKeyword keyword( parser.getKeyword("GRID"));
    deck.addKeyword(keyword);
    deck.addKeyword(keyword);
    BOOST_CHECK_EQUAL(2U , deck.size());

    BOOST_CHECK_THROW( deck["GRID"][3], std::exception);
}

BOOST_AUTO_TEST_CASE(getKeywordList_OK) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));

    const auto& keywordList = deck.getKeywordList("GRID");
    BOOST_CHECK_EQUAL( 3U , keywordList.size() );
}


BOOST_AUTO_TEST_CASE(keywordList_getbyindexoutofbounds_exceptionthrown) {
    Parser parser;
    Deck deck;
    BOOST_CHECK_THROW(deck[0], std::exception);
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("INIT")));
    BOOST_CHECK_NO_THROW(deck[2]);
    BOOST_CHECK_THROW(deck[3], std::exception);
}

BOOST_AUTO_TEST_CASE(keywordList_getbyindex_correctkeywordreturned) {
    Deck deck;
    Parser parser;
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("GRID")));
    deck.addKeyword( DeckKeyword( parser.getKeyword("INIT")));
    BOOST_CHECK_EQUAL("GRID",  deck[0].name());
    BOOST_CHECK_EQUAL("GRID",  deck[1].name());
    BOOST_CHECK_EQUAL("INIT", deck[2].name());
}

BOOST_AUTO_TEST_CASE(set_and_get_data_file) {
    Deck deck;
    BOOST_CHECK_EQUAL("", deck.getInputPath());
    BOOST_CHECK_EQUAL("some/path", deck.makeDeckPath("some/path"));
    BOOST_CHECK_EQUAL("/abs/path", deck.makeDeckPath("/abs/path"));
}

BOOST_AUTO_TEST_CASE(DummyDefaultsString) {
    DeckItem deckStringItem("TEST", std::string() );
    BOOST_CHECK_EQUAL(deckStringItem.data_size(), 0U);

    deckStringItem.push_backDummyDefault<std::string>();
    BOOST_CHECK_EQUAL(deckStringItem.data_size(), 1U);
    BOOST_CHECK_EQUAL(true, deckStringItem.defaultApplied(0));
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(0), std::exception);
}

BOOST_AUTO_TEST_CASE(GetStringAtIndex_NoData_ExceptionThrown) {
    DeckItem deckStringItem( "TEST", std::string() );
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(0), std::exception);
    deckStringItem.push_back("SA");
    BOOST_CHECK_THROW(deckStringItem.get< std::string >(1), std::exception);
}

BOOST_AUTO_TEST_CASE(size_variouspushes_sizecorrect) {
    DeckItem deckStringItem( "TEST", std::string() );

    BOOST_CHECK_EQUAL(0U, deckStringItem.data_size());
    deckStringItem.push_back("WELL-3");
    BOOST_CHECK_EQUAL(1U, deckStringItem.data_size());

    deckStringItem.push_back("WELL-4");
    deckStringItem.push_back("WELL-5");
    BOOST_CHECK_EQUAL(3U, deckStringItem.data_size());
}

BOOST_AUTO_TEST_CASE(DefaultNotAppliedString) {
    DeckItem deckStringItem( "TEST", std::string() );
    BOOST_CHECK( deckStringItem.data_size() == 0 );

    deckStringItem.push_back( "FOO") ;
    BOOST_CHECK( deckStringItem.data_size() == 1 );
    BOOST_CHECK( deckStringItem.get< std::string >(0) == "FOO" );
    BOOST_CHECK( !deckStringItem.defaultApplied(0) );
}

BOOST_AUTO_TEST_CASE(DefaultAppliedString) {
    DeckItem deckStringItem( "TEST", std::string() );
    BOOST_CHECK( deckStringItem.data_size() == 0 );

    deckStringItem.push_backDefault( "FOO" );
    BOOST_CHECK( deckStringItem.data_size() == 1 );
    BOOST_CHECK( deckStringItem.get< std::string >(0) == "FOO" );
    BOOST_CHECK( deckStringItem.defaultApplied(0) );
}


BOOST_AUTO_TEST_CASE(PushBackMultipleString) {
    DeckItem stringItem( "TEST", std::string() );
    stringItem.push_back("Heisann ", 100U );
    BOOST_CHECK_EQUAL( 100U , stringItem.data_size() );
    for (size_t i=0; i < 100; i++)
        BOOST_CHECK_EQUAL("Heisann " , stringItem.get< std::string >(i));
}

BOOST_AUTO_TEST_CASE(GetDoubleAtIndex_NoData_ExceptionThrown) {
    auto dims = make_dims();
    DeckItem deckDoubleItem( "TEST", double(), dims.first, dims.second );
    printf("Current type: %s \n",tag_name(deckDoubleItem.getType()).c_str());
    BOOST_CHECK(deckDoubleItem.getType() == type_tag::fdouble);

    BOOST_CHECK_THROW(deckDoubleItem.get< double >(0), std::exception);
    deckDoubleItem.push_back(1.89);
    BOOST_CHECK_THROW(deckDoubleItem.get< double >(1), std::exception);
}


BOOST_AUTO_TEST_CASE(sizeDouble_correct) {
    auto dims = make_dims();
    DeckItem deckDoubleItem( "TEST", double(), dims.first, dims.second);

    BOOST_CHECK_EQUAL( 0U , deckDoubleItem.data_size());
    deckDoubleItem.push_back( 100.0 );
    BOOST_CHECK_EQUAL( 1U , deckDoubleItem.data_size());

    deckDoubleItem.push_back( 100.0 );
    deckDoubleItem.push_back( 100.0 );
    BOOST_CHECK_EQUAL( 3U , deckDoubleItem.data_size());
}



BOOST_AUTO_TEST_CASE(SetInDeck) {
    auto dims = make_dims();
    DeckItem deckDoubleItem( "TEST", double(), dims.first, dims.second);
    BOOST_CHECK( deckDoubleItem.data_size() == 0 );

    deckDoubleItem.push_backDefault( 1.0 );
    BOOST_CHECK( deckDoubleItem.data_size() == 1 );
    BOOST_CHECK_EQUAL( true , deckDoubleItem.defaultApplied(0) );

    deckDoubleItem.push_back( 10.0 );
    BOOST_CHECK( deckDoubleItem.data_size() == 2 );
    BOOST_CHECK_EQUAL( false , deckDoubleItem.defaultApplied(1) );

    deckDoubleItem.push_backDefault( 1.0 );
    BOOST_CHECK( deckDoubleItem.data_size() == 3 );
    BOOST_CHECK_EQUAL( true , deckDoubleItem.defaultApplied(2) );
}

BOOST_AUTO_TEST_CASE(DummyDefaultsDouble) {
    auto dims = make_dims();
    DeckItem deckDoubleItem( "TEST", double(), dims.first, dims.second);
    BOOST_CHECK_EQUAL(deckDoubleItem.data_size(), 0U);

    deckDoubleItem.push_backDummyDefault<double>();
    BOOST_CHECK_EQUAL(deckDoubleItem.data_size(), 1U);
    BOOST_CHECK_EQUAL(true, deckDoubleItem.defaultApplied(0));
    BOOST_CHECK_THROW(deckDoubleItem.get< double >(0), std::exception);
}

BOOST_AUTO_TEST_CASE(PushBackMultipleDouble) {
    auto dims = make_dims();
    DeckItem item( "HEI", double() , dims.first, dims.second);
    item.push_back(10.22 , 100 );
    BOOST_CHECK_EQUAL( 100U , item.data_size() );
    for (size_t i=0; i < 100; i++)
        BOOST_CHECK_EQUAL(10.22 , item.get< double >(i));
}


BOOST_AUTO_TEST_CASE(GetSIWithoutDimensionThrows) {
    DeckItem item( "HEI", double() , {},{});
    item.push_back(10.22 , 100 );

    BOOST_CHECK_THROW( item.getSIDouble(0) , std::exception );
    BOOST_CHECK_THROW( item.getSIDoubleData() , std::exception );
}

BOOST_AUTO_TEST_CASE(GetSISingleDimensionCorrect) {
    Dimension dim{ 100 };
    DeckItem item( "HEI", double(), { dim }, { dim } );

    item.push_back(1.0 , 100 );

    BOOST_CHECK_EQUAL( 1.0   , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 100 , item.getSIDouble(0) );
}

BOOST_AUTO_TEST_CASE(GetSISingleDefault) {
    Dimension dim{ 1 };
    Dimension defaultDim{  100 };
    DeckItem item( "HEI", double() , {dim}, {defaultDim});

    item.push_backDefault( 1.0 );
    BOOST_CHECK_EQUAL( 1   , item.get< double >(0) );
    BOOST_CHECK_EQUAL( 100 , item.getSIDouble(0) );
}

BOOST_AUTO_TEST_CASE(GetSIMultipleDim) {
    Dimension dim1{  2 };
    Dimension dim2{  4 };
    Dimension dim3{  8 };
    Dimension dim4{ 16 };
    Dimension defaultDim{  100 };
    DeckItem item( "HEI", double(), {dim1, dim2, dim3, dim4}, {defaultDim, defaultDim, defaultDim, defaultDim} );

    item.push_back( 1.0, 16 );

    for (size_t i=0; i < 16; i+= 4) {
        BOOST_CHECK_EQUAL( 2   , item.getSIDouble(i) );
        BOOST_CHECK_EQUAL( 4   , item.getSIDouble(i+ 1) );
        BOOST_CHECK_EQUAL( 8   , item.getSIDouble(i+2) );
        BOOST_CHECK_EQUAL(16   , item.getSIDouble(i+3) );
    }
}

BOOST_AUTO_TEST_CASE(HasValue) {
    DeckItem deckIntItem( "TEST", int() );
    BOOST_CHECK_EQUAL( false , deckIntItem.hasValue(0) );
    deckIntItem.push_back(1);
    BOOST_CHECK_EQUAL( true  , deckIntItem.hasValue(0) );
    BOOST_CHECK_EQUAL( false , deckIntItem.hasValue(1) );
}

BOOST_AUTO_TEST_CASE(DummyDefaultsInt) {
    DeckItem deckIntItem( "TEST", int() );
    BOOST_CHECK_EQUAL(deckIntItem.data_size(), 0U);

    deckIntItem.push_backDummyDefault<int>();
    BOOST_CHECK_EQUAL(deckIntItem.data_size(), 1U);
    BOOST_CHECK_EQUAL(true, deckIntItem.defaultApplied(0));
    BOOST_CHECK_EQUAL( false , deckIntItem.hasValue(0));
    BOOST_CHECK_EQUAL( false , deckIntItem.hasValue(1));
    BOOST_CHECK_THROW(deckIntItem.get< int >(0), std::exception);
}

BOOST_AUTO_TEST_CASE(GetIntAtIndex_NoData_ExceptionThrown) {
    DeckItem deckIntItem( "TEST", int() );
    deckIntItem.push_back(100);
    BOOST_CHECK(deckIntItem.get< int >(0) == 100);
    BOOST_CHECK_THROW(deckIntItem.get< int >(1), std::exception);
}

BOOST_AUTO_TEST_CASE(InitializeDefaultApplied) {
    DeckItem deckIntItem( "TEST", int() );
    BOOST_CHECK( deckIntItem.data_size() == 0 );
}

BOOST_AUTO_TEST_CASE(size_correct) {
    DeckItem deckIntItem( "TEST", int() );

    BOOST_CHECK_EQUAL( 0U , deckIntItem.data_size());
    deckIntItem.push_back( 100 );
    BOOST_CHECK_EQUAL( 1U , deckIntItem.data_size());

    deckIntItem.push_back( 100 );
    deckIntItem.push_back( 100 );
    BOOST_CHECK_EQUAL( 3U , deckIntItem.data_size());
}

BOOST_AUTO_TEST_CASE(DefaultNotAppliedInt) {
    DeckItem deckIntItem( "TEST", int() );
    BOOST_CHECK( deckIntItem.data_size() == 0 );

    deckIntItem.push_back( 100 );
    BOOST_CHECK( deckIntItem.data_size() == 1 );
    BOOST_CHECK( deckIntItem.get< int >(0) == 100 );
    BOOST_CHECK( !deckIntItem.defaultApplied(0) );

    BOOST_CHECK_THROW( deckIntItem.defaultApplied(1), std::exception );
    BOOST_CHECK_THROW( deckIntItem.get< int >(1), std::exception );
}

BOOST_AUTO_TEST_CASE(UseDefault) {
    DeckItem deckIntItem( "TEST", int() );

    deckIntItem.push_backDefault( 100 );

    BOOST_CHECK( deckIntItem.defaultApplied(0) );
    BOOST_CHECK( deckIntItem.get< int >(0) == 100 );

    BOOST_CHECK_THROW( deckIntItem.defaultApplied(1), std::exception );
    BOOST_CHECK_THROW( deckIntItem.get< int >(1), std::exception );
}

BOOST_AUTO_TEST_CASE(DefaultAppliedInt) {
    DeckItem deckIntItem( "TEST", int() );
    BOOST_CHECK( deckIntItem.data_size() == 0 );

    deckIntItem.push_backDefault( 100 );
    BOOST_CHECK( deckIntItem.data_size() == 1 );
    BOOST_CHECK( deckIntItem.get< int >(0) == 100 );
    BOOST_CHECK( deckIntItem.defaultApplied(0) );
    deckIntItem.push_back( 10 );
    BOOST_CHECK_EQUAL( false, deckIntItem.defaultApplied(1) );
    deckIntItem.push_backDefault( 1 );
    BOOST_CHECK_EQUAL( true , deckIntItem.defaultApplied(2) );
    BOOST_CHECK_EQUAL( 3U, deckIntItem.data_size() );
}


BOOST_AUTO_TEST_CASE(PushBackMultipleInt) {
    DeckItem item( "HEI", int() );
    item.push_back(10 , 100U );
    BOOST_CHECK_EQUAL( 100U , item.data_size() );
    for (size_t i=0; i < 100; i++)
        BOOST_CHECK_EQUAL(10 , item.get< int >(i));
}

BOOST_AUTO_TEST_CASE(size_defaultConstructor_sizezero) {
    DeckRecord deckRecord;
    BOOST_CHECK_EQUAL(0U, deckRecord.size());
}

BOOST_AUTO_TEST_CASE(addItem_singleItem_sizeone) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    BOOST_CHECK_EQUAL(1U, deckRecord.size());
}

BOOST_AUTO_TEST_CASE(addItem_multipleItems_sizecorrect) {

    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    deckRecord.addItem( DeckItem { "TEST2", int() } );
    deckRecord.addItem( DeckItem { "TEST3", int() } );

    BOOST_CHECK_EQUAL(3U, deckRecord.size());
}

BOOST_AUTO_TEST_CASE(addItem_differentItemsSameName_throws) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    BOOST_CHECK_THROW( deckRecord.addItem( DeckItem { "TEST", int() } ), std::exception );
    std::vector< DeckItem > items = { DeckItem { "TEST", int() }, DeckItem { "TEST" , int() } };
    BOOST_CHECK_THROW( DeckRecord( std::move( items ) ), std::exception );
}

BOOST_AUTO_TEST_CASE(get_byIndex_returnsItem) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    BOOST_CHECK_NO_THROW(deckRecord.getItem(0U));
}

BOOST_AUTO_TEST_CASE(get_indexoutofbounds_throws) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    BOOST_CHECK_THROW(deckRecord.getItem(1), std::exception);
}

BOOST_AUTO_TEST_CASE(get_byName_returnsItem) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    deckRecord.getItem("TEST");
}

BOOST_AUTO_TEST_CASE(get_byNameNonExisting_throws) {
    DeckRecord deckRecord;
    deckRecord.addItem( DeckItem { "TEST", int() } );
    BOOST_CHECK_THROW(deckRecord.getItem("INVALID"), std::exception);
}

BOOST_AUTO_TEST_CASE(StringsWithSpaceOK) {
    ParserItem itemString("STRINGITEM1", ParserItem::itype::STRING);
    ParserRecord record1;
    RawRecord rawRecord( " ' VALUE ' ", KeywordLocation("KW", "file", 100) );
    ParseContext parseContext;
    ErrorGuard errors;
    UnitSystem active_unitsystem(UnitSystem::UnitType::UNIT_TYPE_LAB);
    KeywordLocation loc;
    record1.addItem( itemString );


    const auto deckRecord = record1.parse( parseContext, errors , rawRecord, active_unitsystem, active_unitsystem, loc);
    BOOST_CHECK_EQUAL(" VALUE " , deckRecord.getItem(0).get< std::string >(0));
}

BOOST_AUTO_TEST_CASE(DataKeyword) {
    Parser parser;
    DeckKeyword kw(parser.getKeyword("GRID"));
    BOOST_CHECK_EQUAL( false , kw.isDataKeyword());
    kw.setDataKeyword( );
    BOOST_CHECK_EQUAL( true , kw.isDataKeyword());
    kw.setDataKeyword( false );
    BOOST_CHECK_EQUAL( false , kw.isDataKeyword());
    kw.setDataKeyword( true );
    BOOST_CHECK_EQUAL( true , kw.isDataKeyword());
}



BOOST_AUTO_TEST_CASE(name_nameSetInConstructor_nameReturned) {
    Parser parser;
    DeckKeyword deckKeyword( parser.getKeyword("GRID"));
    BOOST_CHECK_EQUAL("GRID", deckKeyword.name());
}

BOOST_AUTO_TEST_CASE(size_noRecords_returnszero) {
    Parser parser;
    DeckKeyword deckKeyword( parser.getKeyword("GRID"));;
    BOOST_CHECK_EQUAL(0U, deckKeyword.size());
    BOOST_CHECK_THROW(deckKeyword.getRecord(0), std::exception);
}




BOOST_AUTO_TEST_CASE(DeckItemWrite) {
    DeckItem item("TEST", int());
    std::stringstream s;
    DeckOutput w(s);

    item.push_back(1);
    item.push_back(2);
    item.push_back(3);

    item.write(w);
    {
        int v1,v2,v3;
        s >> v1;
        s >> v2;
        s >> v3;

        BOOST_CHECK_EQUAL( v1 , 1 );
        BOOST_CHECK_EQUAL( v2 , 2 );
        BOOST_CHECK_EQUAL( v3 , 3 );
    }
}

BOOST_AUTO_TEST_CASE(DeckOutputTest) {
    std::string expected = "KEYWORD\n\
==1-2\n\
==3-1*\n\
==5-1*\n\
==7-8\n\
==1*-10 /\n\
/\n\
ABC";
    std::stringstream s;
    DeckOutput out(s);

    out.fmt.record_indent = "==";
    out.fmt.item_sep = "-";
    out.fmt.columns = 2;
    out.fmt.keyword_sep = "ABC";

    out.start_keyword("KEYWORD", true);
    out.start_record();
    out.write<int>(1);
    out.write<int>(2);
    out.write<int>(3);
    out.stash_default( );
    out.write<int>(5);
    out.stash_default( );
    out.write<int>(7);
    out.write<int>(8);
    out.stash_default( );
    out.write<int>(10);
    out.end_record();
    out.end_keyword(true);
    out.write_string( out.fmt.keyword_sep );

    BOOST_CHECK_EQUAL( expected, s.str());
}

BOOST_AUTO_TEST_CASE(DeckItemWriteDefault) {
    DeckItem item("TEST", int());
    item.push_backDefault(1);
    item.push_backDefault(1);
    item.push_backDefault(1);

    {
        std::stringstream s;
        DeckOutput w(s);
        item.write( w );
        BOOST_CHECK_EQUAL( s.str() , "");
    }

    item.push_back(13);
    {
        std::stringstream s;
        DeckOutput w(s);
        item.write( w );
        BOOST_CHECK_EQUAL( s.str() , "3* 13");
    }
}


BOOST_AUTO_TEST_CASE(DeckItemWriteString) {
    DeckItem item("TEST", std::string());
    item.push_back("NO");
    item.push_back("YES");
    std::stringstream s;
    DeckOutput w(s);
    item.write( w );
    BOOST_CHECK_EQUAL( s.str() , "'NO' 'YES'");
}


BOOST_AUTO_TEST_CASE(RecordWrite) {
    auto dims = make_dims();
    DeckRecord deckRecord;
    DeckItem item1("TEST1", int());
    DeckItem item2("TEST2", double(), dims.first, dims.second);
    DeckItem item3("TEST3", std::string());

    item1.push_back( 123 );
    item2.push_backDefault( 100.0 );
    item3.push_back("VALUE");

    deckRecord.addItem( item1 );
    deckRecord.addItem( item2 );
    deckRecord.addItem( item3 );

    std::stringstream s;
    DeckOutput w(s);
    deckRecord.write_data( w );
    BOOST_CHECK_EQUAL( s.str() , "123 1* 'VALUE'");
}


BOOST_AUTO_TEST_CASE(DeckItemEqual) {
    auto dims = make_dims();
    DeckItem item1("TEST1" , int());
    DeckItem item2("TEST2" , int());
    DeckItem item3("TEST1" , double(), dims.first, dims.second);
    DeckItem item4("TEST1" , int());
    DeckItem item5("TEST1" , double(), dims.first, dims.second);

    BOOST_CHECK( item1 != item2 );
    BOOST_CHECK( item1 != item3 );
    BOOST_CHECK( item1 == item1 );
    BOOST_CHECK( item1 == item4 );

    item4.push_back(100);
    BOOST_CHECK( item1 != item4 );
    item1.push_back(100);
    BOOST_CHECK( item1 == item4 );

    item4.push_backDefault( 200 );
    item1.push_back( 200 );
    BOOST_CHECK( item1 == item4 );
    BOOST_CHECK( !item1.equal( item4 , true , true));

    item3.push_back(1.0);
    item5.push_back(1.0);
    BOOST_CHECK( item3.equal( item5 , false, true ));
    BOOST_CHECK( item3.equal( item5 , false, false ));

    item3.push_back(1.0);
    item5.push_back(1.0 - 1e-8);
    BOOST_CHECK( item3.equal( item5 , false, true ));
    BOOST_CHECK( !item3.equal( item5 , false, false ));
}


BOOST_AUTO_TEST_CASE(STRING_TO_BOOL) {
    BOOST_CHECK( DeckItem::to_bool("TRUE") );
    BOOST_CHECK( DeckItem::to_bool("T") );
    BOOST_CHECK( DeckItem::to_bool("YES") );
    BOOST_CHECK( DeckItem::to_bool("yEs") );
    BOOST_CHECK( DeckItem::to_bool("Y") );
    BOOST_CHECK( DeckItem::to_bool("1") );
    //
    BOOST_CHECK( !DeckItem::to_bool("falsE") );
    BOOST_CHECK( !DeckItem::to_bool("f") );
    BOOST_CHECK( !DeckItem::to_bool("NO") );
    BOOST_CHECK( !DeckItem::to_bool("N"));
    BOOST_CHECK( !DeckItem::to_bool("0") );
    //
    BOOST_CHECK_THROW(DeckItem::to_bool("NO - not valid"), std::exception);
    BOOST_CHECK_THROW(DeckItem::to_bool("YE"), std::exception);
    BOOST_CHECK_THROW(DeckItem::to_bool("YE"), std::exception);
}


BOOST_AUTO_TEST_CASE(DeckView2Test) {
    std::string deck_string = R"(
START
7 OCT 2020 /

DIMENS
  10 10 3 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PORO
  300*0.3 /

SCHEDULE

VFPPROD
-- table_num, datum_depth, flo, wfr, gfr, pressure, alq, unit, table_vals
42 7.0E+03 LIQ WCT GOR THP ' ' METRIC BHP /
1.0 / flo axis
0.0 1.0 / THP axis
0.0 / WFR axis
0.0 / GFR axis
0.0 / ALQ axis
-- Table itself: thp_idx wfr_idx gfr_idx alq_idx <vals>
1 1 1 1 0.0 /
2 1 1 1 1.0 /

VFPPROD
-- table_num, datum_depth, flo, wfr, gfr, pressure, alq, unit, table_vals
43 7.0E+03 LIQ WCT GOR THP 'GRAT' METRIC BHP /
1.0 / flo axis
0.0 1.0 / THP axis
0.0 / WFR axis
0.0 / GFR axis
0.0 / ALQ axis
-- Table itself: thp_idx wfr_idx gfr_idx alq_idx <vals>
1 1 1 1 0.0 /
2 1 1 1 1.0 /

WELSPECS -- 0
  'P1' 'G' 10 10 2005 'LIQ' /
  'P2' 'G' 10 10 2005 'LIQ' /
/

COMPDAT
  'P1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
  'P2'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 /
/

WCONPROD
  'P1' 'OPEN' 'ORAT'  123.4  0.0  0.0  0.0  0.0 100 100 42 'UDA' /
  'P2' 'OPEN' 'ORAT'  123.4  0.0  0.0  0.0  0.0 100 100 43 'UDA' /
/

DATES
   10 'JAN' 2000 /
/

DATES
   20 'JAN' 2000 /
/

DATES
   30 'JAN' 2000 /
/

)";

    Parser parser;
    auto deck = parser.parseString(deck_string);
    DeckView dw;
    for (const auto& kw : deck)
        dw.add_keyword(kw);

    BOOST_CHECK(dw.has_keyword("DATES"));
    BOOST_CHECK(!dw.has_keyword("RADIAL"));
    BOOST_CHECK(!dw.empty());
    BOOST_CHECK_EQUAL(dw.size(), 17);

    BOOST_CHECK_THROW(dw[100], std::exception);
    const auto& start = dw[0];
    BOOST_CHECK_EQUAL(start.name(), "START");
    BOOST_CHECK_EQUAL(dw.front().name(), "START");
    BOOST_CHECK(dw.back() == dw[16]);
    {
        DeckView dw2;
        BOOST_CHECK_THROW(dw2.back(), std::exception);
        BOOST_CHECK_THROW(dw2.front(), std::exception);
    }
    const auto& radial_kw = dw["RADIAL"];
    BOOST_CHECK(radial_kw.empty());


    const auto& dates_view = dw["DATES"];
    BOOST_CHECK_EQUAL(dates_view.size(), 3);
    const auto& first_dates = dates_view.front();
    const auto& rec0 = first_dates[0];
    const auto& month = rec0.getItem(1).get<std::string>(0);
    BOOST_CHECK_EQUAL(month, "JAN");

    const auto& last_dates = dates_view.back();
    const auto& rec1 = last_dates[0];
    const auto& day = rec1.getItem(0).get<int>(0);
    BOOST_CHECK_EQUAL(day, 30);


    BOOST_CHECK(dw.has_keyword<ParserKeywords::DATES>());
    std::vector<std::string> expected = {"START", "DIMENS", "GRID", "DXV", "DYV", "DZV", "DEPTHZ", "PORO",
                                         "SCHEDULE", "VFPPROD", "VFPPROD", "WELSPECS", "COMPDAT", "WCONPROD",
                                         "DATES", "DATES", "DATES"};
    std::vector<std::string> actual;
    for (const auto& kw : dw)
        actual.push_back(kw.name());

    BOOST_CHECK( actual == expected );

    const auto& radial_index = dw.index("RADIAL");
    BOOST_CHECK( radial_index.empty() );

    auto dates_index = dw.index("DATES");
    std::vector<std::size_t> dates_expected = {14, 15, 16};
    BOOST_CHECK(dates_index == dates_expected);


    BOOST_CHECK_EQUAL(dw.count("RADIAL"), 0);
    BOOST_CHECK_EQUAL(dw.count("VFPPROD"), 2);
    BOOST_CHECK_EQUAL(dw.count("SCHEDULE"), 1);


    bool have_grid0 = std::any_of(dw.begin(), dw.end(), [](const DeckKeyword& kw) {
        return kw.name() == "GRID";
    });
    BOOST_CHECK( have_grid0 );

    bool have_grid1 = std::any_of(dw.begin() + 3, dw.end(), [](const DeckKeyword& kw) {
        return kw.name() == "GRID";
    });
    BOOST_CHECK( !have_grid1 );

    auto is_vfpprod = [](const DeckKeyword& kw) { return kw.name() == "VFPPROD"; };
    auto iter = std::find_if(dw.begin(), dw.end(), is_vfpprod);
    BOOST_CHECK( iter != dw.end() );

    auto iter2 = std::find_if(iter + 1, dw.end(), is_vfpprod);
    BOOST_CHECK( iter2 != dw.end() );

    auto iter3 = std::find_if(iter2 + 1, dw.end(), is_vfpprod);
    BOOST_CHECK( iter3 == dw.end() );

    auto count = std::count_if(dw.begin(), dw.end(), is_vfpprod);
    BOOST_CHECK_EQUAL(count, 2);
}
