#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/M.hpp>
namespace Opm {
namespace ParserKeywords {
MAPAXES::MAPAXES( ) : ParserKeyword("MAPAXES")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MAPAXES");
  {
     ParserRecord record;
     {
        ParserItem item("X1", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("Y1", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("X2", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("Y2", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("X3", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("Y3", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MAPAXES::keywordName = "MAPAXES";
const std::string MAPAXES::X1::itemName = "X1";
const std::string MAPAXES::Y1::itemName = "Y1";
const std::string MAPAXES::X2::itemName = "X2";
const std::string MAPAXES::Y2::itemName = "Y2";
const std::string MAPAXES::X3::itemName = "X3";
const std::string MAPAXES::Y3::itemName = "Y3";


MAPUNITS::MAPUNITS( ) : ParserKeyword("MAPUNITS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MAPUNITS");
  {
     ParserRecord record;
     {
        ParserItem item("UNIT", ParserItem::itype::STRING);
        item.setDefault( std::string("METRES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MAPUNITS::keywordName = "MAPUNITS";
const std::string MAPUNITS::UNIT::itemName = "UNIT";
const std::string MAPUNITS::UNIT::defaultValue = "METRES";


MASSFLOW::MASSFLOW( ) : ParserKeyword("MASSFLOW")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MASSFLOW");
  {
     ParserRecord record;
     {
        ParserItem item("WORD", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MASSFLOW::keywordName = "MASSFLOW";
const std::string MASSFLOW::WORD::itemName = "WORD";


MATCORR::MATCORR( ) : ParserKeyword("MATCORR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MATCORR");
  {
     ParserRecord record;
     {
        ParserItem item("NEWTON_IT_NUM", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("NON_LIN_CONV_ERR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MATERIAL_BALANCE_ERR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MATCORR::keywordName = "MATCORR";
const std::string MATCORR::NEWTON_IT_NUM::itemName = "NEWTON_IT_NUM";
const int MATCORR::NEWTON_IT_NUM::defaultValue = 12;
const std::string MATCORR::NON_LIN_CONV_ERR::itemName = "NON_LIN_CONV_ERR";
const double MATCORR::NON_LIN_CONV_ERR::defaultValue = 0.010000;
const std::string MATCORR::MATERIAL_BALANCE_ERR::itemName = "MATERIAL_BALANCE_ERR";
const double MATCORR::MATERIAL_BALANCE_ERR::defaultValue = 0.000001;


MAXVALUE::MAXVALUE( ) : ParserKeyword("MAXVALUE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MAXVALUE");
  {
     ParserRecord record;
     {
        ParserItem item("field", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("value", ParserItem::itype::DOUBLE);
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
const std::string MAXVALUE::keywordName = "MAXVALUE";
const std::string MAXVALUE::field::itemName = "field";
const std::string MAXVALUE::value::itemName = "value";
const std::string MAXVALUE::I1::itemName = "I1";
const std::string MAXVALUE::I2::itemName = "I2";
const std::string MAXVALUE::J1::itemName = "J1";
const std::string MAXVALUE::J2::itemName = "J2";
const std::string MAXVALUE::K1::itemName = "K1";
const std::string MAXVALUE::K2::itemName = "K2";


MEMORY::MEMORY( ) : ParserKeyword("MEMORY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MEMORY");
  {
     ParserRecord record;
     {
        ParserItem item("UNUSED", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("THOUSANDS_CHAR8", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MEMORY::keywordName = "MEMORY";
const std::string MEMORY::UNUSED::itemName = "UNUSED";
const std::string MEMORY::THOUSANDS_CHAR8::itemName = "THOUSANDS_CHAR8";


MESSAGE::MESSAGE( ) : ParserKeyword("MESSAGE")
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
  addDeckName("MESSAGE");
  {
     ParserRecord record;
     {
        ParserItem item("MessageText", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MESSAGE::keywordName = "MESSAGE";
const std::string MESSAGE::MessageText::itemName = "MessageText";


MESSAGES::MESSAGES( ) : ParserKeyword("MESSAGES")
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
  addDeckName("MESSAGES");
  {
     ParserRecord record;
     {
        ParserItem item("MESSAGE_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 1000000 );
        record.addItem(item);
     }
     {
        ParserItem item("COMMENT_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 1000000 );
        record.addItem(item);
     }
     {
        ParserItem item("WARNING_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 10000 );
        record.addItem(item);
     }
     {
        ParserItem item("PROBLEM_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     {
        ParserItem item("ERROR_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     {
        ParserItem item("BUG_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     {
        ParserItem item("MESSAGE_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 1000000 );
        record.addItem(item);
     }
     {
        ParserItem item("COMMENT_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 1000000 );
        record.addItem(item);
     }
     {
        ParserItem item("WARNING_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 10000 );
        record.addItem(item);
     }
     {
        ParserItem item("PROBLEM_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     {
        ParserItem item("ERROR_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("BUG_STOP_LIMIT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_PRINT_LIMIT", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MESSAGES::keywordName = "MESSAGES";
const std::string MESSAGES::MESSAGE_PRINT_LIMIT::itemName = "MESSAGE_PRINT_LIMIT";
const int MESSAGES::MESSAGE_PRINT_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::COMMENT_PRINT_LIMIT::itemName = "COMMENT_PRINT_LIMIT";
const int MESSAGES::COMMENT_PRINT_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::WARNING_PRINT_LIMIT::itemName = "WARNING_PRINT_LIMIT";
const int MESSAGES::WARNING_PRINT_LIMIT::defaultValue = 10000;
const std::string MESSAGES::PROBLEM_PRINT_LIMIT::itemName = "PROBLEM_PRINT_LIMIT";
const int MESSAGES::PROBLEM_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::ERROR_PRINT_LIMIT::itemName = "ERROR_PRINT_LIMIT";
const int MESSAGES::ERROR_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::BUG_PRINT_LIMIT::itemName = "BUG_PRINT_LIMIT";
const int MESSAGES::BUG_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::MESSAGE_STOP_LIMIT::itemName = "MESSAGE_STOP_LIMIT";
const int MESSAGES::MESSAGE_STOP_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::COMMENT_STOP_LIMIT::itemName = "COMMENT_STOP_LIMIT";
const int MESSAGES::COMMENT_STOP_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::WARNING_STOP_LIMIT::itemName = "WARNING_STOP_LIMIT";
const int MESSAGES::WARNING_STOP_LIMIT::defaultValue = 10000;
const std::string MESSAGES::PROBLEM_STOP_LIMIT::itemName = "PROBLEM_STOP_LIMIT";
const int MESSAGES::PROBLEM_STOP_LIMIT::defaultValue = 100;
const std::string MESSAGES::ERROR_STOP_LIMIT::itemName = "ERROR_STOP_LIMIT";
const int MESSAGES::ERROR_STOP_LIMIT::defaultValue = 10;
const std::string MESSAGES::BUG_STOP_LIMIT::itemName = "BUG_STOP_LIMIT";
const int MESSAGES::BUG_STOP_LIMIT::defaultValue = 1;
const std::string MESSAGES::GROUP_PRINT_LIMIT::itemName = "GROUP_PRINT_LIMIT";
const int MESSAGES::GROUP_PRINT_LIMIT::defaultValue = 10;


MESSOPTS::MESSOPTS( ) : ParserKeyword("MESSOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MESSOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("MNEMONIC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEVERITY", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MESSOPTS::keywordName = "MESSOPTS";
const std::string MESSOPTS::MNEMONIC::itemName = "MNEMONIC";
const std::string MESSOPTS::SEVERITY::itemName = "SEVERITY";


MESSSRVC::MESSSRVC( ) : ParserKeyword("MESSSRVC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MESSSRVC");
  {
     ParserRecord record;
     {
        ParserItem item("PRODUCE_MESSAGE", ParserItem::itype::STRING);
        item.setDefault( std::string("ON") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MESSSRVC::keywordName = "MESSSRVC";
const std::string MESSSRVC::PRODUCE_MESSAGE::itemName = "PRODUCE_MESSAGE";
const std::string MESSSRVC::PRODUCE_MESSAGE::defaultValue = "ON";


METRIC::METRIC( ) : ParserKeyword("METRIC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("METRIC");
}
const std::string METRIC::keywordName = "METRIC";


MINNNCT::MINNNCT( ) : ParserKeyword("MINNNCT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINNNCT");
  {
     ParserRecord record;
     {
        ParserItem item("CUTOFF_TRANSMISSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     {
        ParserItem item("DIFFUSIVITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("CUTOFF_THERMAL_TRANSMISSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Energy/AbsoluteTemperature*Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINNNCT::keywordName = "MINNNCT";
const std::string MINNNCT::CUTOFF_TRANSMISSIBILITY::itemName = "CUTOFF_TRANSMISSIBILITY";
const double MINNNCT::CUTOFF_TRANSMISSIBILITY::defaultValue = 0.000000;
const std::string MINNNCT::DIFFUSIVITY::itemName = "DIFFUSIVITY";
const double MINNNCT::DIFFUSIVITY::defaultValue = 0.000000;
const std::string MINNNCT::CUTOFF_THERMAL_TRANSMISSIBILITY::itemName = "CUTOFF_THERMAL_TRANSMISSIBILITY";
const double MINNNCT::CUTOFF_THERMAL_TRANSMISSIBILITY::defaultValue = 0.000000;


MINPORV::MINPORV( ) : ParserKeyword("MINPORV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPORV");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_PORE_VOL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("ReservoirVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINPORV::keywordName = "MINPORV";
const std::string MINPORV::MIN_PORE_VOL::itemName = "MIN_PORE_VOL";
const double MINPORV::MIN_PORE_VOL::defaultValue = 0.000001;


MINPV::MINPV( ) : ParserKeyword("MINPV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPV");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("ReservoirVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINPV::keywordName = "MINPV";
const std::string MINPV::VALUE::itemName = "VALUE";
const double MINPV::VALUE::defaultValue = 0.000001;


MINPVFIL::MINPVFIL( ) : ParserKeyword("MINPVFIL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPVFIL");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("ReservoirVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINPVFIL::keywordName = "MINPVFIL";
const std::string MINPVFIL::VALUE::itemName = "VALUE";
const double MINPVFIL::VALUE::defaultValue = 0.000001;


MINPVV::MINPVV( ) : ParserKeyword("MINPVV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPVV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(1e-06) );
        item.push_backDimension("ReservoirVolume");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MINPVV::keywordName = "MINPVV";
const std::string MINPVV::data::itemName = "data";
const double MINPVV::data::defaultValue = 0.000001;


MINVALUE::MINVALUE( ) : ParserKeyword("MINVALUE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MINVALUE");
  {
     ParserRecord record;
     {
        ParserItem item("field", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("value", ParserItem::itype::DOUBLE);
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
const std::string MINVALUE::keywordName = "MINVALUE";
const std::string MINVALUE::field::itemName = "field";
const std::string MINVALUE::value::itemName = "value";
const std::string MINVALUE::I1::itemName = "I1";
const std::string MINVALUE::I2::itemName = "I2";
const std::string MINVALUE::J1::itemName = "J1";
const std::string MINVALUE::J2::itemName = "J2";
const std::string MINVALUE::K1::itemName = "K1";
const std::string MINVALUE::K2::itemName = "K2";


MISC::MISC( ) : ParserKeyword("MISC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MISC");
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
const std::string MISC::keywordName = "MISC";
const std::string MISC::DATA::itemName = "DATA";


MISCIBLE::MISCIBLE( ) : ParserKeyword("MISCIBLE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MISCIBLE");
  {
     ParserRecord record;
     {
        ParserItem item("NTMISC", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NSMISC", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("TWOPOINT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MISCIBLE::keywordName = "MISCIBLE";
const std::string MISCIBLE::NTMISC::itemName = "NTMISC";
const int MISCIBLE::NTMISC::defaultValue = 1;
const std::string MISCIBLE::NSMISC::itemName = "NSMISC";
const int MISCIBLE::NSMISC::defaultValue = 20;
const std::string MISCIBLE::TWOPOINT::itemName = "TWOPOINT";
const std::string MISCIBLE::TWOPOINT::defaultValue = "NONE";


MISCNUM::MISCNUM( ) : ParserKeyword("MISCNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("MISCNUM");
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
const std::string MISCNUM::keywordName = "MISCNUM";
const std::string MISCNUM::data::itemName = "data";


MLANG::MLANG( ) : ParserKeyword("MLANG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MLANG");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MLANG::keywordName = "MLANG";
const std::string MLANG::data::itemName = "data";


MLANGSLV::MLANGSLV( ) : ParserKeyword("MLANGSLV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MLANGSLV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MLANGSLV::keywordName = "MLANGSLV";
const std::string MLANGSLV::data::itemName = "data";


MONITOR::MONITOR( ) : ParserKeyword("MONITOR")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("MONITOR");
}
const std::string MONITOR::keywordName = "MONITOR";


MPFANUM::MPFANUM( ) : ParserKeyword("MPFANUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MPFANUM");
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
const std::string MPFANUM::keywordName = "MPFANUM";
const std::string MPFANUM::data::itemName = "data";


MPFNNC::MPFNNC( ) : ParserKeyword("MPFNNC")
{
  setSizeType(DOUBLE_SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MPFNNC");
  setDoubleRecordsKeyword(true);
  {
     ParserRecord record;
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
        ParserItem item("TRANP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("KX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRANS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MPFNNC::keywordName = "MPFNNC";
const std::string MPFNNC::IX::itemName = "IX";
const std::string MPFNNC::IY::itemName = "IY";
const std::string MPFNNC::IZ::itemName = "IZ";
const std::string MPFNNC::JX::itemName = "JX";
const std::string MPFNNC::JY::itemName = "JY";
const std::string MPFNNC::JZ::itemName = "JZ";
const std::string MPFNNC::TRANP::itemName = "TRANP";
const std::string MPFNNC::KX::itemName = "KX";
const std::string MPFNNC::KY::itemName = "KY";
const std::string MPFNNC::KZ::itemName = "KZ";
const std::string MPFNNC::TRANS::itemName = "TRANS";


MSFN::MSFN( ) : ParserKeyword("MSFN")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MSFN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MSFN::keywordName = "MSFN";
const std::string MSFN::DATA::itemName = "DATA";


MSGFILE::MSGFILE( ) : ParserKeyword("MSGFILE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MSGFILE");
  {
     ParserRecord record;
     {
        ParserItem item("ENABLE_FLAG", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MSGFILE::keywordName = "MSGFILE";
const std::string MSGFILE::ENABLE_FLAG::itemName = "ENABLE_FLAG";


MULSGGD::MULSGGD( ) : ParserKeyword("MULSGGD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULSGGD");
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
const std::string MULSGGD::keywordName = "MULSGGD";
const std::string MULSGGD::data::itemName = "data";


MULSGGDV::MULSGGDV( ) : ParserKeyword("MULSGGDV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULSGGDV");
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
const std::string MULSGGDV::keywordName = "MULSGGDV";
const std::string MULSGGDV::data::itemName = "data";


MULTFLT::MULTFLT( ) : ParserKeyword("MULTFLT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTFLT");
  {
     ParserRecord record;
     {
        ParserItem item("fault", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("factor", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTFLT::keywordName = "MULTFLT";
const std::string MULTFLT::fault::itemName = "fault";
const std::string MULTFLT::factor::itemName = "factor";


MULTIN::MULTIN( ) : ParserKeyword("MULTIN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MULTIN");
}
const std::string MULTIN::keywordName = "MULTIN";


MULTIPLY::MULTIPLY( ) : ParserKeyword("MULTIPLY")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("MULTIPLY");
  {
     ParserRecord record;
     {
        ParserItem item("field", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("factor", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
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
const std::string MULTIPLY::keywordName = "MULTIPLY";
const std::string MULTIPLY::field::itemName = "field";
const std::string MULTIPLY::factor::itemName = "factor";
const std::string MULTIPLY::I1::itemName = "I1";
const std::string MULTIPLY::I2::itemName = "I2";
const std::string MULTIPLY::J1::itemName = "J1";
const std::string MULTIPLY::J2::itemName = "J2";
const std::string MULTIPLY::K1::itemName = "K1";
const std::string MULTIPLY::K2::itemName = "K2";


MULTIREG::MULTIREG( ) : ParserKeyword("MULTIREG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("MULTIREG");
  {
     ParserRecord record;
     {
        ParserItem item("ARRAY", ParserItem::itype::STRING);
        item.setDescription("The 3D array we will update");
        record.addItem(item);
     }
     {
        ParserItem item("FACTOR", ParserItem::itype::DOUBLE);
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
const std::string MULTIREG::keywordName = "MULTIREG";
const std::string MULTIREG::ARRAY::itemName = "ARRAY";
const std::string MULTIREG::FACTOR::itemName = "FACTOR";
const double MULTIREG::FACTOR::defaultValue = 0.000000;
const std::string MULTIREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string MULTIREG::REGION_NAME::itemName = "REGION_NAME";
const std::string MULTIREG::REGION_NAME::defaultValue = "M";


MULTNUM::MULTNUM( ) : ParserKeyword("MULTNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTNUM");
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
const std::string MULTNUM::keywordName = "MULTNUM";
const std::string MULTNUM::data::itemName = "data";


MULTOUT::MULTOUT( ) : ParserKeyword("MULTOUT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MULTOUT");
}
const std::string MULTOUT::keywordName = "MULTOUT";


MULTOUTS::MULTOUTS( ) : ParserKeyword("MULTOUTS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MULTOUTS");
}
const std::string MULTOUTS::keywordName = "MULTOUTS";


MULTPV::MULTPV( ) : ParserKeyword("MULTPV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTPV");
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
const std::string MULTPV::keywordName = "MULTPV";
const std::string MULTPV::data::itemName = "data";


MULTREAL::MULTREAL( ) : ParserKeyword("MULTREAL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MULTREAL");
  {
     ParserRecord record;
     {
        ParserItem item("SESSION_SPEC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STANDARD_LICENCE", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREAL::keywordName = "MULTREAL";
const std::string MULTREAL::SESSION_SPEC::itemName = "SESSION_SPEC";
const std::string MULTREAL::STANDARD_LICENCE::itemName = "STANDARD_LICENCE";
const std::string MULTREAL::STANDARD_LICENCE::defaultValue = "YES";


MULTREGD::MULTREGD( ) : ParserKeyword("MULTREGD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTREGD");
  {
     ParserRecord record;
     {
        ParserItem item("FROM_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TO_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MULTIPLIER", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("XYZ") );
        record.addItem(item);
     }
     {
        ParserItem item("FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     {
        ParserItem item("CHOICE", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREGD::keywordName = "MULTREGD";
const std::string MULTREGD::FROM_REGION::itemName = "FROM_REGION";
const std::string MULTREGD::TO_REGION::itemName = "TO_REGION";
const std::string MULTREGD::MULTIPLIER::itemName = "MULTIPLIER";
const std::string MULTREGD::DIRECTION::itemName = "DIRECTION";
const std::string MULTREGD::DIRECTION::defaultValue = "XYZ";
const std::string MULTREGD::FLAG::itemName = "FLAG";
const std::string MULTREGD::FLAG::defaultValue = "ALL";
const std::string MULTREGD::CHOICE::itemName = "CHOICE";
const std::string MULTREGD::CHOICE::defaultValue = "M";


MULTREGH::MULTREGH( ) : ParserKeyword("MULTREGH")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTREGH");
  {
     ParserRecord record;
     {
        ParserItem item("FROM_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TO_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MULTIPLIER", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("XYZ") );
        record.addItem(item);
     }
     {
        ParserItem item("FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     {
        ParserItem item("CHOICE", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREGH::keywordName = "MULTREGH";
const std::string MULTREGH::FROM_REGION::itemName = "FROM_REGION";
const std::string MULTREGH::TO_REGION::itemName = "TO_REGION";
const std::string MULTREGH::MULTIPLIER::itemName = "MULTIPLIER";
const std::string MULTREGH::DIRECTION::itemName = "DIRECTION";
const std::string MULTREGH::DIRECTION::defaultValue = "XYZ";
const std::string MULTREGH::FLAG::itemName = "FLAG";
const std::string MULTREGH::FLAG::defaultValue = "ALL";
const std::string MULTREGH::CHOICE::itemName = "CHOICE";
const std::string MULTREGH::CHOICE::defaultValue = "M";


MULTREGP::MULTREGP( ) : ParserKeyword("MULTREGP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTREGP");
  {
     ParserRecord record;
     {
        ParserItem item("REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MULTIPLIER", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REGION_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREGP::keywordName = "MULTREGP";
const std::string MULTREGP::REGION::itemName = "REGION";
const std::string MULTREGP::MULTIPLIER::itemName = "MULTIPLIER";
const std::string MULTREGP::REGION_TYPE::itemName = "REGION_TYPE";
const std::string MULTREGP::REGION_TYPE::defaultValue = "M";


MULTREGT::MULTREGT( ) : ParserKeyword("MULTREGT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTREGT");
  {
     ParserRecord record;
     {
        ParserItem item("SRC_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TARGET_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRAN_MULT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTIONS", ParserItem::itype::STRING);
        item.setDefault( std::string("XYZ") );
        record.addItem(item);
     }
     {
        ParserItem item("NNC_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     {
        ParserItem item("REGION_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREGT::keywordName = "MULTREGT";
const std::string MULTREGT::SRC_REGION::itemName = "SRC_REGION";
const std::string MULTREGT::TARGET_REGION::itemName = "TARGET_REGION";
const std::string MULTREGT::TRAN_MULT::itemName = "TRAN_MULT";
const std::string MULTREGT::DIRECTIONS::itemName = "DIRECTIONS";
const std::string MULTREGT::DIRECTIONS::defaultValue = "XYZ";
const std::string MULTREGT::NNC_MULT::itemName = "NNC_MULT";
const std::string MULTREGT::NNC_MULT::defaultValue = "ALL";
const std::string MULTREGT::REGION_DEF::itemName = "REGION_DEF";
const std::string MULTREGT::REGION_DEF::defaultValue = "M";


MULTSIG::MULTSIG( ) : ParserKeyword("MULTSIG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTSIG");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTSIG::keywordName = "MULTSIG";
const std::string MULTSIG::VALUE::itemName = "VALUE";


MULTSIGV::MULTSIGV( ) : ParserKeyword("MULTSIGV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTSIGV");
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
const std::string MULTSIGV::keywordName = "MULTSIGV";
const std::string MULTSIGV::data::itemName = "data";


MULT_XYZ::MULT_XYZ( ) : ParserKeyword("MULT_XYZ")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTR");
  addDeckName("MULTR-");
  addDeckName("MULTTHT");
  addDeckName("MULTTHT-");
  addDeckName("MULTX");
  addDeckName("MULTX-");
  addDeckName("MULTY");
  addDeckName("MULTY-");
  addDeckName("MULTZ");
  addDeckName("MULTZ-");
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
const std::string MULT_XYZ::keywordName = "MULT_XYZ";
const std::string MULT_XYZ::data::itemName = "data";


MW::MW( ) : ParserKeyword("MW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MW");
  {
     ParserRecord record;
     {
        ParserItem item("MOLAR_WEIGHT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MW::keywordName = "MW";
const std::string MW::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


MWS::MWS( ) : ParserKeyword("MWS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MWS");
  {
     ParserRecord record;
     {
        ParserItem item("MOLAR_WEIGHT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string MWS::keywordName = "MWS";
const std::string MWS::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


}
}
