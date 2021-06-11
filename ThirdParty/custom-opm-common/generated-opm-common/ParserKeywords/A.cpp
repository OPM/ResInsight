#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/A.hpp>
namespace Opm {
namespace ParserKeywords {
ACTDIMS::ACTDIMS( ) : ParserKeyword("ACTDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ACTDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_ACTION", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ACTION_LINES", ParserItem::itype::INT);
        item.setDefault( 50 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ACTION_LINE_CHARACTERS", ParserItem::itype::INT);
        item.setDefault( 80 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ACTION_COND", ParserItem::itype::INT);
        item.setDefault( 3 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTDIMS::keywordName = "ACTDIMS";
const std::string ACTDIMS::MAX_ACTION::itemName = "MAX_ACTION";
const int ACTDIMS::MAX_ACTION::defaultValue = 2;
const std::string ACTDIMS::MAX_ACTION_LINES::itemName = "MAX_ACTION_LINES";
const int ACTDIMS::MAX_ACTION_LINES::defaultValue = 50;
const std::string ACTDIMS::MAX_ACTION_LINE_CHARACTERS::itemName = "MAX_ACTION_LINE_CHARACTERS";
const int ACTDIMS::MAX_ACTION_LINE_CHARACTERS::defaultValue = 80;
const std::string ACTDIMS::MAX_ACTION_COND::itemName = "MAX_ACTION_COND";
const int ACTDIMS::MAX_ACTION_COND::defaultValue = 3;


ACTION::ACTION( ) : ParserKeyword("ACTION")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTION");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTION::keywordName = "ACTION";
const std::string ACTION::ACTION_NAME::itemName = "ACTION_NAME";
const std::string ACTION::QUANTITY::itemName = "QUANTITY";
const std::string ACTION::OPERATOR::itemName = "OPERATOR";
const std::string ACTION::TRIGGER_VALUE::itemName = "TRIGGER_VALUE";


ACTIONG::ACTIONG( ) : ParserKeyword("ACTIONG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTIONG");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REPETITIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTIONG::keywordName = "ACTIONG";
const std::string ACTIONG::ACTION::itemName = "ACTION";
const std::string ACTIONG::GROUP_NAME::itemName = "GROUP_NAME";
const std::string ACTIONG::QUANTITY::itemName = "QUANTITY";
const std::string ACTIONG::OPERATOR::itemName = "OPERATOR";
const std::string ACTIONG::TRIGGER_VALUE::itemName = "TRIGGER_VALUE";
const std::string ACTIONG::REPETITIONS::itemName = "REPETITIONS";
const int ACTIONG::REPETITIONS::defaultValue = 1;
const std::string ACTIONG::INCREMENT::itemName = "INCREMENT";
const double ACTIONG::INCREMENT::defaultValue = 0.000000;


ACTIONR::ACTIONR( ) : ParserKeyword("ACTIONR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTIONR");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FLUID_IN_PLACE_NR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FLUID_IN_PLACE_REG_FAM", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REPETITIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTIONR::keywordName = "ACTIONR";
const std::string ACTIONR::ACTION::itemName = "ACTION";
const std::string ACTIONR::FLUID_IN_PLACE_NR::itemName = "FLUID_IN_PLACE_NR";
const std::string ACTIONR::FLUID_IN_PLACE_REG_FAM::itemName = "FLUID_IN_PLACE_REG_FAM";
const std::string ACTIONR::QUANTITY::itemName = "QUANTITY";
const std::string ACTIONR::OPERATOR::itemName = "OPERATOR";
const std::string ACTIONR::TRIGGER_VALUE::itemName = "TRIGGER_VALUE";
const std::string ACTIONR::REPETITIONS::itemName = "REPETITIONS";
const int ACTIONR::REPETITIONS::defaultValue = 1;
const std::string ACTIONR::INCREMENT::itemName = "INCREMENT";
const double ACTIONR::INCREMENT::defaultValue = 0.000000;


ACTIONS::ACTIONS( ) : ParserKeyword("ACTIONS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTIONS");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELL_SEGMENT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REPETITIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTIONS::keywordName = "ACTIONS";
const std::string ACTIONS::ACTION::itemName = "ACTION";
const std::string ACTIONS::WELL::itemName = "WELL";
const std::string ACTIONS::WELL_SEGMENT::itemName = "WELL_SEGMENT";
const std::string ACTIONS::QUANTITY::itemName = "QUANTITY";
const std::string ACTIONS::OPERATOR::itemName = "OPERATOR";
const std::string ACTIONS::TRIGGER_VALUE::itemName = "TRIGGER_VALUE";
const std::string ACTIONS::REPETITIONS::itemName = "REPETITIONS";
const int ACTIONS::REPETITIONS::defaultValue = 1;
const std::string ACTIONS::INCREMENT::itemName = "INCREMENT";
const double ACTIONS::INCREMENT::defaultValue = 0.000000;


ACTIONW::ACTIONW( ) : ParserKeyword("ACTIONW")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTIONW");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REPETITIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTIONW::keywordName = "ACTIONW";
const std::string ACTIONW::ACTION::itemName = "ACTION";
const std::string ACTIONW::WELL_NAME::itemName = "WELL_NAME";
const std::string ACTIONW::QUANTITY::itemName = "QUANTITY";
const std::string ACTIONW::OPERATOR::itemName = "OPERATOR";
const std::string ACTIONW::TRIGGER_VALUE::itemName = "TRIGGER_VALUE";
const std::string ACTIONW::REPETITIONS::itemName = "REPETITIONS";
const int ACTIONW::REPETITIONS::defaultValue = 1;
const std::string ACTIONW::INCREMENT::itemName = "INCREMENT";
const double ACTIONW::INCREMENT::defaultValue = 0.000000;


ACTIONX::ACTIONX( ) : ParserKeyword("ACTIONX")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ACTIONX");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_WAIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("CONDITION", ParserItem::itype::RAW_STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTIONX::keywordName = "ACTIONX";
const std::string ACTIONX::NAME::itemName = "NAME";
const std::string ACTIONX::NUM::itemName = "NUM";
const int ACTIONX::NUM::defaultValue = 1;
const std::string ACTIONX::MIN_WAIT::itemName = "MIN_WAIT";
const double ACTIONX::MIN_WAIT::defaultValue = 0.000000;
const std::string ACTIONX::CONDITION::itemName = "CONDITION";


ACTNUM::ACTNUM( ) : ParserKeyword("ACTNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ACTNUM");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ACTNUM::keywordName = "ACTNUM";
const std::string ACTNUM::data::itemName = "data";


ACTPARAM::ACTPARAM( ) : ParserKeyword("ACTPARAM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ACTPARAM");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0.000100) );
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ACTPARAM::keywordName = "ACTPARAM";
const std::string ACTPARAM::data::itemName = "data";
const double ACTPARAM::data::defaultValue = 0.000100;


ADD::ADD( ) : ParserKeyword("ADD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ADD");
  {
     ParserRecord record;
     {
        ParserItem item("field", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("shift", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADD::keywordName = "ADD";
const std::string ADD::field::itemName = "field";
const std::string ADD::shift::itemName = "shift";
const std::string ADD::I1::itemName = "I1";
const std::string ADD::I2::itemName = "I2";
const std::string ADD::J1::itemName = "J1";
const std::string ADD::J2::itemName = "J2";
const std::string ADD::K1::itemName = "K1";
const std::string ADD::K2::itemName = "K2";


ADDREG::ADDREG( ) : ParserKeyword("ADDREG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ADDREG");
  {
     ParserRecord record;
     {
        ParserItem item("ARRAY", ParserItem::itype::STRING);
        item.setDescription("The 3D array we will update");
        record.addItem(item);
     }
     {
        ParserItem item("SHIFT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.setDescription("The value we will multiply with");
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NUMBER", ParserItem::itype::INT);
        item.setDescription("The region number we are interested in");
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NAME", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        item.setDescription("The name of the region we are interested in");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADDREG::keywordName = "ADDREG";
const std::string ADDREG::ARRAY::itemName = "ARRAY";
const std::string ADDREG::SHIFT::itemName = "SHIFT";
const double ADDREG::SHIFT::defaultValue = 0.000000;
const std::string ADDREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string ADDREG::REGION_NAME::itemName = "REGION_NAME";
const std::string ADDREG::REGION_NAME::defaultValue = "M";


ADDZCORN::ADDZCORN( ) : ParserKeyword("ADDZCORN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ADDZCORN");
  {
     ParserRecord record;
     {
        ParserItem item("ADDED_VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("IX1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KZ1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KZ2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX1A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADDZCORN::keywordName = "ADDZCORN";
const std::string ADDZCORN::ADDED_VALUE::itemName = "ADDED_VALUE";
const std::string ADDZCORN::IX1::itemName = "IX1";
const std::string ADDZCORN::IX2::itemName = "IX2";
const std::string ADDZCORN::JY1::itemName = "JY1";
const std::string ADDZCORN::JY2::itemName = "JY2";
const std::string ADDZCORN::KZ1::itemName = "KZ1";
const std::string ADDZCORN::KZ2::itemName = "KZ2";
const std::string ADDZCORN::IX1A::itemName = "IX1A";
const std::string ADDZCORN::IX2A::itemName = "IX2A";
const std::string ADDZCORN::JY1A::itemName = "JY1A";
const std::string ADDZCORN::JY2A::itemName = "JY2A";
const std::string ADDZCORN::ACTION::itemName = "ACTION";


ADSALNOD::ADSALNOD( ) : ParserKeyword("ADSALNOD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ADSALNOD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADSALNOD::keywordName = "ADSALNOD";
const std::string ADSALNOD::DATA::itemName = "DATA";


ADSORP::ADSORP( ) : ParserKeyword("ADSORP")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ADSORP");
  {
     ParserRecord record;
     {
        ParserItem item("ADSORBING_COMP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("ADORPTION_ISOTHERM", ParserItem::itype::STRING);
        item.setDefault( std::string("LANGMUIR") );
        record.addItem(item);
     }
     {
        ParserItem item("A1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("A2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("M", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("N", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("K_REF", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADSORP::keywordName = "ADSORP";
const std::string ADSORP::ADSORBING_COMP::itemName = "ADSORBING_COMP";
const std::string ADSORP::ADORPTION_ISOTHERM::itemName = "ADORPTION_ISOTHERM";
const std::string ADSORP::ADORPTION_ISOTHERM::defaultValue = "LANGMUIR";
const std::string ADSORP::A1::itemName = "A1";
const std::string ADSORP::A2::itemName = "A2";
const std::string ADSORP::B::itemName = "B";
const std::string ADSORP::M::itemName = "M";
const double ADSORP::M::defaultValue = 0.500000;
const std::string ADSORP::N::itemName = "N";
const double ADSORP::N::defaultValue = 0.500000;
const std::string ADSORP::K_REF::itemName = "K_REF";


AITS::AITS( ) : ParserKeyword("AITS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("AITS");
}
const std::string AITS::keywordName = "AITS";


AITSOFF::AITSOFF( ) : ParserKeyword("AITSOFF")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("AITSOFF");
}
const std::string AITSOFF::keywordName = "AITSOFF";


ALKADS::ALKADS( ) : ParserKeyword("ALKADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ALKADS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ALKADS::keywordName = "ALKADS";
const std::string ALKADS::DATA::itemName = "DATA";


ALKALINE::ALKALINE( ) : ParserKeyword("ALKALINE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ALKALINE");
}
const std::string ALKALINE::keywordName = "ALKALINE";


ALKROCK::ALKROCK( ) : ParserKeyword("ALKROCK")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ALKROCK");
  {
     ParserRecord record;
     {
        ParserItem item("ROCK_ADS_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ALKROCK::keywordName = "ALKROCK";
const std::string ALKROCK::ROCK_ADS_INDEX::itemName = "ROCK_ADS_INDEX";


ALL::ALL( ) : ParserKeyword("ALL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ALL");
}
const std::string ALL::keywordName = "ALL";


ALPOLADS::ALPOLADS( ) : ParserKeyword("ALPOLADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ALPOLADS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ALPOLADS::keywordName = "ALPOLADS";
const std::string ALPOLADS::DATA::itemName = "DATA";


ALSURFAD::ALSURFAD( ) : ParserKeyword("ALSURFAD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ALSURFAD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ALSURFAD::keywordName = "ALSURFAD";
const std::string ALSURFAD::DATA::itemName = "DATA";


ALSURFST::ALSURFST( ) : ParserKeyword("ALSURFST")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ALSURFST");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ALSURFST::keywordName = "ALSURFST";
const std::string ALSURFST::DATA::itemName = "DATA";


AMALGAM::AMALGAM( ) : ParserKeyword("AMALGAM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AMALGAM");
  {
     ParserRecord record;
     {
        ParserItem item("LGR_GROUPS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AMALGAM::keywordName = "AMALGAM";
const std::string AMALGAM::LGR_GROUPS::itemName = "LGR_GROUPS";


API::API( ) : ParserKeyword("API")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("API");
}
const std::string API::keywordName = "API";


APIGROUP::APIGROUP( ) : ParserKeyword("APIGROUP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("APIGROUP");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_OIL_PVT_GROUP_COUNT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string APIGROUP::keywordName = "APIGROUP";
const std::string APIGROUP::MAX_OIL_PVT_GROUP_COUNT::itemName = "MAX_OIL_PVT_GROUP_COUNT";
const int APIGROUP::MAX_OIL_PVT_GROUP_COUNT::defaultValue = 1;


APILIM::APILIM( ) : ParserKeyword("APILIM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("APILIM");
  {
     ParserRecord record;
     {
        ParserItem item("LIMITER", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("SCOPE", ParserItem::itype::STRING);
        item.setDefault( std::string("BOTH") );
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_API_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_API_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_ROWS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string APILIM::keywordName = "APILIM";
const std::string APILIM::LIMITER::itemName = "LIMITER";
const std::string APILIM::LIMITER::defaultValue = "NONE";
const std::string APILIM::SCOPE::itemName = "SCOPE";
const std::string APILIM::SCOPE::defaultValue = "BOTH";
const std::string APILIM::LOWER_API_LIMIT::itemName = "LOWER_API_LIMIT";
const std::string APILIM::UPPER_API_LIMIT::itemName = "UPPER_API_LIMIT";
const std::string APILIM::NUM_ROWS::itemName = "NUM_ROWS";
const int APILIM::NUM_ROWS::defaultValue = 10;


APIVID::APIVID( ) : ParserKeyword("APIVID")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("APIVID");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string APIVID::keywordName = "APIVID";
const std::string APIVID::DATA::itemName = "DATA";


AQANCONL::AQANCONL( ) : ParserKeyword("AQANCONL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQANCONL");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_K", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_K", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_INDX", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_INFLUX_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_INFLUX_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("ALLOW", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQANCONL::keywordName = "AQANCONL";
const std::string AQANCONL::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQANCONL::NAME::itemName = "NAME";
const std::string AQANCONL::LOWER_I::itemName = "LOWER_I";
const std::string AQANCONL::UPPER_I::itemName = "UPPER_I";
const std::string AQANCONL::LOWER_J::itemName = "LOWER_J";
const std::string AQANCONL::UPPER_J::itemName = "UPPER_J";
const std::string AQANCONL::LOWER_K::itemName = "LOWER_K";
const std::string AQANCONL::UPPER_K::itemName = "UPPER_K";
const std::string AQANCONL::FACE_INDX::itemName = "FACE_INDX";
const std::string AQANCONL::AQUIFER_INFLUX_COEFF::itemName = "AQUIFER_INFLUX_COEFF";
const std::string AQANCONL::AQUIFER_INFLUX_MULT::itemName = "AQUIFER_INFLUX_MULT";
const double AQANCONL::AQUIFER_INFLUX_MULT::defaultValue = 1.000000;
const std::string AQANCONL::ALLOW::itemName = "ALLOW";


AQANNC::AQANNC( ) : ParserKeyword("AQANNC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQANNC");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("AREA", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQANNC::keywordName = "AQANNC";
const std::string AQANNC::AQUIFER_NUMBER::itemName = "AQUIFER_NUMBER";
const std::string AQANNC::IX::itemName = "IX";
const std::string AQANNC::IY::itemName = "IY";
const std::string AQANNC::IZ::itemName = "IZ";
const std::string AQANNC::AREA::itemName = "AREA";
const double AQANNC::AREA::defaultValue = 0.000000;


AQANTRC::AQANTRC( ) : ParserKeyword("AQANTRC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQANTRC");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQANTRC::keywordName = "AQANTRC";
const std::string AQANTRC::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQANTRC::TRACER::itemName = "TRACER";
const std::string AQANTRC::VALUE::itemName = "VALUE";


AQUALIST::AQUALIST( ) : ParserKeyword("AQUALIST")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUALIST");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_LIST", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LIST", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUALIST::keywordName = "AQUALIST";
const std::string AQUALIST::AQUIFER_LIST::itemName = "AQUIFER_LIST";
const std::string AQUALIST::LIST::itemName = "LIST";


AQUANCON::AQUANCON( ) : ParserKeyword("AQUANCON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUANCON");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INFLUX_COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("INFLUX_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("CONNECT_ADJOINING_ACTIVE_CELL", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUANCON::keywordName = "AQUANCON";
const std::string AQUANCON::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUANCON::I1::itemName = "I1";
const std::string AQUANCON::I2::itemName = "I2";
const std::string AQUANCON::J1::itemName = "J1";
const std::string AQUANCON::J2::itemName = "J2";
const std::string AQUANCON::K1::itemName = "K1";
const std::string AQUANCON::K2::itemName = "K2";
const std::string AQUANCON::FACE::itemName = "FACE";
const std::string AQUANCON::INFLUX_COEFF::itemName = "INFLUX_COEFF";
const std::string AQUANCON::INFLUX_MULT::itemName = "INFLUX_MULT";
const double AQUANCON::INFLUX_MULT::defaultValue = 1.000000;
const std::string AQUANCON::CONNECT_ADJOINING_ACTIVE_CELL::itemName = "CONNECT_ADJOINING_ACTIVE_CELL";
const std::string AQUANCON::CONNECT_ADJOINING_ACTIVE_CELL::defaultValue = "NO";


AQUCHGAS::AQUCHGAS( ) : ParserKeyword("AQUCHGAS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUCHGAS");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PROD_INDEX", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasSurfaceVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCHGAS::keywordName = "AQUCHGAS";
const std::string AQUCHGAS::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUCHGAS::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string AQUCHGAS::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string AQUCHGAS::AQUIFER_PROD_INDEX::itemName = "AQUIFER_PROD_INDEX";
const std::string AQUCHGAS::TABLE_NUM::itemName = "TABLE_NUM";
const int AQUCHGAS::TABLE_NUM::defaultValue = 1;
const std::string AQUCHGAS::TEMPERATURE::itemName = "TEMPERATURE";


AQUCHWAT::AQUCHWAT( ) : ParserKeyword("AQUCHWAT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUCHWAT");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INPUT_4", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ITEM4", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PROD_INDEX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INIT_SALT_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("MINIMUM", ParserItem::itype::DOUBLE);
        item.setDefault( double(-99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAXIMUM", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("IGNORE_CAP_PRESSURE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_FLOW_PR_CONN", ParserItem::itype::DOUBLE);
        item.setDefault( double(-99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FLOW_PR_CONN", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("REMOVE_DEPTH_TERM", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("IMPORT_MAX_MIN_FLOW_RATE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCHWAT::keywordName = "AQUCHWAT";
const std::string AQUCHWAT::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUCHWAT::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string AQUCHWAT::INPUT_4::itemName = "INPUT_4";
const std::string AQUCHWAT::ITEM4::itemName = "ITEM4";
const std::string AQUCHWAT::AQUIFER_PROD_INDEX::itemName = "AQUIFER_PROD_INDEX";
const std::string AQUCHWAT::TABLE_NUM::itemName = "TABLE_NUM";
const int AQUCHWAT::TABLE_NUM::defaultValue = 1;
const std::string AQUCHWAT::INIT_SALT_CONC::itemName = "INIT_SALT_CONC";
const double AQUCHWAT::INIT_SALT_CONC::defaultValue = 1.000000;
const std::string AQUCHWAT::MINIMUM::itemName = "MINIMUM";
const double AQUCHWAT::MINIMUM::defaultValue = -99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string AQUCHWAT::MAXIMUM::itemName = "MAXIMUM";
const double AQUCHWAT::MAXIMUM::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string AQUCHWAT::IGNORE_CAP_PRESSURE::itemName = "IGNORE_CAP_PRESSURE";
const std::string AQUCHWAT::IGNORE_CAP_PRESSURE::defaultValue = "NO";
const std::string AQUCHWAT::MIN_FLOW_PR_CONN::itemName = "MIN_FLOW_PR_CONN";
const double AQUCHWAT::MIN_FLOW_PR_CONN::defaultValue = -99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string AQUCHWAT::MAX_FLOW_PR_CONN::itemName = "MAX_FLOW_PR_CONN";
const double AQUCHWAT::MAX_FLOW_PR_CONN::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string AQUCHWAT::REMOVE_DEPTH_TERM::itemName = "REMOVE_DEPTH_TERM";
const std::string AQUCHWAT::REMOVE_DEPTH_TERM::defaultValue = "NO";
const std::string AQUCHWAT::IMPORT_MAX_MIN_FLOW_RATE::itemName = "IMPORT_MAX_MIN_FLOW_RATE";
const int AQUCHWAT::IMPORT_MAX_MIN_FLOW_RATE::defaultValue = 0;
const std::string AQUCHWAT::TEMPERATURE::itemName = "TEMPERATURE";


AQUCON::AQUCON( ) : ParserKeyword("AQUCON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AQUCON");
  {
     ParserRecord record;
     {
        ParserItem item("ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CONNECT_FACE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRANS_MULT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("TRANS_OPTION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ALLOW_INTERNAL_CELLS", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VEFRAC", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("VEFRACP", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCON::keywordName = "AQUCON";
const std::string AQUCON::ID::itemName = "ID";
const std::string AQUCON::I1::itemName = "I1";
const std::string AQUCON::I2::itemName = "I2";
const std::string AQUCON::J1::itemName = "J1";
const std::string AQUCON::J2::itemName = "J2";
const std::string AQUCON::K1::itemName = "K1";
const std::string AQUCON::K2::itemName = "K2";
const std::string AQUCON::CONNECT_FACE::itemName = "CONNECT_FACE";
const std::string AQUCON::TRANS_MULT::itemName = "TRANS_MULT";
const std::string AQUCON::TRANS_OPTION::itemName = "TRANS_OPTION";
const std::string AQUCON::ALLOW_INTERNAL_CELLS::itemName = "ALLOW_INTERNAL_CELLS";
const std::string AQUCON::VEFRAC::itemName = "VEFRAC";
const std::string AQUCON::VEFRACP::itemName = "VEFRACP";


AQUCT::AQUCT( ) : ParserKeyword("AQUCT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUCT");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DAT_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("P_INI", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PERM_AQ", ParserItem::itype::DOUBLE);
        item.push_backDimension("Permeability");
        record.addItem(item);
     }
     {
        ParserItem item("PORO_AQ", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("C_T", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("RAD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("THICKNESS_AQ", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INFLUENCE_ANGLE", ParserItem::itype::DOUBLE);
        item.setDefault( double(360.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM_WATER_PRESS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM_INFLUENCE_FN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("INI_SALT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     {
        ParserItem item("TEMP_AQUIFER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCT::keywordName = "AQUCT";
const std::string AQUCT::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUCT::DAT_DEPTH::itemName = "DAT_DEPTH";
const std::string AQUCT::P_INI::itemName = "P_INI";
const std::string AQUCT::PERM_AQ::itemName = "PERM_AQ";
const std::string AQUCT::PORO_AQ::itemName = "PORO_AQ";
const double AQUCT::PORO_AQ::defaultValue = 1.000000;
const std::string AQUCT::C_T::itemName = "C_T";
const std::string AQUCT::RAD::itemName = "RAD";
const std::string AQUCT::THICKNESS_AQ::itemName = "THICKNESS_AQ";
const std::string AQUCT::INFLUENCE_ANGLE::itemName = "INFLUENCE_ANGLE";
const double AQUCT::INFLUENCE_ANGLE::defaultValue = 360.000000;
const std::string AQUCT::TABLE_NUM_WATER_PRESS::itemName = "TABLE_NUM_WATER_PRESS";
const int AQUCT::TABLE_NUM_WATER_PRESS::defaultValue = 1;
const std::string AQUCT::TABLE_NUM_INFLUENCE_FN::itemName = "TABLE_NUM_INFLUENCE_FN";
const int AQUCT::TABLE_NUM_INFLUENCE_FN::defaultValue = 1;
const std::string AQUCT::INI_SALT::itemName = "INI_SALT";
const double AQUCT::INI_SALT::defaultValue = 0.000000;
const std::string AQUCT::TEMP_AQUIFER::itemName = "TEMP_AQUIFER";


AQUCWFAC::AQUCWFAC( ) : ParserKeyword("AQUCWFAC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("AQUCWFAC");
  {
     ParserRecord record;
     {
        ParserItem item("ADD_TO_DEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("MULTIPLY", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCWFAC::keywordName = "AQUCWFAC";
const std::string AQUCWFAC::ADD_TO_DEPTH::itemName = "ADD_TO_DEPTH";
const double AQUCWFAC::ADD_TO_DEPTH::defaultValue = 0.000000;
const std::string AQUCWFAC::MULTIPLY::itemName = "MULTIPLY";
const double AQUCWFAC::MULTIPLY::defaultValue = 1.000000;


AQUDIMS::AQUDIMS( ) : ParserKeyword("AQUDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("AQUDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MXNAQN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXNAQC", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NIFTBL", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NRIFTB", ParserItem::itype::INT);
        item.setDefault( 36 );
        record.addItem(item);
     }
     {
        ParserItem item("NANAQU", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NCAMAX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXNALI", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXAAQL", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUDIMS::keywordName = "AQUDIMS";
const std::string AQUDIMS::MXNAQN::itemName = "MXNAQN";
const int AQUDIMS::MXNAQN::defaultValue = 1;
const std::string AQUDIMS::MXNAQC::itemName = "MXNAQC";
const int AQUDIMS::MXNAQC::defaultValue = 1;
const std::string AQUDIMS::NIFTBL::itemName = "NIFTBL";
const int AQUDIMS::NIFTBL::defaultValue = 1;
const std::string AQUDIMS::NRIFTB::itemName = "NRIFTB";
const int AQUDIMS::NRIFTB::defaultValue = 36;
const std::string AQUDIMS::NANAQU::itemName = "NANAQU";
const int AQUDIMS::NANAQU::defaultValue = 1;
const std::string AQUDIMS::NCAMAX::itemName = "NCAMAX";
const int AQUDIMS::NCAMAX::defaultValue = 1;
const std::string AQUDIMS::MXNALI::itemName = "MXNALI";
const int AQUDIMS::MXNALI::defaultValue = 0;
const std::string AQUDIMS::MXAAQL::itemName = "MXAAQL";
const int AQUDIMS::MXAAQL::defaultValue = 0;


AQUFET::AQUFET( ) : ParserKeyword("AQUFET")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("AQUDIMS","NANAQU",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUFET");
  {
     ParserRecord record;
     {
        ParserItem item("DAT_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("P0", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("V0", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("C_T", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PI", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Pressure*Time");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM_WATER_PRESS", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_K", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_K", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_INDX", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SC_0", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUFET::keywordName = "AQUFET";
const std::string AQUFET::DAT_DEPTH::itemName = "DAT_DEPTH";
const std::string AQUFET::P0::itemName = "P0";
const std::string AQUFET::V0::itemName = "V0";
const std::string AQUFET::C_T::itemName = "C_T";
const std::string AQUFET::PI::itemName = "PI";
const std::string AQUFET::TABLE_NUM_WATER_PRESS::itemName = "TABLE_NUM_WATER_PRESS";
const std::string AQUFET::LOWER_I::itemName = "LOWER_I";
const std::string AQUFET::UPPER_I::itemName = "UPPER_I";
const std::string AQUFET::LOWER_J::itemName = "LOWER_J";
const std::string AQUFET::UPPER_J::itemName = "UPPER_J";
const std::string AQUFET::LOWER_K::itemName = "LOWER_K";
const std::string AQUFET::UPPER_K::itemName = "UPPER_K";
const std::string AQUFET::FACE_INDX::itemName = "FACE_INDX";
const std::string AQUFET::SC_0::itemName = "SC_0";
const double AQUFET::SC_0::defaultValue = 0.000000;


AQUFETP::AQUFETP( ) : ParserKeyword("AQUFETP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUFETP");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DAT_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("P0", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("V0", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("C_T", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PI", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Pressure*Time");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM_WATER_PRESS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("SALINITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUFETP::keywordName = "AQUFETP";
const std::string AQUFETP::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUFETP::DAT_DEPTH::itemName = "DAT_DEPTH";
const std::string AQUFETP::P0::itemName = "P0";
const std::string AQUFETP::V0::itemName = "V0";
const std::string AQUFETP::C_T::itemName = "C_T";
const std::string AQUFETP::PI::itemName = "PI";
const std::string AQUFETP::TABLE_NUM_WATER_PRESS::itemName = "TABLE_NUM_WATER_PRESS";
const int AQUFETP::TABLE_NUM_WATER_PRESS::defaultValue = 1;
const std::string AQUFETP::SALINITY::itemName = "SALINITY";
const double AQUFETP::SALINITY::defaultValue = 0.000000;
const std::string AQUFETP::TEMP::itemName = "TEMP";


AQUFLUX::AQUFLUX( ) : ParserKeyword("AQUFLUX")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("AQUDIMS","NANAQU",0);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("AQUFLUX");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DAT_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SC_0", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUFLUX::keywordName = "AQUFLUX";
const std::string AQUFLUX::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUFLUX::DAT_DEPTH::itemName = "DAT_DEPTH";
const std::string AQUFLUX::SC_0::itemName = "SC_0";
const double AQUFLUX::SC_0::defaultValue = 0.000000;
const std::string AQUFLUX::TEMP::itemName = "TEMP";
const std::string AQUFLUX::PRESSURE::itemName = "PRESSURE";


AQUIFER_PROBE_ANALYTIC::AQUIFER_PROBE_ANALYTIC( ) : ParserKeyword("AQUIFER_PROBE_ANALYTIC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("AAQP");
  addDeckName("AAQPD");
  addDeckName("AAQR");
  addDeckName("AAQRG");
  addDeckName("AAQT");
  addDeckName("AAQTD");
  addDeckName("AAQTG");
  setMatchRegex("AA.+");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string AQUIFER_PROBE_ANALYTIC::keywordName = "AQUIFER_PROBE_ANALYTIC";
const std::string AQUIFER_PROBE_ANALYTIC::data::itemName = "data";


AQUIFER_PROBE_NUMERIC::AQUIFER_PROBE_NUMERIC( ) : ParserKeyword("AQUIFER_PROBE_NUMERIC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ANQP");
  addDeckName("ANQR");
  addDeckName("ANQT");
  setMatchRegex("ANQ.");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string AQUIFER_PROBE_NUMERIC::keywordName = "AQUIFER_PROBE_NUMERIC";
const std::string AQUIFER_PROBE_NUMERIC::data::itemName = "data";


AQUNNC::AQUNNC( ) : ParserKeyword("AQUNNC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AQUNNC");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRAN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     {
        ParserItem item("IST1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IST2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IPT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IPT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ZF1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ZF2", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUNNC::keywordName = "AQUNNC";
const std::string AQUNNC::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUNNC::IX::itemName = "IX";
const std::string AQUNNC::IY::itemName = "IY";
const std::string AQUNNC::IZ::itemName = "IZ";
const std::string AQUNNC::JX::itemName = "JX";
const std::string AQUNNC::JY::itemName = "JY";
const std::string AQUNNC::JZ::itemName = "JZ";
const std::string AQUNNC::TRAN::itemName = "TRAN";
const double AQUNNC::TRAN::defaultValue = 0.000000;
const std::string AQUNNC::IST1::itemName = "IST1";
const std::string AQUNNC::IST2::itemName = "IST2";
const std::string AQUNNC::IPT1::itemName = "IPT1";
const std::string AQUNNC::IPT2::itemName = "IPT2";
const std::string AQUNNC::ZF1::itemName = "ZF1";
const std::string AQUNNC::ZF2::itemName = "ZF2";
const std::string AQUNNC::DIFF::itemName = "DIFF";


AQUNUM::AQUNUM( ) : ParserKeyword("AQUNUM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AQUNUM");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CROSS_SECTION", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PORO", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PERM", ParserItem::itype::DOUBLE);
        item.push_backDimension("Permeability");
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INITIAL_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PVT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUNUM::keywordName = "AQUNUM";
const std::string AQUNUM::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUNUM::I::itemName = "I";
const std::string AQUNUM::J::itemName = "J";
const std::string AQUNUM::K::itemName = "K";
const std::string AQUNUM::CROSS_SECTION::itemName = "CROSS_SECTION";
const std::string AQUNUM::LENGTH::itemName = "LENGTH";
const std::string AQUNUM::PORO::itemName = "PORO";
const std::string AQUNUM::PERM::itemName = "PERM";
const std::string AQUNUM::DEPTH::itemName = "DEPTH";
const std::string AQUNUM::INITIAL_PRESSURE::itemName = "INITIAL_PRESSURE";
const std::string AQUNUM::PVT_TABLE_NUM::itemName = "PVT_TABLE_NUM";
const std::string AQUNUM::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";


AQUTAB::AQUTAB( ) : ParserKeyword("AQUTAB")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("AQUDIMS","NIFTBL",-1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("AQUTAB");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUTAB::keywordName = "AQUTAB";
const std::string AQUTAB::DATA::itemName = "DATA";


AUTOCOAR::AUTOCOAR( ) : ParserKeyword("AUTOCOAR")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AUTOCOAR");
  {
     ParserRecord record;
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
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
const std::string AUTOCOAR::keywordName = "AUTOCOAR";
const std::string AUTOCOAR::I1::itemName = "I1";
const std::string AUTOCOAR::I2::itemName = "I2";
const std::string AUTOCOAR::J1::itemName = "J1";
const std::string AUTOCOAR::J2::itemName = "J2";
const std::string AUTOCOAR::K1::itemName = "K1";
const std::string AUTOCOAR::K2::itemName = "K2";
const std::string AUTOCOAR::NX::itemName = "NX";
const std::string AUTOCOAR::NY::itemName = "NY";
const std::string AUTOCOAR::NZ::itemName = "NZ";


AUTOREF::AUTOREF( ) : ParserKeyword("AUTOREF")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("AUTOREF");
  {
     ParserRecord record;
     {
        ParserItem item("NX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NY", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NZ", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("OPTION_TRANS_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string AUTOREF::keywordName = "AUTOREF";
const std::string AUTOREF::NX::itemName = "NX";
const int AUTOREF::NX::defaultValue = 1;
const std::string AUTOREF::NY::itemName = "NY";
const int AUTOREF::NY::defaultValue = 1;
const std::string AUTOREF::NZ::itemName = "NZ";
const int AUTOREF::NZ::defaultValue = 1;
const std::string AUTOREF::OPTION_TRANS_MULT::itemName = "OPTION_TRANS_MULT";
const double AUTOREF::OPTION_TRANS_MULT::defaultValue = 0.000000;


}
}
