
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>
namespace Opm {
namespace ParserKeywords {
DATE::DATE() : ParserKeyword("DATE", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("DATE");
}
const std::string DATE::keywordName = "DATE";


DATES::DATES() : ParserKeyword("DATES", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DATES");
  {
     ParserRecord record;
     {
        ParserItem item("DAY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MONTH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("YEAR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TIME", ParserItem::itype::STRING);
        item.setDefault( std::string("00:00:00.000") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DATES::keywordName = "DATES";
const std::string DATES::DAY::itemName = "DAY";
const std::string DATES::MONTH::itemName = "MONTH";
const std::string DATES::YEAR::itemName = "YEAR";
const std::string DATES::TIME::itemName = "TIME";
const std::string DATES::TIME::defaultValue = "00:00:00.000";


DATUM::DATUM() : ParserKeyword("DATUM", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("DATUM");
  {
     ParserRecord record;
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DATUM::keywordName = "DATUM";
const std::string DATUM::DEPTH::itemName = "DEPTH";


DATUMR::DATUMR() : ParserKeyword("DATUMR", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("DATUMR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DATUMR::keywordName = "DATUMR";
const std::string DATUMR::data::itemName = "data";


DATUMRX::DATUMRX() : ParserKeyword("DATUMRX", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("DATUMRX");
  {
     ParserRecord record;
     {
        ParserItem item("REGION_FAMILY", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DATUMRX::keywordName = "DATUMRX";
const std::string DATUMRX::REGION_FAMILY::itemName = "REGION_FAMILY";
const std::string DATUMRX::REGION_FAMILY::defaultValue = "";
const std::string DATUMRX::DEPTH::itemName = "DEPTH";


DCQDEFN::DCQDEFN() : ParserKeyword("DCQDEFN", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DCQDEFN");
  {
     ParserRecord record;
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        item.setDefault( std::string("GAS") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DCQDEFN::keywordName = "DCQDEFN";
const std::string DCQDEFN::QUANTITY::itemName = "QUANTITY";
const std::string DCQDEFN::QUANTITY::defaultValue = "GAS";


DEBUG_::DEBUG_() : ParserKeyword("DEBUG_", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DEBUG");
  {
     ParserRecord record;
     {
        ParserItem item("Item1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item3", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item4", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item5", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item6", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item7", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item8", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item9", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item10", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item11", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item12", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item13", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item14", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item15", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item16", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item17", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item18", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item19", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item20", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item21", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item22", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item23", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item24", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item25", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item26", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item27", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item28", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item29", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item30", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item31", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item32", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item33", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item34", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item35", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item36", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item37", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item38", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item39", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item40", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item41", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item42", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item43", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item44", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item45", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item46", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item47", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item48", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item49", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item50", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item51", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item52", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item53", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item54", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item55", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item56", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item57", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item58", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item59", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item60", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item61", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item62", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item63", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item64", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item65", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item66", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item67", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item68", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item69", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item70", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item71", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item72", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item73", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item74", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item75", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item76", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item77", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item78", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item79", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item80", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item81", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item82", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item83", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item84", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item85", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item86", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("Item87", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DEBUG_::keywordName = "DEBUG_";
const std::string DEBUG_::Item1::itemName = "Item1";
const std::string DEBUG_::Item2::itemName = "Item2";
const std::string DEBUG_::Item3::itemName = "Item3";
const std::string DEBUG_::Item4::itemName = "Item4";
const std::string DEBUG_::Item5::itemName = "Item5";
const std::string DEBUG_::Item6::itemName = "Item6";
const std::string DEBUG_::Item7::itemName = "Item7";
const std::string DEBUG_::Item8::itemName = "Item8";
const std::string DEBUG_::Item9::itemName = "Item9";
const std::string DEBUG_::Item10::itemName = "Item10";
const std::string DEBUG_::Item11::itemName = "Item11";
const std::string DEBUG_::Item12::itemName = "Item12";
const std::string DEBUG_::Item13::itemName = "Item13";
const std::string DEBUG_::Item14::itemName = "Item14";
const std::string DEBUG_::Item15::itemName = "Item15";
const std::string DEBUG_::Item16::itemName = "Item16";
const std::string DEBUG_::Item17::itemName = "Item17";
const std::string DEBUG_::Item18::itemName = "Item18";
const std::string DEBUG_::Item19::itemName = "Item19";
const std::string DEBUG_::Item20::itemName = "Item20";
const std::string DEBUG_::Item21::itemName = "Item21";
const std::string DEBUG_::Item22::itemName = "Item22";
const std::string DEBUG_::Item23::itemName = "Item23";
const std::string DEBUG_::Item24::itemName = "Item24";
const std::string DEBUG_::Item25::itemName = "Item25";
const std::string DEBUG_::Item26::itemName = "Item26";
const std::string DEBUG_::Item27::itemName = "Item27";
const std::string DEBUG_::Item28::itemName = "Item28";
const std::string DEBUG_::Item29::itemName = "Item29";
const std::string DEBUG_::Item30::itemName = "Item30";
const std::string DEBUG_::Item31::itemName = "Item31";
const std::string DEBUG_::Item32::itemName = "Item32";
const std::string DEBUG_::Item33::itemName = "Item33";
const std::string DEBUG_::Item34::itemName = "Item34";
const std::string DEBUG_::Item35::itemName = "Item35";
const std::string DEBUG_::Item36::itemName = "Item36";
const std::string DEBUG_::Item37::itemName = "Item37";
const std::string DEBUG_::Item38::itemName = "Item38";
const std::string DEBUG_::Item39::itemName = "Item39";
const std::string DEBUG_::Item40::itemName = "Item40";
const std::string DEBUG_::Item41::itemName = "Item41";
const std::string DEBUG_::Item42::itemName = "Item42";
const std::string DEBUG_::Item43::itemName = "Item43";
const std::string DEBUG_::Item44::itemName = "Item44";
const std::string DEBUG_::Item45::itemName = "Item45";
const std::string DEBUG_::Item46::itemName = "Item46";
const std::string DEBUG_::Item47::itemName = "Item47";
const std::string DEBUG_::Item48::itemName = "Item48";
const std::string DEBUG_::Item49::itemName = "Item49";
const std::string DEBUG_::Item50::itemName = "Item50";
const std::string DEBUG_::Item51::itemName = "Item51";
const std::string DEBUG_::Item52::itemName = "Item52";
const std::string DEBUG_::Item53::itemName = "Item53";
const std::string DEBUG_::Item54::itemName = "Item54";
const std::string DEBUG_::Item55::itemName = "Item55";
const std::string DEBUG_::Item56::itemName = "Item56";
const std::string DEBUG_::Item57::itemName = "Item57";
const std::string DEBUG_::Item58::itemName = "Item58";
const std::string DEBUG_::Item59::itemName = "Item59";
const std::string DEBUG_::Item60::itemName = "Item60";
const std::string DEBUG_::Item61::itemName = "Item61";
const std::string DEBUG_::Item62::itemName = "Item62";
const std::string DEBUG_::Item63::itemName = "Item63";
const std::string DEBUG_::Item64::itemName = "Item64";
const std::string DEBUG_::Item65::itemName = "Item65";
const std::string DEBUG_::Item66::itemName = "Item66";
const std::string DEBUG_::Item67::itemName = "Item67";
const std::string DEBUG_::Item68::itemName = "Item68";
const std::string DEBUG_::Item69::itemName = "Item69";
const std::string DEBUG_::Item70::itemName = "Item70";
const std::string DEBUG_::Item71::itemName = "Item71";
const std::string DEBUG_::Item72::itemName = "Item72";
const std::string DEBUG_::Item73::itemName = "Item73";
const std::string DEBUG_::Item74::itemName = "Item74";
const std::string DEBUG_::Item75::itemName = "Item75";
const std::string DEBUG_::Item76::itemName = "Item76";
const std::string DEBUG_::Item77::itemName = "Item77";
const std::string DEBUG_::Item78::itemName = "Item78";
const std::string DEBUG_::Item79::itemName = "Item79";
const std::string DEBUG_::Item80::itemName = "Item80";
const std::string DEBUG_::Item81::itemName = "Item81";
const std::string DEBUG_::Item82::itemName = "Item82";
const std::string DEBUG_::Item83::itemName = "Item83";
const std::string DEBUG_::Item84::itemName = "Item84";
const std::string DEBUG_::Item85::itemName = "Item85";
const std::string DEBUG_::Item86::itemName = "Item86";
const std::string DEBUG_::Item87::itemName = "Item87";


DELAYACT::DELAYACT() : ParserKeyword("DELAYACT", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DELAYACT");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ACTION_TRIGGER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DELAY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("NUM_TIMES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DELAYACT::keywordName = "DELAYACT";
const std::string DELAYACT::ACTION_NAME::itemName = "ACTION_NAME";
const std::string DELAYACT::ACTION_TRIGGER::itemName = "ACTION_TRIGGER";
const std::string DELAYACT::DELAY::itemName = "DELAY";
const std::string DELAYACT::NUM_TIMES::itemName = "NUM_TIMES";
const std::string DELAYACT::INCREMENT::itemName = "INCREMENT";


DENAQA::DENAQA() : ParserKeyword("DENAQA", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DENAQA");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DENAQA::keywordName = "DENAQA";
const std::string DENAQA::DATA::itemName = "DATA";


DENSITY::DENSITY() : ParserKeyword("DENSITY", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DENSITY");
  {
     ParserRecord record;
     {
        ParserItem item("OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(600.000000) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("WATER", ParserItem::itype::DOUBLE);
        item.setDefault( double(999.014000) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DENSITY::keywordName = "DENSITY";
const std::string DENSITY::OIL::itemName = "OIL";
const std::string DENSITY::WATER::itemName = "WATER";
const std::string DENSITY::GAS::itemName = "GAS";


DEPTH::DEPTH() : ParserKeyword("DEPTH", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DEPTH");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DEPTH::keywordName = "DEPTH";
const std::string DEPTH::data::itemName = "data";


DEPTHTAB::DEPTHTAB() : ParserKeyword("DEPTHTAB", KeywordSize("RIVRDIMS", "NMDEPT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DEPTHTAB");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DEPTHTAB::keywordName = "DEPTHTAB";
const std::string DEPTHTAB::DATA::itemName = "DATA";


DEPTHZ::DEPTHZ() : ParserKeyword("DEPTHZ", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DEPTHZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DEPTHZ::keywordName = "DEPTHZ";
const std::string DEPTHZ::data::itemName = "data";


DIAGDISP::DIAGDISP() : ParserKeyword("DIAGDISP", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIAGDISP");
}
const std::string DIAGDISP::keywordName = "DIAGDISP";


DIFF::DIFF() : ParserKeyword("DIFF", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMR-");
  addDeckName("DIFFMZ-");
  addDeckName("DIFFMTH-");
  addDeckName("DIFFMX-");
  addDeckName("DIFFMY-");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFF::keywordName = "DIFF";
const std::string DIFF::data::itemName = "data";


DIFFAGAS::DIFFAGAS() : ParserKeyword("DIFFAGAS", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "DIFFCWAT",
    "DIFFCGAS",
  });
  setRequiredKeywords({
    "GAS",
    "WATER",
  });
  clearDeckNames();
  addDeckName("DIFFAGAS");
  {
     ParserRecord record;
     {
        ParserItem item("CO2_IN_GAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("H20_IN_GAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFAGAS::keywordName = "DIFFAGAS";
const std::string DIFFAGAS::CO2_IN_GAS::itemName = "CO2_IN_GAS";
const std::string DIFFAGAS::H20_IN_GAS::itemName = "H20_IN_GAS";


DIFFAWAT::DIFFAWAT() : ParserKeyword("DIFFAWAT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "DIFFCWAT",
    "DIFFCGAS",
  });
  setRequiredKeywords({
    "GAS",
    "WATER",
  });
  clearDeckNames();
  addDeckName("DIFFAWAT");
  {
     ParserRecord record;
     {
        ParserItem item("CO2_IN_WATER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("H20_IN_WATER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFAWAT::keywordName = "DIFFAWAT";
const std::string DIFFAWAT::CO2_IN_WATER::itemName = "CO2_IN_WATER";
const std::string DIFFAWAT::H20_IN_WATER::itemName = "H20_IN_WATER";


DIFFC::DIFFC() : ParserKeyword("DIFFC", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIFFC");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_MOL_WEIGHT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_MOL_WEIGHT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_GAS_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_GAS_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_OIL_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_OIL_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_OIL_CROSS_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_OIL_CROSS_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFC::keywordName = "DIFFC";
const std::string DIFFC::OIL_MOL_WEIGHT::itemName = "OIL_MOL_WEIGHT";
const std::string DIFFC::GAS_MOL_WEIGHT::itemName = "GAS_MOL_WEIGHT";
const std::string DIFFC::GAS_GAS_DIFF_COEFF::itemName = "GAS_GAS_DIFF_COEFF";
const std::string DIFFC::OIL_GAS_DIFF_COEFF::itemName = "OIL_GAS_DIFF_COEFF";
const std::string DIFFC::GAS_OIL_DIFF_COEFF::itemName = "GAS_OIL_DIFF_COEFF";
const std::string DIFFC::OIL_OIL_DIFF_COEFF::itemName = "OIL_OIL_DIFF_COEFF";
const std::string DIFFC::GAS_OIL_CROSS_DIFF_COEFF::itemName = "GAS_OIL_CROSS_DIFF_COEFF";
const std::string DIFFC::OIL_OIL_CROSS_DIFF_COEFF::itemName = "OIL_OIL_CROSS_DIFF_COEFF";


DIFFCGAS::DIFFCGAS() : ParserKeyword("DIFFCGAS", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "DIFFAWAT",
    "DIFFAGAS",
  });
  setRequiredKeywords({
    "GAS",
    "WATER",
  });
  clearDeckNames();
  addDeckName("DIFFCGAS");
  {
     ParserRecord record;
     {
        ParserItem item("CO2_IN_GAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("H20_IN_GAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFCGAS::keywordName = "DIFFCGAS";
const std::string DIFFCGAS::CO2_IN_GAS::itemName = "CO2_IN_GAS";
const std::string DIFFCGAS::H20_IN_GAS::itemName = "H20_IN_GAS";


DIFFCOAL::DIFFCOAL() : ParserKeyword("DIFFCOAL", KeywordSize("REGDIMS", "NTCREG", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIFFCOAL");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("RE_ADSORB_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SOL_DIFF_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFCOAL::keywordName = "DIFFCOAL";
const std::string DIFFCOAL::GAS_DIFF_COEFF::itemName = "GAS_DIFF_COEFF";
const std::string DIFFCOAL::RE_ADSORB_FRACTION::itemName = "RE_ADSORB_FRACTION";
const std::string DIFFCOAL::SOL_DIFF_COEFF::itemName = "SOL_DIFF_COEFF";


DIFFCWAT::DIFFCWAT() : ParserKeyword("DIFFCWAT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "DIFFAWAT",
    "DIFFAGAS",
  });
  setRequiredKeywords({
    "GAS",
    "WATER",
  });
  clearDeckNames();
  addDeckName("DIFFCWAT");
  {
     ParserRecord record;
     {
        ParserItem item("CO2_IN_WATER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("H20_IN_WATER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIFFCWAT::keywordName = "DIFFCWAT";
const std::string DIFFCWAT::CO2_IN_WATER::itemName = "CO2_IN_WATER";
const std::string DIFFCWAT::H20_IN_WATER::itemName = "H20_IN_WATER";


DIFFDP::DIFFDP() : ParserKeyword("DIFFDP", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIFFDP");
}
const std::string DIFFDP::keywordName = "DIFFDP";


DIFFMMF::DIFFMMF() : ParserKeyword("DIFFMMF", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DIFFMMF");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMMF::keywordName = "DIFFMMF";
const std::string DIFFMMF::data::itemName = "data";


DIFFMR::DIFFMR() : ParserKeyword("DIFFMR", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMR::keywordName = "DIFFMR";
const std::string DIFFMR::data::itemName = "data";


DIFFMTHT::DIFFMTHT() : ParserKeyword("DIFFMTHT", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMTHT");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMTHT::keywordName = "DIFFMTHT";
const std::string DIFFMTHT::data::itemName = "data";


DIFFMX::DIFFMX() : ParserKeyword("DIFFMX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMX::keywordName = "DIFFMX";
const std::string DIFFMX::data::itemName = "data";


DIFFMY::DIFFMY() : ParserKeyword("DIFFMY", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMY::keywordName = "DIFFMY";
const std::string DIFFMY::data::itemName = "data";


DIFFMZ::DIFFMZ() : ParserKeyword("DIFFMZ", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFFMZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFMZ::keywordName = "DIFFMZ";
const std::string DIFFMZ::data::itemName = "data";


DIFFR::DIFFR() : ParserKeyword("DIFFR", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DIFFR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFR::keywordName = "DIFFR";
const std::string DIFFR::data::itemName = "data";


DIFFTHT::DIFFTHT() : ParserKeyword("DIFFTHT", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DIFFTHT");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFTHT::keywordName = "DIFFTHT";
const std::string DIFFTHT::data::itemName = "data";


DIFFUSE::DIFFUSE() : ParserKeyword("DIFFUSE", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DIFFUSE");
}
const std::string DIFFUSE::keywordName = "DIFFUSE";


DIFFX::DIFFX() : ParserKeyword("DIFFX", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DIFFX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFX::keywordName = "DIFFX";
const std::string DIFFX::data::itemName = "data";


DIFFY::DIFFY() : ParserKeyword("DIFFY", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DIFFY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFY::keywordName = "DIFFY";
const std::string DIFFY::data::itemName = "data";


DIFFZ::DIFFZ() : ParserKeyword("DIFFZ", KeywordSize(1, false)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DIFFZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DIFFZ::keywordName = "DIFFZ";
const std::string DIFFZ::data::itemName = "data";


DIMENS::DIMENS() : ParserKeyword("DIMENS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DIMENS");
  {
     ParserRecord record;
     {
        ParserItem item("NX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIMENS::keywordName = "DIMENS";
const std::string DIMENS::NX::itemName = "NX";
const std::string DIMENS::NY::itemName = "NY";
const std::string DIMENS::NZ::itemName = "NZ";


DIMPES::DIMPES() : ParserKeyword("DIMPES", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DIMPES");
  {
     ParserRecord record;
     {
        ParserItem item("DSTARG", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.050000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DSMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DPMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(13.790000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIMPES::keywordName = "DIMPES";
const std::string DIMPES::DSTARG::itemName = "DSTARG";
const std::string DIMPES::DSMAX::itemName = "DSMAX";
const std::string DIMPES::DPMAX::itemName = "DPMAX";


DIMPLICT::DIMPLICT() : ParserKeyword("DIMPLICT", KeywordSize(0, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DIMPLICT");
}
const std::string DIMPLICT::keywordName = "DIMPLICT";


DISGAS::DISGAS() : ParserKeyword("DISGAS", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DISGAS");
}
const std::string DISGAS::keywordName = "DISGAS";


DISGASW::DISGASW() : ParserKeyword("DISGASW", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DISGASW");
}
const std::string DISGASW::keywordName = "DISGASW";


DISPDIMS::DISPDIMS() : ParserKeyword("DISPDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DISPDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_DISP_TABLES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_VELOCITY_NODES", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_CONCENTRATION_NODES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DISPDIMS::keywordName = "DISPDIMS";
const std::string DISPDIMS::NUM_DISP_TABLES::itemName = "NUM_DISP_TABLES";
const std::string DISPDIMS::MAX_VELOCITY_NODES::itemName = "MAX_VELOCITY_NODES";
const std::string DISPDIMS::MAX_CONCENTRATION_NODES::itemName = "MAX_CONCENTRATION_NODES";


DISPERC::DISPERC() : ParserKeyword("DISPERC", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DISPERC");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DISPERC::keywordName = "DISPERC";
const std::string DISPERC::data::itemName = "data";


DISPERSE::DISPERSE() : ParserKeyword("DISPERSE", KeywordSize("DISPDIMS", "MXDIST", true, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DISPERSE");
  {
     ParserRecord record;
     {
        ParserItem item("VELOCITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DISPERSE::keywordName = "DISPERSE";
const std::string DISPERSE::VELOCITY::itemName = "VELOCITY";
const std::string DISPERSE::DATA::itemName = "DATA";


DOMAINS::DOMAINS() : ParserKeyword("DOMAINS", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DOMAINS");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DOMAINS::keywordName = "DOMAINS";
const std::string DOMAINS::data::itemName = "data";


DPGRID::DPGRID() : ParserKeyword("DPGRID", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DPGRID");
}
const std::string DPGRID::keywordName = "DPGRID";


DPKRMOD::DPKRMOD() : ParserKeyword("DPKRMOD", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DPKRMOD");
  {
     ParserRecord record;
     {
        ParserItem item("MOD_OIL_WAT_PERM", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MOD_OIL_GAS_PERM", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_PERM_FRACTURE", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DPKRMOD::keywordName = "DPKRMOD";
const std::string DPKRMOD::MOD_OIL_WAT_PERM::itemName = "MOD_OIL_WAT_PERM";
const std::string DPKRMOD::MOD_OIL_GAS_PERM::itemName = "MOD_OIL_GAS_PERM";
const std::string DPKRMOD::SCALE_PERM_FRACTURE::itemName = "SCALE_PERM_FRACTURE";
const std::string DPKRMOD::SCALE_PERM_FRACTURE::defaultValue = "YES";


DPNUM::DPNUM() : ParserKeyword("DPNUM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DPNUM");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DPNUM::keywordName = "DPNUM";
const std::string DPNUM::VALUE::itemName = "VALUE";


DR::DR() : ParserKeyword("DR", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DR::keywordName = "DR";
const std::string DR::data::itemName = "data";


DREF::DREF() : ParserKeyword("DREF", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DREF");
  {
     ParserRecord record;
     {
        ParserItem item("DENSITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DREF::keywordName = "DREF";
const std::string DREF::DENSITY::itemName = "DENSITY";


DREFS::DREFS() : ParserKeyword("DREFS", KeywordSize("TABDIMS", "NUM_EOS_SURFACE", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DREFS");
  {
     ParserRecord record;
     {
        ParserItem item("DENSITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DREFS::keywordName = "DREFS";
const std::string DREFS::DENSITY::itemName = "DENSITY";


DRILPRI::DRILPRI() : ParserKeyword("DRILPRI", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRILPRI");
  {
     ParserRecord record;
     {
        ParserItem item("INTERVAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("B", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("C", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("D", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("E", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("F", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("G", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("H", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("LOOK_AHEAD", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("CALCULATION", ParserItem::itype::STRING);
        item.setDefault( std::string("SINGLE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRILPRI::keywordName = "DRILPRI";
const std::string DRILPRI::INTERVAL::itemName = "INTERVAL";
const std::string DRILPRI::A::itemName = "A";
const std::string DRILPRI::B::itemName = "B";
const std::string DRILPRI::C::itemName = "C";
const std::string DRILPRI::D::itemName = "D";
const std::string DRILPRI::E::itemName = "E";
const std::string DRILPRI::F::itemName = "F";
const std::string DRILPRI::G::itemName = "G";
const std::string DRILPRI::H::itemName = "H";
const std::string DRILPRI::LOOK_AHEAD::itemName = "LOOK_AHEAD";
const std::string DRILPRI::CALCULATION::itemName = "CALCULATION";
const std::string DRILPRI::CALCULATION::defaultValue = "SINGLE";


DRSDT::DRSDT() : ParserKeyword("DRSDT", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRSDT");
  {
     ParserRecord record;
     {
        ParserItem item("DRSDT_MAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDT::keywordName = "DRSDT";
const std::string DRSDT::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDT::OPTION::itemName = "OPTION";
const std::string DRSDT::OPTION::defaultValue = "ALL";


DRSDTCON::DRSDTCON() : ParserKeyword("DRSDTCON", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRSDTCON");
  {
     ParserRecord record;
     {
        ParserItem item("DRSDT_MAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.040000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PSI", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.340000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("OMEGA", ParserItem::itype::DOUBLE);
        item.setDefault( double(3e-09) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDTCON::keywordName = "DRSDTCON";
const std::string DRSDTCON::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDTCON::PSI::itemName = "PSI";
const std::string DRSDTCON::OMEGA::itemName = "OMEGA";
const std::string DRSDTCON::OPTION::itemName = "OPTION";
const std::string DRSDTCON::OPTION::defaultValue = "ALL";


DRSDTR::DRSDTR() : ParserKeyword("DRSDTR", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRSDTR");
  {
     ParserRecord record;
     {
        ParserItem item("DRSDT_MAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDTR::keywordName = "DRSDTR";
const std::string DRSDTR::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDTR::OPTION::itemName = "OPTION";
const std::string DRSDTR::OPTION::defaultValue = "ALL";


DRV::DRV() : ParserKeyword("DRV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DRV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DRV::keywordName = "DRV";
const std::string DRV::data::itemName = "data";


DRVDT::DRVDT() : ParserKeyword("DRVDT", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRVDT");
  {
     ParserRecord record;
     {
        ParserItem item("DRVDT_MAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("OilDissolutionFactor/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRVDT::keywordName = "DRVDT";
const std::string DRVDT::DRVDT_MAX::itemName = "DRVDT_MAX";


DRVDTR::DRVDTR() : ParserKeyword("DRVDTR", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRVDTR");
  {
     ParserRecord record;
     {
        ParserItem item("DRVDT_MAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("OilDissolutionFactor/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRVDTR::keywordName = "DRVDTR";
const std::string DRVDTR::DRVDT_MAX::itemName = "DRVDT_MAX";


DSPDEINT::DSPDEINT() : ParserKeyword("DSPDEINT", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DSPDEINT");
}
const std::string DSPDEINT::keywordName = "DSPDEINT";


DTHETA::DTHETA() : ParserKeyword("DTHETA", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DTHETA");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DTHETA::keywordName = "DTHETA";
const std::string DTHETA::data::itemName = "data";


DTHETAV::DTHETAV() : ParserKeyword("DTHETAV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DTHETAV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DTHETAV::keywordName = "DTHETAV";
const std::string DTHETAV::data::itemName = "data";


DUALPERM::DUALPERM() : ParserKeyword("DUALPERM", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DUALPERM");
}
const std::string DUALPERM::keywordName = "DUALPERM";


DUALPORO::DUALPORO() : ParserKeyword("DUALPORO", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DUALPORO");
}
const std::string DUALPORO::keywordName = "DUALPORO";


DUMPCUPL::DUMPCUPL() : ParserKeyword("DUMPCUPL", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DUMPCUPL");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DUMPCUPL::keywordName = "DUMPCUPL";
const std::string DUMPCUPL::VALUE::itemName = "VALUE";


DUMPFLUX::DUMPFLUX() : ParserKeyword("DUMPFLUX", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DUMPFLUX");
}
const std::string DUMPFLUX::keywordName = "DUMPFLUX";


DX::DX() : ParserKeyword("DX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  setProhibitedKeywords({
    "DXV",
  });
  clearDeckNames();
  addDeckName("DX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DX::keywordName = "DX";
const std::string DX::data::itemName = "data";


DXV::DXV() : ParserKeyword("DXV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  setProhibitedKeywords({
    "DX",
  });
  clearDeckNames();
  addDeckName("DXV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DXV::keywordName = "DXV";
const std::string DXV::data::itemName = "data";


DY::DY() : ParserKeyword("DY", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  setProhibitedKeywords({
    "DYV",
  });
  clearDeckNames();
  addDeckName("DY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DY::keywordName = "DY";
const std::string DY::data::itemName = "data";


DYNAMICR::DYNAMICR() : ParserKeyword("DYNAMICR", KeywordSize(1, true)) {
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DYNAMICR");
  setCodeEnd("ENDDYN");
  {
     ParserRecord record;
     {
        ParserItem item("code", ParserItem::itype::RAW_STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DYNAMICR::keywordName = "DYNAMICR";
const std::string DYNAMICR::code::itemName = "code";


DYNRDIMS::DYNRDIMS() : ParserKeyword("DYNRDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DYNRDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MNUMDR", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXDYNF", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXDYNR", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DYNRDIMS::keywordName = "DYNRDIMS";
const std::string DYNRDIMS::MNUMDR::itemName = "MNUMDR";
const std::string DYNRDIMS::MXDYNF::itemName = "MXDYNF";
const std::string DYNRDIMS::MXDYNR::itemName = "MXDYNR";


DYV::DYV() : ParserKeyword("DYV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  setProhibitedKeywords({
    "DY",
  });
  clearDeckNames();
  addDeckName("DYV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DYV::keywordName = "DYV";
const std::string DYV::data::itemName = "data";


DZ::DZ() : ParserKeyword("DZ", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZ::keywordName = "DZ";
const std::string DZ::data::itemName = "data";


DZMATRIX::DZMATRIX() : ParserKeyword("DZMATRIX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZMATRIX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZMATRIX::keywordName = "DZMATRIX";
const std::string DZMATRIX::data::itemName = "data";


DZMTRX::DZMTRX() : ParserKeyword("DZMTRX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZMTRX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZMTRX::keywordName = "DZMTRX";
const std::string DZMTRX::data::itemName = "data";


DZMTRXV::DZMTRXV() : ParserKeyword("DZMTRXV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZMTRXV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZMTRXV::keywordName = "DZMTRXV";
const std::string DZMTRXV::data::itemName = "data";


DZNET::DZNET() : ParserKeyword("DZNET", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZNET");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZNET::keywordName = "DZNET";
const std::string DZNET::data::itemName = "data";


DZV::DZV() : ParserKeyword("DZV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZV::keywordName = "DZV";
const std::string DZV::data::itemName = "data";


}
}
