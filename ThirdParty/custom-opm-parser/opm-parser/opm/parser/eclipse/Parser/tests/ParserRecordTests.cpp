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


#define BOOST_TEST_MODULE ParserTests
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ParserEnums.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/RawDeck/RawRecord.hpp>
#include <boost/test/test_tools.hpp>

#include "opm/parser/eclipse/RawDeck/RawKeyword.hpp"
#include "opm/parser/eclipse/Parser/ParserKeyword.hpp"

using namespace Opm;

BOOST_AUTO_TEST_CASE(DefaultConstructor_NoParams_NoThrow) {
    BOOST_CHECK_NO_THROW(ParserRecord record);
}

BOOST_AUTO_TEST_CASE(InitSharedPointer_NoThrow) {
    BOOST_CHECK_NO_THROW(ParserRecordConstPtr ptr(new ParserRecord()));
    BOOST_CHECK_NO_THROW(ParserRecordPtr ptr(new ParserRecord()));
}

BOOST_AUTO_TEST_CASE(Size_NoElements_ReturnsZero) {
    ParserRecord record;
    BOOST_CHECK_EQUAL(0U, record.size());
}

BOOST_AUTO_TEST_CASE(Size_OneItem_Return1) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt(new ParserIntItem("ITEM1", sizeType));
    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt);
    BOOST_CHECK_EQUAL(1U, record->size());
}

BOOST_AUTO_TEST_CASE(Get_OneItem_Return1) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt(new ParserIntItem("ITEM1", sizeType));
    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt);
    {
        ParserItemConstPtr item = record->get(0);
        BOOST_CHECK_EQUAL(item, itemInt);
    }
}

BOOST_AUTO_TEST_CASE(Get_outOfRange_Throw) {
    ParserRecordConstPtr record(new ParserRecord());
    BOOST_CHECK_THROW(record->get(0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(Get_KeyNotFound_Throw) {
    ParserRecordPtr record(new ParserRecord());
    BOOST_CHECK_THROW(record->get("Hei"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Get_KeyFound_OK) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt(new ParserIntItem("ITEM1", sizeType));
    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt);
    {
        ParserItemConstPtr item = record->get("ITEM1");
        BOOST_CHECK_EQUAL(item, itemInt);
    }
}

BOOST_AUTO_TEST_CASE(Get_GetByNameAndIndex_OK) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt(new ParserIntItem("ITEM1", sizeType));
    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt);
    {
        ParserItemConstPtr itemByName = record->get("ITEM1");
        ParserItemConstPtr itemByIndex = record->get(0);
        BOOST_CHECK_EQUAL(itemInt, itemByName);
        BOOST_CHECK_EQUAL(itemInt, itemByIndex);
    }
}

BOOST_AUTO_TEST_CASE(addItem_SameName_Throw) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt1(new ParserIntItem("ITEM1", sizeType));
    ParserIntItemPtr itemInt2(new ParserIntItem("ITEM1", sizeType));
    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt1);
    BOOST_CHECK_THROW(record->addItem(itemInt2), std::invalid_argument);
}

static ParserRecordPtr createSimpleParserRecord() {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt1(new ParserIntItem("ITEM1", sizeType));
    ParserIntItemPtr itemInt2(new ParserIntItem("ITEM2", sizeType));
    ParserRecordPtr record(new ParserRecord());

    record->addItem(itemInt1);
    record->addItem(itemInt2);

    return record;
}

BOOST_AUTO_TEST_CASE(parse_validRecord_noThrow) {
    ParserRecordPtr record = createSimpleParserRecord();
    ParseContext parseContext;
    RawRecord raw( string_view( "100 443" ) );
    MessageContainer msgContainer;
    BOOST_CHECK_NO_THROW(record->parse(parseContext, msgContainer, raw ) );
}

BOOST_AUTO_TEST_CASE(parse_validRecord_deckRecordCreated) {
    ParserRecordPtr record = createSimpleParserRecord();
    RawRecord rawRecord( string_view( "100 443" ) );
    ParseContext parseContext;
    MessageContainer msgContainer;
    const auto deckRecord = record->parse(parseContext , msgContainer, rawRecord);
    BOOST_CHECK_EQUAL(2U, deckRecord.size());
}


// INT INT DOUBLE DOUBLE INT DOUBLE

static ParserRecordPtr createMixedParserRecord() {

    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt1(new ParserIntItem("INTITEM1", sizeType));
    ParserIntItemPtr itemInt2(new ParserIntItem("INTITEM2", sizeType));
    ParserDoubleItemPtr itemDouble1(new ParserDoubleItem("DOUBLEITEM1", sizeType));
    ParserDoubleItemPtr itemDouble2(new ParserDoubleItem("DOUBLEITEM2", sizeType));

    ParserIntItemPtr itemInt3(new ParserIntItem("INTITEM3", sizeType));
    ParserDoubleItemPtr itemDouble3(new ParserDoubleItem("DOUBLEITEM3", sizeType));

    ParserRecordPtr record(new ParserRecord());
    record->addItem(itemInt1);
    record->addItem(itemInt2);
    record->addItem(itemDouble1);
    record->addItem(itemDouble2);
    record->addItem(itemInt3);
    record->addItem(itemDouble3);

    return record;
}

BOOST_AUTO_TEST_CASE(parse_validMixedRecord_noThrow) {
    ParserRecordPtr record = createMixedParserRecord();
    RawRecord rawRecord( string_view( "1 2 10.0 20.0 4 90.0") );
    ParseContext parseContext;
    MessageContainer msgContainer;
    BOOST_CHECK_NO_THROW(record->parse(parseContext , msgContainer, rawRecord));
}

BOOST_AUTO_TEST_CASE(Equal_Equal_ReturnsTrue) {
    ParserRecordPtr record1 = createMixedParserRecord();
    ParserRecordPtr record2 = createMixedParserRecord();

    BOOST_CHECK(record1->equal(*record1));
    BOOST_CHECK(record1->equal(*record2));
}

BOOST_AUTO_TEST_CASE(Equal_Different_ReturnsFalse) {
    ParserItemSizeEnum sizeType = SINGLE;
    ParserIntItemPtr itemInt(new ParserIntItem("INTITEM1", sizeType, 0));
    ParserDoubleItemPtr itemDouble(new ParserDoubleItem("DOUBLEITEM1", sizeType, 0));
    ParserStringItemPtr itemString(new ParserStringItem("STRINGITEM1", sizeType));
    ParserRecordPtr record1(new ParserRecord());
    ParserRecordPtr record2(new ParserRecord());
    ParserRecordPtr record3(new ParserRecord());

    record1->addItem(itemInt);
    record1->addItem(itemDouble);

    record2->addItem(itemInt);
    record2->addItem(itemDouble);
    record2->addItem(itemString);

    record3->addItem(itemDouble);
    record3->addItem(itemInt);
    BOOST_CHECK(!record1->equal(*record2));
    BOOST_CHECK(!record1->equal(*record3));

}

BOOST_AUTO_TEST_CASE(ParseWithDefault_defaultAppliedCorrectInDeck) {
    ParserRecord parserRecord;
    ParserIntItemConstPtr itemInt(new ParserIntItem("ITEM1", SINGLE , 100));
    ParserStringItemConstPtr itemString(new ParserStringItem("ITEM2", SINGLE , "DEFAULT"));
    ParserDoubleItemConstPtr itemDouble(new ParserDoubleItem("ITEM3", SINGLE , 3.14 ));

    parserRecord.addItem(itemInt);
    parserRecord.addItem(itemString);
    parserRecord.addItem(itemDouble);

    // according to the RM, this is invalid ("an asterisk by itself is not sufficient"),
    // but it seems to appear in the wild. Thus, we interpret this as "1*"...
    {
        RawRecord rawRecord( "* " );
        const auto deckStringItem = itemString->scan(rawRecord);
        const auto deckIntItem = itemInt->scan(rawRecord);
        const auto deckDoubleItem = itemDouble->scan(rawRecord);

        BOOST_CHECK(deckStringItem.size() == 1);
        BOOST_CHECK(deckIntItem.size() == 1);
        BOOST_CHECK(deckDoubleItem.size() == 1);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }

    {
        RawRecord rawRecord( "" );
        const auto deckStringItem = itemString->scan(rawRecord);
        const auto deckIntItem = itemInt->scan(rawRecord);
        const auto deckDoubleItem = itemDouble->scan(rawRecord);

        BOOST_CHECK(deckStringItem.size() == 1);
        BOOST_CHECK(deckIntItem.size() == 1);
        BOOST_CHECK(deckDoubleItem.size() == 1);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }


    {
        RawRecord rawRecord( "TRYGVE 10 2.9 " );

        // let the raw record be "consumed" by the items. Note that the scan() method
        // modifies the rawRecord object!
        const auto deckStringItem = itemString->scan(rawRecord);
        const auto deckIntItem = itemInt->scan(rawRecord);
        const auto deckDoubleItem = itemDouble->scan(rawRecord);

        BOOST_CHECK(deckStringItem.size() == 1);
        BOOST_CHECK(deckIntItem.size() == 1);
        BOOST_CHECK(deckDoubleItem.size() == 1);

        BOOST_CHECK(!deckStringItem.defaultApplied(0));
        BOOST_CHECK(!deckIntItem.defaultApplied(0));
        BOOST_CHECK(!deckDoubleItem.defaultApplied(0));
    }

    // again this is invalid according to the RM, but it is used anyway in the wild...
    {
        RawRecord rawRecord( "* * *" );
        const auto deckStringItem = itemString->scan(rawRecord);
        const auto deckIntItem = itemInt->scan(rawRecord);
        const auto deckDoubleItem = itemDouble->scan(rawRecord);

        BOOST_CHECK(deckStringItem.size() == 1);
        BOOST_CHECK(deckIntItem.size() == 1);
        BOOST_CHECK(deckDoubleItem.size() == 1);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }

    {
        RawRecord rawRecord(  "3*" );
        const auto deckStringItem = itemString->scan(rawRecord);
        const auto deckIntItem = itemInt->scan(rawRecord);
        const auto deckDoubleItem = itemDouble->scan(rawRecord);

        BOOST_CHECK(deckStringItem.size() == 1);
        BOOST_CHECK(deckIntItem.size() == 1);
        BOOST_CHECK(deckDoubleItem.size() == 1);

        BOOST_CHECK(deckStringItem.defaultApplied(0));
        BOOST_CHECK(deckIntItem.defaultApplied(0));
        BOOST_CHECK(deckDoubleItem.defaultApplied(0));
    }
}

BOOST_AUTO_TEST_CASE(Parse_RawRecordTooManyItems_Throws) {
    ParserRecordPtr parserRecord(new ParserRecord());
    ParserIntItemConstPtr itemI(new ParserIntItem("I", SINGLE));
    ParserIntItemConstPtr itemJ(new ParserIntItem("J", SINGLE));
    ParserIntItemConstPtr itemK(new ParserIntItem("K", SINGLE));
    ParseContext parseContext;

    parserRecord->addItem(itemI);
    parserRecord->addItem(itemJ);
    parserRecord->addItem(itemK);


    RawRecord rawRecord(  "3 3 3 " );
    MessageContainer msgContainer;

    BOOST_CHECK_NO_THROW(parserRecord->parse(parseContext , msgContainer, rawRecord));

    RawRecord rawRecordOneExtra(  "3 3 3 4 " );
    BOOST_CHECK_THROW(parserRecord->parse(parseContext , msgContainer, rawRecordOneExtra), std::invalid_argument);

    RawRecord rawRecordForgotRecordTerminator(  "3 3 3 \n 4 4 4 " );
    BOOST_CHECK_THROW(parserRecord->parse(parseContext , msgContainer, rawRecordForgotRecordTerminator), std::invalid_argument);

}


BOOST_AUTO_TEST_CASE(Parse_RawRecordTooFewItems) {
    ParserRecordPtr parserRecord(new ParserRecord());
    ParserIntItemConstPtr itemI(new ParserIntItem("I", SINGLE));
    ParserIntItemConstPtr itemJ(new ParserIntItem("J", SINGLE));
    ParserIntItemConstPtr itemK(new ParserIntItem("K", SINGLE));

    parserRecord->addItem(itemI);
    parserRecord->addItem(itemJ);
    parserRecord->addItem(itemK);

    ParseContext parseContext;
    RawRecord rawRecord(  "3 3  " );
    // no default specified for the third item, record can be parsed just fine but trying
    // to access the data will raise an exception...
    MessageContainer msgContainer;
    BOOST_CHECK_NO_THROW(parserRecord->parse(parseContext , msgContainer, rawRecord));
    auto record = parserRecord->parse(parseContext , msgContainer, rawRecord);
    BOOST_CHECK_NO_THROW(record.getItem(2));
    BOOST_CHECK_THROW(record.getItem(2).get< int >(0), std::out_of_range);
}



BOOST_AUTO_TEST_CASE(ParseRecordHasDimensionCorrect) {
    ParserRecordPtr parserRecord(new ParserRecord());
    ParserIntItemConstPtr itemI(new ParserIntItem("I", SINGLE));
    ParserDoubleItemPtr item2(new ParserDoubleItem("ID", SINGLE));

    BOOST_CHECK_EQUAL( false , parserRecord->hasDimension());

    parserRecord->addItem( itemI );
    parserRecord->addItem( item2 );
    BOOST_CHECK_EQUAL( false , parserRecord->hasDimension());

    item2->push_backDimension("Length*Length/Time");
    BOOST_CHECK_EQUAL( true , parserRecord->hasDimension());
}


BOOST_AUTO_TEST_CASE(DefaultNotDataRecord) {
    ParserRecord record;
    BOOST_CHECK_EQUAL( false , record.isDataRecord() );
}



BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws1) {
    ParserRecord record;
    ParserIntItemConstPtr dataItem = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL));
    ParserIntItemConstPtr item     = ParserIntItemConstPtr(new ParserIntItem( "XXX" , ALL));
    record.addDataItem( dataItem );
    BOOST_CHECK_THROW( record.addItem( item ) , std::invalid_argument);
    BOOST_CHECK_THROW( record.addItem( dataItem ) , std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(MixingDataAndItems_throws2) {
    ParserRecord record;
    ParserIntItemConstPtr dataItem = ParserIntItemConstPtr(new ParserIntItem( "ACTNUM" , ALL));
    ParserIntItemConstPtr item     = ParserIntItemConstPtr(new ParserIntItem( "XXX" , ALL));

    record.addItem( item );
    BOOST_CHECK_THROW( record.addDataItem( dataItem ) , std::invalid_argument);
}
