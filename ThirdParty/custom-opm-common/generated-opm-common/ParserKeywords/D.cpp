#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/D.hpp>
namespace Opm {
namespace ParserKeywords {
DATE::DATE( ) : ParserKeyword("DATE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("DATE");
}
const std::string DATE::keywordName = "DATE";


DATES::DATES( ) : ParserKeyword("DATES")
{
  setSizeType(SLASH_TERMINATED);
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


DATUM::DATUM( ) : ParserKeyword("DATUM")
{
  setFixedSize( (size_t) 1);
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


DATUMR::DATUMR( ) : ParserKeyword("DATUMR")
{
  setFixedSize( (size_t) 1);
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


DATUMRX::DATUMRX( ) : ParserKeyword("DATUMRX")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("DATUMRX");
  {
     ParserRecord record;
     {
        ParserItem item("REGION_FAMILY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DATUMRX::keywordName = "DATUMRX";
const std::string DATUMRX::REGION_FAMILY::itemName = "REGION_FAMILY";
const std::string DATUMRX::DEPTH::itemName = "DEPTH";


DCQDEFN::DCQDEFN( ) : ParserKeyword("DCQDEFN")
{
  setFixedSize( (size_t) 1);
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


DEBUG_::DEBUG_( ) : ParserKeyword("DEBUG_")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
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
const int DEBUG_::Item1::defaultValue = 0;
const std::string DEBUG_::Item2::itemName = "Item2";
const int DEBUG_::Item2::defaultValue = 0;
const std::string DEBUG_::Item3::itemName = "Item3";
const int DEBUG_::Item3::defaultValue = 0;
const std::string DEBUG_::Item4::itemName = "Item4";
const int DEBUG_::Item4::defaultValue = 0;
const std::string DEBUG_::Item5::itemName = "Item5";
const int DEBUG_::Item5::defaultValue = 0;
const std::string DEBUG_::Item6::itemName = "Item6";
const int DEBUG_::Item6::defaultValue = 0;
const std::string DEBUG_::Item7::itemName = "Item7";
const int DEBUG_::Item7::defaultValue = 0;
const std::string DEBUG_::Item8::itemName = "Item8";
const int DEBUG_::Item8::defaultValue = 0;
const std::string DEBUG_::Item9::itemName = "Item9";
const int DEBUG_::Item9::defaultValue = 0;
const std::string DEBUG_::Item10::itemName = "Item10";
const int DEBUG_::Item10::defaultValue = 0;
const std::string DEBUG_::Item11::itemName = "Item11";
const int DEBUG_::Item11::defaultValue = 0;
const std::string DEBUG_::Item12::itemName = "Item12";
const int DEBUG_::Item12::defaultValue = 0;
const std::string DEBUG_::Item13::itemName = "Item13";
const int DEBUG_::Item13::defaultValue = 0;
const std::string DEBUG_::Item14::itemName = "Item14";
const int DEBUG_::Item14::defaultValue = 0;
const std::string DEBUG_::Item15::itemName = "Item15";
const int DEBUG_::Item15::defaultValue = 0;
const std::string DEBUG_::Item16::itemName = "Item16";
const int DEBUG_::Item16::defaultValue = 0;
const std::string DEBUG_::Item17::itemName = "Item17";
const int DEBUG_::Item17::defaultValue = 0;
const std::string DEBUG_::Item18::itemName = "Item18";
const int DEBUG_::Item18::defaultValue = 0;
const std::string DEBUG_::Item19::itemName = "Item19";
const int DEBUG_::Item19::defaultValue = 0;
const std::string DEBUG_::Item20::itemName = "Item20";
const int DEBUG_::Item20::defaultValue = 0;
const std::string DEBUG_::Item21::itemName = "Item21";
const int DEBUG_::Item21::defaultValue = 0;
const std::string DEBUG_::Item22::itemName = "Item22";
const int DEBUG_::Item22::defaultValue = 0;
const std::string DEBUG_::Item23::itemName = "Item23";
const int DEBUG_::Item23::defaultValue = 0;
const std::string DEBUG_::Item24::itemName = "Item24";
const int DEBUG_::Item24::defaultValue = 0;
const std::string DEBUG_::Item25::itemName = "Item25";
const int DEBUG_::Item25::defaultValue = 0;
const std::string DEBUG_::Item26::itemName = "Item26";
const int DEBUG_::Item26::defaultValue = 0;
const std::string DEBUG_::Item27::itemName = "Item27";
const int DEBUG_::Item27::defaultValue = 0;
const std::string DEBUG_::Item28::itemName = "Item28";
const int DEBUG_::Item28::defaultValue = 0;
const std::string DEBUG_::Item29::itemName = "Item29";
const int DEBUG_::Item29::defaultValue = 0;
const std::string DEBUG_::Item30::itemName = "Item30";
const int DEBUG_::Item30::defaultValue = 0;
const std::string DEBUG_::Item31::itemName = "Item31";
const int DEBUG_::Item31::defaultValue = 0;
const std::string DEBUG_::Item32::itemName = "Item32";
const int DEBUG_::Item32::defaultValue = 0;
const std::string DEBUG_::Item33::itemName = "Item33";
const int DEBUG_::Item33::defaultValue = 0;
const std::string DEBUG_::Item34::itemName = "Item34";
const int DEBUG_::Item34::defaultValue = 0;
const std::string DEBUG_::Item35::itemName = "Item35";
const int DEBUG_::Item35::defaultValue = 0;
const std::string DEBUG_::Item36::itemName = "Item36";
const int DEBUG_::Item36::defaultValue = 0;
const std::string DEBUG_::Item37::itemName = "Item37";
const int DEBUG_::Item37::defaultValue = 0;
const std::string DEBUG_::Item38::itemName = "Item38";
const int DEBUG_::Item38::defaultValue = 0;
const std::string DEBUG_::Item39::itemName = "Item39";
const int DEBUG_::Item39::defaultValue = 0;
const std::string DEBUG_::Item40::itemName = "Item40";
const int DEBUG_::Item40::defaultValue = 0;
const std::string DEBUG_::Item41::itemName = "Item41";
const int DEBUG_::Item41::defaultValue = 0;
const std::string DEBUG_::Item42::itemName = "Item42";
const int DEBUG_::Item42::defaultValue = 0;
const std::string DEBUG_::Item43::itemName = "Item43";
const int DEBUG_::Item43::defaultValue = 0;
const std::string DEBUG_::Item44::itemName = "Item44";
const int DEBUG_::Item44::defaultValue = 0;
const std::string DEBUG_::Item45::itemName = "Item45";
const int DEBUG_::Item45::defaultValue = 0;
const std::string DEBUG_::Item46::itemName = "Item46";
const int DEBUG_::Item46::defaultValue = 0;
const std::string DEBUG_::Item47::itemName = "Item47";
const int DEBUG_::Item47::defaultValue = 0;
const std::string DEBUG_::Item48::itemName = "Item48";
const int DEBUG_::Item48::defaultValue = 0;
const std::string DEBUG_::Item49::itemName = "Item49";
const int DEBUG_::Item49::defaultValue = 0;
const std::string DEBUG_::Item50::itemName = "Item50";
const int DEBUG_::Item50::defaultValue = 0;
const std::string DEBUG_::Item51::itemName = "Item51";
const int DEBUG_::Item51::defaultValue = 0;
const std::string DEBUG_::Item52::itemName = "Item52";
const int DEBUG_::Item52::defaultValue = 0;
const std::string DEBUG_::Item53::itemName = "Item53";
const int DEBUG_::Item53::defaultValue = 0;
const std::string DEBUG_::Item54::itemName = "Item54";
const int DEBUG_::Item54::defaultValue = 0;
const std::string DEBUG_::Item55::itemName = "Item55";
const int DEBUG_::Item55::defaultValue = 0;
const std::string DEBUG_::Item56::itemName = "Item56";
const int DEBUG_::Item56::defaultValue = 0;
const std::string DEBUG_::Item57::itemName = "Item57";
const int DEBUG_::Item57::defaultValue = 0;
const std::string DEBUG_::Item58::itemName = "Item58";
const int DEBUG_::Item58::defaultValue = 0;
const std::string DEBUG_::Item59::itemName = "Item59";
const int DEBUG_::Item59::defaultValue = 0;
const std::string DEBUG_::Item60::itemName = "Item60";
const int DEBUG_::Item60::defaultValue = 0;
const std::string DEBUG_::Item61::itemName = "Item61";
const int DEBUG_::Item61::defaultValue = 0;
const std::string DEBUG_::Item62::itemName = "Item62";
const int DEBUG_::Item62::defaultValue = 0;
const std::string DEBUG_::Item63::itemName = "Item63";
const int DEBUG_::Item63::defaultValue = 0;
const std::string DEBUG_::Item64::itemName = "Item64";
const int DEBUG_::Item64::defaultValue = 0;
const std::string DEBUG_::Item65::itemName = "Item65";
const int DEBUG_::Item65::defaultValue = 0;
const std::string DEBUG_::Item66::itemName = "Item66";
const int DEBUG_::Item66::defaultValue = 0;
const std::string DEBUG_::Item67::itemName = "Item67";
const int DEBUG_::Item67::defaultValue = 0;
const std::string DEBUG_::Item68::itemName = "Item68";
const int DEBUG_::Item68::defaultValue = 0;
const std::string DEBUG_::Item69::itemName = "Item69";
const int DEBUG_::Item69::defaultValue = 0;
const std::string DEBUG_::Item70::itemName = "Item70";
const int DEBUG_::Item70::defaultValue = 0;
const std::string DEBUG_::Item71::itemName = "Item71";
const int DEBUG_::Item71::defaultValue = 0;
const std::string DEBUG_::Item72::itemName = "Item72";
const int DEBUG_::Item72::defaultValue = 0;
const std::string DEBUG_::Item73::itemName = "Item73";
const int DEBUG_::Item73::defaultValue = 0;
const std::string DEBUG_::Item74::itemName = "Item74";
const int DEBUG_::Item74::defaultValue = 0;
const std::string DEBUG_::Item75::itemName = "Item75";
const int DEBUG_::Item75::defaultValue = 0;
const std::string DEBUG_::Item76::itemName = "Item76";
const int DEBUG_::Item76::defaultValue = 0;
const std::string DEBUG_::Item77::itemName = "Item77";
const int DEBUG_::Item77::defaultValue = 0;
const std::string DEBUG_::Item78::itemName = "Item78";
const int DEBUG_::Item78::defaultValue = 0;
const std::string DEBUG_::Item79::itemName = "Item79";
const int DEBUG_::Item79::defaultValue = 0;
const std::string DEBUG_::Item80::itemName = "Item80";
const int DEBUG_::Item80::defaultValue = 0;
const std::string DEBUG_::Item81::itemName = "Item81";
const int DEBUG_::Item81::defaultValue = 0;
const std::string DEBUG_::Item82::itemName = "Item82";
const int DEBUG_::Item82::defaultValue = 0;
const std::string DEBUG_::Item83::itemName = "Item83";
const int DEBUG_::Item83::defaultValue = 0;
const std::string DEBUG_::Item84::itemName = "Item84";
const int DEBUG_::Item84::defaultValue = 0;
const std::string DEBUG_::Item85::itemName = "Item85";
const int DEBUG_::Item85::defaultValue = 0;
const std::string DEBUG_::Item86::itemName = "Item86";
const int DEBUG_::Item86::defaultValue = 0;
const std::string DEBUG_::Item87::itemName = "Item87";
const int DEBUG_::Item87::defaultValue = 0;


DELAYACT::DELAYACT( ) : ParserKeyword("DELAYACT")
{
  setFixedSize( (size_t) 1);
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
const int DELAYACT::NUM_TIMES::defaultValue = 1;
const std::string DELAYACT::INCREMENT::itemName = "INCREMENT";
const double DELAYACT::INCREMENT::defaultValue = 0.000000;


DENSITY::DENSITY( ) : ParserKeyword("DENSITY")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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
const double DENSITY::OIL::defaultValue = 600.000000;
const std::string DENSITY::WATER::itemName = "WATER";
const double DENSITY::WATER::defaultValue = 999.014000;
const std::string DENSITY::GAS::itemName = "GAS";
const double DENSITY::GAS::defaultValue = 1.000000;


DEPTH::DEPTH( ) : ParserKeyword("DEPTH")
{
  setFixedSize( (size_t) 1);
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


DEPTHTAB::DEPTHTAB( ) : ParserKeyword("DEPTHTAB")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("RIVRDIMS","NMDEPT",0);
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


DEPTHZ::DEPTHZ( ) : ParserKeyword("DEPTHZ")
{
  setFixedSize( (size_t) 1);
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


DIAGDISP::DIAGDISP( ) : ParserKeyword("DIAGDISP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIAGDISP");
}
const std::string DIAGDISP::keywordName = "DIAGDISP";


DIFF::DIFF( ) : ParserKeyword("DIFF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DIFF");
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
const double DIFF::data::defaultValue = 1.000000;


DIFFC::DIFFC( ) : ParserKeyword("DIFFC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIFFC");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_MOL_WEIGHT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_MOL_WEIGHT", ParserItem::itype::DOUBLE);
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
        item.push_backDimension("Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_OIL_CROSS_DIFF_COEFF", ParserItem::itype::DOUBLE);
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


DIFFCOAL::DIFFCOAL( ) : ParserKeyword("DIFFCOAL")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NTCREG",0);
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
const double DIFFCOAL::RE_ADSORB_FRACTION::defaultValue = 1.000000;
const std::string DIFFCOAL::SOL_DIFF_COEFF::itemName = "SOL_DIFF_COEFF";


DIFFDP::DIFFDP( ) : ParserKeyword("DIFFDP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DIFFDP");
}
const std::string DIFFDP::keywordName = "DIFFDP";


DIFFMMF::DIFFMMF( ) : ParserKeyword("DIFFMMF")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMMF::data::defaultValue = 1.000000;


DIFFMR::DIFFMR( ) : ParserKeyword("DIFFMR")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMR::data::defaultValue = 1.000000;


DIFFMTHT::DIFFMTHT( ) : ParserKeyword("DIFFMTHT")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMTHT::data::defaultValue = 1.000000;


DIFFMX::DIFFMX( ) : ParserKeyword("DIFFMX")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMX::data::defaultValue = 1.000000;


DIFFMY::DIFFMY( ) : ParserKeyword("DIFFMY")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMY::data::defaultValue = 1.000000;


DIFFMZ::DIFFMZ( ) : ParserKeyword("DIFFMZ")
{
  setFixedSize( (size_t) 1);
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
const double DIFFMZ::data::defaultValue = 1.000000;


DIFFR::DIFFR( ) : ParserKeyword("DIFFR")
{
  setFixedSize( (size_t) 1);
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


DIFFTHT::DIFFTHT( ) : ParserKeyword("DIFFTHT")
{
  setFixedSize( (size_t) 1);
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


DIFFUSE::DIFFUSE( ) : ParserKeyword("DIFFUSE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DIFFUSE");
}
const std::string DIFFUSE::keywordName = "DIFFUSE";


DIFFX::DIFFX( ) : ParserKeyword("DIFFX")
{
  setFixedSize( (size_t) 1);
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


DIFFY::DIFFY( ) : ParserKeyword("DIFFY")
{
  setFixedSize( (size_t) 1);
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


DIFFZ::DIFFZ( ) : ParserKeyword("DIFFZ")
{
  setFixedSize( (size_t) 1);
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


DIMENS::DIMENS( ) : ParserKeyword("DIMENS")
{
  setFixedSize( (size_t) 1);
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


DIMPES::DIMPES( ) : ParserKeyword("DIMPES")
{
  setFixedSize( (size_t) 1);
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
const double DIMPES::DSTARG::defaultValue = 0.050000;
const std::string DIMPES::DSMAX::itemName = "DSMAX";
const double DIMPES::DSMAX::defaultValue = 0.100000;
const std::string DIMPES::DPMAX::itemName = "DPMAX";
const double DIMPES::DPMAX::defaultValue = 13.790000;


DIMPLICT::DIMPLICT( ) : ParserKeyword("DIMPLICT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DIMPLICT");
}
const std::string DIMPLICT::keywordName = "DIMPLICT";


DISGAS::DISGAS( ) : ParserKeyword("DISGAS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DISGAS");
}
const std::string DISGAS::keywordName = "DISGAS";


DISPDIMS::DISPDIMS( ) : ParserKeyword("DISPDIMS")
{
  setFixedSize( (size_t) 1);
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
const int DISPDIMS::NUM_DISP_TABLES::defaultValue = 1;
const std::string DISPDIMS::MAX_VELOCITY_NODES::itemName = "MAX_VELOCITY_NODES";
const int DISPDIMS::MAX_VELOCITY_NODES::defaultValue = 2;
const std::string DISPDIMS::MAX_CONCENTRATION_NODES::itemName = "MAX_CONCENTRATION_NODES";
const int DISPDIMS::MAX_CONCENTRATION_NODES::defaultValue = 1;


DISPERSE::DISPERSE( ) : ParserKeyword("DISPERSE")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("DISPDIMS","MXDIST",0);
  setTableCollection( true );
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


DOMAINS::DOMAINS( ) : ParserKeyword("DOMAINS")
{
  setFixedSize( (size_t) 1);
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


DPGRID::DPGRID( ) : ParserKeyword("DPGRID")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DPGRID");
}
const std::string DPGRID::keywordName = "DPGRID";


DPKRMOD::DPKRMOD( ) : ParserKeyword("DPKRMOD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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
const double DPKRMOD::MOD_OIL_WAT_PERM::defaultValue = 0.000000;
const std::string DPKRMOD::MOD_OIL_GAS_PERM::itemName = "MOD_OIL_GAS_PERM";
const double DPKRMOD::MOD_OIL_GAS_PERM::defaultValue = 0.000000;
const std::string DPKRMOD::SCALE_PERM_FRACTURE::itemName = "SCALE_PERM_FRACTURE";
const std::string DPKRMOD::SCALE_PERM_FRACTURE::defaultValue = "YES";


DPNUM::DPNUM( ) : ParserKeyword("DPNUM")
{
  setFixedSize( (size_t) 1);
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


DR::DR( ) : ParserKeyword("DR")
{
  setFixedSize( (size_t) 1);
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


DREF::DREF( ) : ParserKeyword("DREF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


DREFS::DREFS( ) : ParserKeyword("DREFS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


DRILPRI::DRILPRI( ) : ParserKeyword("DRILPRI")
{
  setFixedSize( (size_t) 1);
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
const double DRILPRI::INTERVAL::defaultValue = 0.000000;
const std::string DRILPRI::A::itemName = "A";
const double DRILPRI::A::defaultValue = 0.000000;
const std::string DRILPRI::B::itemName = "B";
const double DRILPRI::B::defaultValue = 0.000000;
const std::string DRILPRI::C::itemName = "C";
const double DRILPRI::C::defaultValue = 0.000000;
const std::string DRILPRI::D::itemName = "D";
const double DRILPRI::D::defaultValue = 0.000000;
const std::string DRILPRI::E::itemName = "E";
const double DRILPRI::E::defaultValue = 0.000000;
const std::string DRILPRI::F::itemName = "F";
const double DRILPRI::F::defaultValue = 0.000000;
const std::string DRILPRI::G::itemName = "G";
const double DRILPRI::G::defaultValue = 0.000000;
const std::string DRILPRI::H::itemName = "H";
const double DRILPRI::H::defaultValue = 0.000000;
const std::string DRILPRI::LOOK_AHEAD::itemName = "LOOK_AHEAD";
const double DRILPRI::LOOK_AHEAD::defaultValue = 0.000000;
const std::string DRILPRI::CALCULATION::itemName = "CALCULATION";
const std::string DRILPRI::CALCULATION::defaultValue = "SINGLE";


DRSDT::DRSDT( ) : ParserKeyword("DRSDT")
{
  setFixedSize( (size_t) 1);
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
        ParserItem item("Option", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDT::keywordName = "DRSDT";
const std::string DRSDT::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDT::Option::itemName = "Option";
const std::string DRSDT::Option::defaultValue = "ALL";


DRSDTR::DRSDTR( ) : ParserKeyword("DRSDTR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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
        ParserItem item("Option", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDTR::keywordName = "DRSDTR";
const std::string DRSDTR::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDTR::Option::itemName = "Option";
const std::string DRSDTR::Option::defaultValue = "ALL";


DRV::DRV( ) : ParserKeyword("DRV")
{
  setFixedSize( (size_t) 1);
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


DRVDT::DRVDT( ) : ParserKeyword("DRVDT")
{
  setFixedSize( (size_t) 1);
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


DRVDTR::DRVDTR( ) : ParserKeyword("DRVDTR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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


DSPDEINT::DSPDEINT( ) : ParserKeyword("DSPDEINT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DSPDEINT");
}
const std::string DSPDEINT::keywordName = "DSPDEINT";


DTHETA::DTHETA( ) : ParserKeyword("DTHETA")
{
  setFixedSize( (size_t) 1);
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


DTHETAV::DTHETAV( ) : ParserKeyword("DTHETAV")
{
  setFixedSize( (size_t) 1);
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


DUALPERM::DUALPERM( ) : ParserKeyword("DUALPERM")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DUALPERM");
}
const std::string DUALPERM::keywordName = "DUALPERM";


DUALPORO::DUALPORO( ) : ParserKeyword("DUALPORO")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DUALPORO");
}
const std::string DUALPORO::keywordName = "DUALPORO";


DUMPCUPL::DUMPCUPL( ) : ParserKeyword("DUMPCUPL")
{
  setFixedSize( (size_t) 1);
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


DUMPFLUX::DUMPFLUX( ) : ParserKeyword("DUMPFLUX")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DUMPFLUX");
}
const std::string DUMPFLUX::keywordName = "DUMPFLUX";


DX::DX( ) : ParserKeyword("DX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
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


DXV::DXV( ) : ParserKeyword("DXV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
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


DY::DY( ) : ParserKeyword("DY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
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


DYNAMICR::DYNAMICR( ) : ParserKeyword("DYNAMICR")
{
  setSizeType(FIXED_CODE);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
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


DYNRDIMS::DYNRDIMS( ) : ParserKeyword("DYNRDIMS")
{
  setFixedSize( (size_t) 1);
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
const int DYNRDIMS::MNUMDR::defaultValue = 0;
const std::string DYNRDIMS::MXDYNF::itemName = "MXDYNF";
const int DYNRDIMS::MXDYNF::defaultValue = 0;
const std::string DYNRDIMS::MXDYNR::itemName = "MXDYNR";
const int DYNRDIMS::MXDYNR::defaultValue = 0;


DYV::DYV( ) : ParserKeyword("DYV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
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


DZ::DZ( ) : ParserKeyword("DZ")
{
  setFixedSize( (size_t) 1);
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


DZMATRIX::DZMATRIX( ) : ParserKeyword("DZMATRIX")
{
  setFixedSize( (size_t) 1);
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
const double DZMATRIX::data::defaultValue = 0.000000;


DZMTRX::DZMTRX( ) : ParserKeyword("DZMTRX")
{
  setFixedSize( (size_t) 1);
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
const double DZMTRX::data::defaultValue = 0.000000;


DZMTRXV::DZMTRXV( ) : ParserKeyword("DZMTRXV")
{
  setFixedSize( (size_t) 1);
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
const double DZMTRXV::data::defaultValue = 0.000000;


DZNET::DZNET( ) : ParserKeyword("DZNET")
{
  setFixedSize( (size_t) 1);
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


DZV::DZV( ) : ParserKeyword("DZV")
{
  setFixedSize( (size_t) 1);
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
