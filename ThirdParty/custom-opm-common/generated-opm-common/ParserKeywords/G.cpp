#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/G.hpp>
namespace Opm {
namespace ParserKeywords {
GAS::GAS( ) : ParserKeyword("GAS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GAS");
}
const std::string GAS::keywordName = "GAS";


GASBEGIN::GASBEGIN( ) : ParserKeyword("GASBEGIN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASBEGIN");
}
const std::string GASBEGIN::keywordName = "GASBEGIN";


GASCONC::GASCONC( ) : ParserKeyword("GASCONC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GASCONC");
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
const std::string GASCONC::keywordName = "GASCONC";
const std::string GASCONC::data::itemName = "data";


GASDENT::GASDENT( ) : ParserKeyword("GASDENT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GASDENT");
  {
     ParserRecord record;
     {
        ParserItem item("REFERENCE_TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(293.150000) );
        item.push_backDimension("AbsoluteTemperature");
        record.addItem(item);
     }
     {
        ParserItem item("EXPANSION_COEFF_LINEAR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1/AbsoluteTemperature");
        record.addItem(item);
     }
     {
        ParserItem item("EXPANSION_COEFF_QUADRATIC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1/AbsoluteTemperature*AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASDENT::keywordName = "GASDENT";
const std::string GASDENT::REFERENCE_TEMPERATURE::itemName = "REFERENCE_TEMPERATURE";
const double GASDENT::REFERENCE_TEMPERATURE::defaultValue = 293.150000;
const std::string GASDENT::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const double GASDENT::EXPANSION_COEFF_LINEAR::defaultValue = 0.000000;
const std::string GASDENT::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";
const double GASDENT::EXPANSION_COEFF_QUADRATIC::defaultValue = 0.000000;


GASEND::GASEND( ) : ParserKeyword("GASEND")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASEND");
}
const std::string GASEND::keywordName = "GASEND";


GASFCOMP::GASFCOMP( ) : ParserKeyword("GASFCOMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASFCOMP");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ARTFICIAL_LIFT_QNTY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_CONSUMPTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("COMPRESSION_LVL", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("ACTION_SEQ_NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASFCOMP::keywordName = "GASFCOMP";
const std::string GASFCOMP::GROUP::itemName = "GROUP";
const std::string GASFCOMP::VFP_TABLE_NUM::itemName = "VFP_TABLE_NUM";
const int GASFCOMP::VFP_TABLE_NUM::defaultValue = 0;
const std::string GASFCOMP::ARTFICIAL_LIFT_QNTY::itemName = "ARTFICIAL_LIFT_QNTY";
const double GASFCOMP::ARTFICIAL_LIFT_QNTY::defaultValue = 0.000000;
const std::string GASFCOMP::GAS_CONSUMPTION_RATE::itemName = "GAS_CONSUMPTION_RATE";
const double GASFCOMP::GAS_CONSUMPTION_RATE::defaultValue = 0.000000;
const std::string GASFCOMP::COMPRESSION_LVL::itemName = "COMPRESSION_LVL";
const int GASFCOMP::COMPRESSION_LVL::defaultValue = 1;
const std::string GASFCOMP::ACTION_SEQ_NUM::itemName = "ACTION_SEQ_NUM";
const int GASFCOMP::ACTION_SEQ_NUM::defaultValue = 1;


GASFDECR::GASFDECR( ) : ParserKeyword("GASFDECR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASFDECR");
  {
     ParserRecord record;
     {
        ParserItem item("JAN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("FEB", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("APR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("JUN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("JUL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("AUG", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SEP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("OCT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("NOV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("DEC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASFDECR::keywordName = "GASFDECR";
const std::string GASFDECR::JAN::itemName = "JAN";
const double GASFDECR::JAN::defaultValue = 0.000000;
const std::string GASFDECR::FEB::itemName = "FEB";
const double GASFDECR::FEB::defaultValue = 0.000000;
const std::string GASFDECR::MAR::itemName = "MAR";
const double GASFDECR::MAR::defaultValue = 0.000000;
const std::string GASFDECR::APR::itemName = "APR";
const double GASFDECR::APR::defaultValue = 0.000000;
const std::string GASFDECR::MAY::itemName = "MAY";
const double GASFDECR::MAY::defaultValue = 0.000000;
const std::string GASFDECR::JUN::itemName = "JUN";
const double GASFDECR::JUN::defaultValue = 0.000000;
const std::string GASFDECR::JUL::itemName = "JUL";
const double GASFDECR::JUL::defaultValue = 0.000000;
const std::string GASFDECR::AUG::itemName = "AUG";
const double GASFDECR::AUG::defaultValue = 0.000000;
const std::string GASFDECR::SEP::itemName = "SEP";
const double GASFDECR::SEP::defaultValue = 0.000000;
const std::string GASFDECR::OCT::itemName = "OCT";
const double GASFDECR::OCT::defaultValue = 0.000000;
const std::string GASFDECR::NOV::itemName = "NOV";
const double GASFDECR::NOV::defaultValue = 0.000000;
const std::string GASFDECR::DEC::itemName = "DEC";
const double GASFDECR::DEC::defaultValue = 0.000000;


GASFDELC::GASFDELC( ) : ParserKeyword("GASFDELC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASFDELC");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASFDELC::keywordName = "GASFDELC";
const std::string GASFDELC::VALUE::itemName = "VALUE";


GASFIELD::GASFIELD( ) : ParserKeyword("GASFIELD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GASFIELD");
  {
     ParserRecord record;
     {
        ParserItem item("FLAG_COMP", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("FLAG_IT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASFIELD::keywordName = "GASFIELD";
const std::string GASFIELD::FLAG_COMP::itemName = "FLAG_COMP";
const std::string GASFIELD::FLAG_COMP::defaultValue = "NO";
const std::string GASFIELD::FLAG_IT::itemName = "FLAG_IT";
const std::string GASFIELD::FLAG_IT::defaultValue = "NO";


GASFTARG::GASFTARG( ) : ParserKeyword("GASFTARG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASFTARG");
  {
     ParserRecord record;
     {
        ParserItem item("JAN", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("FEB", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("APR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("JUN", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("JUL", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("AUG", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("SEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("OCT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("NOV", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("DEC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASFTARG::keywordName = "GASFTARG";
const std::string GASFTARG::JAN::itemName = "JAN";
const std::string GASFTARG::FEB::itemName = "FEB";
const std::string GASFTARG::MAR::itemName = "MAR";
const std::string GASFTARG::APR::itemName = "APR";
const std::string GASFTARG::MAY::itemName = "MAY";
const std::string GASFTARG::JUN::itemName = "JUN";
const std::string GASFTARG::JUL::itemName = "JUL";
const std::string GASFTARG::AUG::itemName = "AUG";
const std::string GASFTARG::SEP::itemName = "SEP";
const std::string GASFTARG::OCT::itemName = "OCT";
const std::string GASFTARG::NOV::itemName = "NOV";
const std::string GASFTARG::DEC::itemName = "DEC";


GASMONTH::GASMONTH( ) : ParserKeyword("GASMONTH")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASMONTH");
  {
     ParserRecord record;
     {
        ParserItem item("MONTH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WRITE_REPORT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASMONTH::keywordName = "GASMONTH";
const std::string GASMONTH::MONTH::itemName = "MONTH";
const std::string GASMONTH::WRITE_REPORT::itemName = "WRITE_REPORT";
const std::string GASMONTH::WRITE_REPORT::defaultValue = "NO";


GASPERIO::GASPERIO( ) : ParserKeyword("GASPERIO")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASPERIO");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_PERIODS", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_MONTHS", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("INITIAL_DCQ", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_REQ", ParserItem::itype::STRING);
        item.setDefault( std::string("PER") );
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_TIMESTEPS", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_DCQ_RED_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("ANTICIPATED_DCQ_RED_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITERATIONS", ParserItem::itype::INT);
        item.setDefault( 3 );
        record.addItem(item);
     }
     {
        ParserItem item("DCQ_CONV_TOLERANCE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASPERIO::keywordName = "GASPERIO";
const std::string GASPERIO::NUM_PERIODS::itemName = "NUM_PERIODS";
const std::string GASPERIO::NUM_MONTHS::itemName = "NUM_MONTHS";
const int GASPERIO::NUM_MONTHS::defaultValue = 12;
const std::string GASPERIO::INITIAL_DCQ::itemName = "INITIAL_DCQ";
const std::string GASPERIO::SWING_REQ::itemName = "SWING_REQ";
const std::string GASPERIO::SWING_REQ::defaultValue = "PER";
const std::string GASPERIO::LIMIT_TIMESTEPS::itemName = "LIMIT_TIMESTEPS";
const std::string GASPERIO::LIMIT_TIMESTEPS::defaultValue = "YES";
const std::string GASPERIO::LIMIT_DCQ_RED_FACTOR::itemName = "LIMIT_DCQ_RED_FACTOR";
const double GASPERIO::LIMIT_DCQ_RED_FACTOR::defaultValue = 0.000000;
const std::string GASPERIO::ANTICIPATED_DCQ_RED_FACTOR::itemName = "ANTICIPATED_DCQ_RED_FACTOR";
const double GASPERIO::ANTICIPATED_DCQ_RED_FACTOR::defaultValue = 1.000000;
const std::string GASPERIO::MAX_ITERATIONS::itemName = "MAX_ITERATIONS";
const int GASPERIO::MAX_ITERATIONS::defaultValue = 3;
const std::string GASPERIO::DCQ_CONV_TOLERANCE::itemName = "DCQ_CONV_TOLERANCE";
const double GASPERIO::DCQ_CONV_TOLERANCE::defaultValue = 0.100000;


GASSATC::GASSATC( ) : ParserKeyword("GASSATC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GASSATC");
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
const std::string GASSATC::keywordName = "GASSATC";
const std::string GASSATC::data::itemName = "data";


GASVISCT::GASVISCT( ) : ParserKeyword("GASVISCT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GASVISCT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("ContextDependent");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASVISCT::keywordName = "GASVISCT";
const std::string GASVISCT::DATA::itemName = "DATA";


GASYEAR::GASYEAR( ) : ParserKeyword("GASYEAR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GASYEAR");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_YEARS", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("INITIAL_DCQ", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_REQ", ParserItem::itype::STRING);
        item.setDefault( std::string("YEAR") );
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_TIMESTEPS", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_DCQ_RED_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("ANTICIPATED_DCQ_RED_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITERATIONS", ParserItem::itype::INT);
        item.setDefault( 3 );
        record.addItem(item);
     }
     {
        ParserItem item("DCQ_CONV_TOLERANCE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASYEAR::keywordName = "GASYEAR";
const std::string GASYEAR::NUM_YEARS::itemName = "NUM_YEARS";
const std::string GASYEAR::INITIAL_DCQ::itemName = "INITIAL_DCQ";
const std::string GASYEAR::SWING_REQ::itemName = "SWING_REQ";
const std::string GASYEAR::SWING_REQ::defaultValue = "YEAR";
const std::string GASYEAR::LIMIT_TIMESTEPS::itemName = "LIMIT_TIMESTEPS";
const std::string GASYEAR::LIMIT_TIMESTEPS::defaultValue = "YES";
const std::string GASYEAR::LIMIT_DCQ_RED_FACTOR::itemName = "LIMIT_DCQ_RED_FACTOR";
const double GASYEAR::LIMIT_DCQ_RED_FACTOR::defaultValue = 0.000000;
const std::string GASYEAR::ANTICIPATED_DCQ_RED_FACTOR::itemName = "ANTICIPATED_DCQ_RED_FACTOR";
const double GASYEAR::ANTICIPATED_DCQ_RED_FACTOR::defaultValue = 1.000000;
const std::string GASYEAR::MAX_ITERATIONS::itemName = "MAX_ITERATIONS";
const int GASYEAR::MAX_ITERATIONS::defaultValue = 3;
const std::string GASYEAR::DCQ_CONV_TOLERANCE::itemName = "DCQ_CONV_TOLERANCE";
const double GASYEAR::DCQ_CONV_TOLERANCE::defaultValue = 0.100000;


GCALECON::GCALECON( ) : ParserKeyword("GCALECON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCALECON");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_ENERGY_PROD_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Energy/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_CALORIFIC_VAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Energy/GasSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("FLAG_END_RUN", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCALECON::keywordName = "GCALECON";
const std::string GCALECON::GROUP::itemName = "GROUP";
const std::string GCALECON::MIN_ENERGY_PROD_RATE::itemName = "MIN_ENERGY_PROD_RATE";
const double GCALECON::MIN_ENERGY_PROD_RATE::defaultValue = 0.000000;
const std::string GCALECON::MIN_CALORIFIC_VAL::itemName = "MIN_CALORIFIC_VAL";
const double GCALECON::MIN_CALORIFIC_VAL::defaultValue = 0.000000;
const std::string GCALECON::FLAG_END_RUN::itemName = "FLAG_END_RUN";
const std::string GCALECON::FLAG_END_RUN::defaultValue = "NO";


GCOMPIDX::GCOMPIDX( ) : ParserKeyword("GCOMPIDX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GCOMPIDX");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_COMPONENT_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCOMPIDX::keywordName = "GCOMPIDX";
const std::string GCOMPIDX::GAS_COMPONENT_INDEX::itemName = "GAS_COMPONENT_INDEX";


GCONCAL::GCONCAL( ) : ParserKeyword("GCONCAL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONCAL");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MEAN_CALORIFIC_VAL", ParserItem::itype::DOUBLE);
        item.push_backDimension("Energy/GasSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("RATE_RED_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.900000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONCAL::keywordName = "GCONCAL";
const std::string GCONCAL::GROUP::itemName = "GROUP";
const std::string GCONCAL::MEAN_CALORIFIC_VAL::itemName = "MEAN_CALORIFIC_VAL";
const std::string GCONCAL::ACTION::itemName = "ACTION";
const std::string GCONCAL::ACTION::defaultValue = "NONE";
const std::string GCONCAL::RATE_RED_FACTOR::itemName = "RATE_RED_FACTOR";
const double GCONCAL::RATE_RED_FACTOR::defaultValue = 0.900000;


GCONENG::GCONENG( ) : ParserKeyword("GCONENG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONENG");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ENERGY_PROD_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Energy/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONENG::keywordName = "GCONENG";
const std::string GCONENG::GROUP::itemName = "GROUP";
const std::string GCONENG::ENERGY_PROD_RATE::itemName = "ENERGY_PROD_RATE";
const double GCONENG::ENERGY_PROD_RATE::defaultValue = 100000000000000000000.000000;


GCONINJE::GCONINJE( ) : ParserKeyword("GCONINJE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONINJE");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_MODE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("RESV_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("REINJ_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("VOIDAGE_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("FREE", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("GUIDE_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GUIDE_DEF", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REINJECT_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VOIDAGE_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WETGAS_TARGET", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONINJE::keywordName = "GCONINJE";
const std::string GCONINJE::GROUP::itemName = "GROUP";
const std::string GCONINJE::PHASE::itemName = "PHASE";
const std::string GCONINJE::CONTROL_MODE::itemName = "CONTROL_MODE";
const std::string GCONINJE::CONTROL_MODE::defaultValue = "NONE";
const std::string GCONINJE::SURFACE_TARGET::itemName = "SURFACE_TARGET";
const UDAValue GCONINJE::SURFACE_TARGET::defaultValue = UDAValue(0.000000);
const std::string GCONINJE::RESV_TARGET::itemName = "RESV_TARGET";
const UDAValue GCONINJE::RESV_TARGET::defaultValue = UDAValue(0.000000);
const std::string GCONINJE::REINJ_TARGET::itemName = "REINJ_TARGET";
const UDAValue GCONINJE::REINJ_TARGET::defaultValue = UDAValue(0.000000);
const std::string GCONINJE::VOIDAGE_TARGET::itemName = "VOIDAGE_TARGET";
const UDAValue GCONINJE::VOIDAGE_TARGET::defaultValue = UDAValue(0.000000);
const std::string GCONINJE::FREE::itemName = "FREE";
const std::string GCONINJE::FREE::defaultValue = "YES";
const std::string GCONINJE::GUIDE_FRACTION::itemName = "GUIDE_FRACTION";
const double GCONINJE::GUIDE_FRACTION::defaultValue = 0.000000;
const std::string GCONINJE::GUIDE_DEF::itemName = "GUIDE_DEF";
const std::string GCONINJE::REINJECT_GROUP::itemName = "REINJECT_GROUP";
const std::string GCONINJE::VOIDAGE_GROUP::itemName = "VOIDAGE_GROUP";
const std::string GCONINJE::WETGAS_TARGET::itemName = "WETGAS_TARGET";


GCONPRI::GCONPRI( ) : ParserKeyword("GCONPRI")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONPRI");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OIL_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_OIL_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("WAT_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_WAT_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_GAS_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("LIQ_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_LIQ_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("RES_FLUID_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("RES_VOL_BALANCING_FRAC_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WET_GAS_PROD_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_WET_GAS_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("SURF_GAS_BALANCING_FRAC_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SURF_WAT_BALANCING_FRAC_UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_LIMIT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONPRI::keywordName = "GCONPRI";
const std::string GCONPRI::GROUP::itemName = "GROUP";
const std::string GCONPRI::OIL_PROD_UPPER_LIMIT::itemName = "OIL_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::OIL_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_OIL_LIMIT::itemName = "PROCEDURE_OIL_LIMIT";
const std::string GCONPRI::PROCEDURE_OIL_LIMIT::defaultValue = "NONE";
const std::string GCONPRI::WAT_PROD_UPPER_LIMIT::itemName = "WAT_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::WAT_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_WAT_LIMIT::itemName = "PROCEDURE_WAT_LIMIT";
const std::string GCONPRI::PROCEDURE_WAT_LIMIT::defaultValue = "NONE";
const std::string GCONPRI::GAS_PROD_UPPER_LIMIT::itemName = "GAS_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::GAS_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_GAS_LIMIT::itemName = "PROCEDURE_GAS_LIMIT";
const std::string GCONPRI::PROCEDURE_GAS_LIMIT::defaultValue = "NONE";
const std::string GCONPRI::LIQ_PROD_UPPER_LIMIT::itemName = "LIQ_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::LIQ_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_LIQ_LIMIT::itemName = "PROCEDURE_LIQ_LIMIT";
const std::string GCONPRI::PROCEDURE_LIQ_LIMIT::defaultValue = "NONE";
const std::string GCONPRI::RES_FLUID_PROD_UPPER_LIMIT::itemName = "RES_FLUID_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::RES_FLUID_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::RES_VOL_BALANCING_FRAC_UPPER_LIMIT::itemName = "RES_VOL_BALANCING_FRAC_UPPER_LIMIT";
const UDAValue GCONPRI::RES_VOL_BALANCING_FRAC_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::WET_GAS_PROD_UPPER_LIMIT::itemName = "WET_GAS_PROD_UPPER_LIMIT";
const UDAValue GCONPRI::WET_GAS_PROD_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_WET_GAS_LIMIT::itemName = "PROCEDURE_WET_GAS_LIMIT";
const std::string GCONPRI::PROCEDURE_WET_GAS_LIMIT::defaultValue = "NONE";
const std::string GCONPRI::SURF_GAS_BALANCING_FRAC_UPPER_LIMIT::itemName = "SURF_GAS_BALANCING_FRAC_UPPER_LIMIT";
const UDAValue GCONPRI::SURF_GAS_BALANCING_FRAC_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::SURF_WAT_BALANCING_FRAC_UPPER_LIMIT::itemName = "SURF_WAT_BALANCING_FRAC_UPPER_LIMIT";
const UDAValue GCONPRI::SURF_WAT_BALANCING_FRAC_UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::UPPER_LIMIT::itemName = "UPPER_LIMIT";
const UDAValue GCONPRI::UPPER_LIMIT::defaultValue = UDAValue(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000);
const std::string GCONPRI::PROCEDURE_LIMIT::itemName = "PROCEDURE_LIMIT";
const std::string GCONPRI::PROCEDURE_LIMIT::defaultValue = "NONE";


GCONPROD::GCONPROD( ) : ParserKeyword("GCONPROD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONPROD");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_MODE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("LIQUID_TARGET", ParserItem::itype::UDA);
        item.setDefault( UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("EXCEED_PROC", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("RESPOND_TO_PARENT", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("GUIDE_RATE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("GUIDE_RATE_DEF", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WATER_EXCEED_PROCEDURE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_EXCEED_PROCEDURE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LIQUID_EXCEED_PROCEDURE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RESERVOIR_FLUID_TARGET", ParserItem::itype::DOUBLE);
        item.setDefault( double(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000) );
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("RESERVOIR_VOLUME_BALANCE", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WET_GAS_TARGET", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("CALORIFIC_TARGET", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_GAS_BALANCE", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_WATER_BALANCE", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("LINEAR_COMBINED_TARGET", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("LIN_TARGET_EXCEED_PROCEDURE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONPROD::keywordName = "GCONPROD";
const std::string GCONPROD::GROUP::itemName = "GROUP";
const std::string GCONPROD::CONTROL_MODE::itemName = "CONTROL_MODE";
const std::string GCONPROD::CONTROL_MODE::defaultValue = "NONE";
const std::string GCONPROD::OIL_TARGET::itemName = "OIL_TARGET";
const UDAValue GCONPROD::OIL_TARGET::defaultValue = UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000);
const std::string GCONPROD::WATER_TARGET::itemName = "WATER_TARGET";
const UDAValue GCONPROD::WATER_TARGET::defaultValue = UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000);
const std::string GCONPROD::GAS_TARGET::itemName = "GAS_TARGET";
const UDAValue GCONPROD::GAS_TARGET::defaultValue = UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000);
const std::string GCONPROD::LIQUID_TARGET::itemName = "LIQUID_TARGET";
const UDAValue GCONPROD::LIQUID_TARGET::defaultValue = UDAValue(-9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000);
const std::string GCONPROD::EXCEED_PROC::itemName = "EXCEED_PROC";
const std::string GCONPROD::EXCEED_PROC::defaultValue = "NONE";
const std::string GCONPROD::RESPOND_TO_PARENT::itemName = "RESPOND_TO_PARENT";
const std::string GCONPROD::RESPOND_TO_PARENT::defaultValue = "YES";
const std::string GCONPROD::GUIDE_RATE::itemName = "GUIDE_RATE";
const std::string GCONPROD::GUIDE_RATE_DEF::itemName = "GUIDE_RATE_DEF";
const std::string GCONPROD::WATER_EXCEED_PROCEDURE::itemName = "WATER_EXCEED_PROCEDURE";
const std::string GCONPROD::GAS_EXCEED_PROCEDURE::itemName = "GAS_EXCEED_PROCEDURE";
const std::string GCONPROD::LIQUID_EXCEED_PROCEDURE::itemName = "LIQUID_EXCEED_PROCEDURE";
const std::string GCONPROD::RESERVOIR_FLUID_TARGET::itemName = "RESERVOIR_FLUID_TARGET";
const double GCONPROD::RESERVOIR_FLUID_TARGET::defaultValue = -9989999999999999764508097064678579891241680670206998059982957902605768119280291502755086619213633159168.000000;
const std::string GCONPROD::RESERVOIR_VOLUME_BALANCE::itemName = "RESERVOIR_VOLUME_BALANCE";
const std::string GCONPROD::WET_GAS_TARGET::itemName = "WET_GAS_TARGET";
const std::string GCONPROD::CALORIFIC_TARGET::itemName = "CALORIFIC_TARGET";
const std::string GCONPROD::SURFACE_GAS_BALANCE::itemName = "SURFACE_GAS_BALANCE";
const std::string GCONPROD::SURFACE_WATER_BALANCE::itemName = "SURFACE_WATER_BALANCE";
const std::string GCONPROD::LINEAR_COMBINED_TARGET::itemName = "LINEAR_COMBINED_TARGET";
const std::string GCONPROD::LIN_TARGET_EXCEED_PROCEDURE::itemName = "LIN_TARGET_EXCEED_PROCEDURE";


GCONSALE::GCONSALE( ) : ParserKeyword("GCONSALE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONSALE");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SALES_TARGET", ParserItem::itype::UDA);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SALES_RATE", ParserItem::itype::UDA);
        item.setDefault( UDAValue(100000000000000000000.000000) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_SALES_RATE", ParserItem::itype::UDA);
        item.setDefault( UDAValue(-100000000000000000000.000000) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PROC", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONSALE::keywordName = "GCONSALE";
const std::string GCONSALE::GROUP::itemName = "GROUP";
const std::string GCONSALE::SALES_TARGET::itemName = "SALES_TARGET";
const std::string GCONSALE::MAX_SALES_RATE::itemName = "MAX_SALES_RATE";
const UDAValue GCONSALE::MAX_SALES_RATE::defaultValue = UDAValue(100000000000000000000.000000);
const std::string GCONSALE::MIN_SALES_RATE::itemName = "MIN_SALES_RATE";
const UDAValue GCONSALE::MIN_SALES_RATE::defaultValue = UDAValue(-100000000000000000000.000000);
const std::string GCONSALE::MAX_PROC::itemName = "MAX_PROC";
const std::string GCONSALE::MAX_PROC::defaultValue = "NONE";


GCONSUMP::GCONSUMP( ) : ParserKeyword("GCONSUMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONSUMP");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_CONSUMP_RATE", ParserItem::itype::UDA);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_IMPORT_RATE", ParserItem::itype::UDA);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("NETWORK_NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONSUMP::keywordName = "GCONSUMP";
const std::string GCONSUMP::GROUP::itemName = "GROUP";
const std::string GCONSUMP::GAS_CONSUMP_RATE::itemName = "GAS_CONSUMP_RATE";
const std::string GCONSUMP::GAS_IMPORT_RATE::itemName = "GAS_IMPORT_RATE";
const std::string GCONSUMP::NETWORK_NODE::itemName = "NETWORK_NODE";


GCONTOL::GCONTOL( ) : ParserKeyword("GCONTOL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONTOL");
  {
     ParserRecord record;
     {
        ParserItem item("TOLERANCE_FRACTION", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NUPCOL_VALUE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TOLERANCE_FRACTION_INJ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_IT_INJ", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONTOL::keywordName = "GCONTOL";
const std::string GCONTOL::TOLERANCE_FRACTION::itemName = "TOLERANCE_FRACTION";
const std::string GCONTOL::NUPCOL_VALUE::itemName = "NUPCOL_VALUE";
const std::string GCONTOL::TOLERANCE_FRACTION_INJ::itemName = "TOLERANCE_FRACTION_INJ";
const double GCONTOL::TOLERANCE_FRACTION_INJ::defaultValue = 0.001000;
const std::string GCONTOL::MAX_IT_INJ::itemName = "MAX_IT_INJ";
const int GCONTOL::MAX_IT_INJ::defaultValue = 5;


GCUTBACK::GCUTBACK( ) : ParserKeyword("GCUTBACK")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCUTBACK");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WATER_CUT_UPPER_LIM", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_OIL_UPPER_LIM", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GAS_LIQ_UPPER_LIM", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WAT_GAS_UPPER_LIM", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("RATE_CUTBACK_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCUTBACK::keywordName = "GCUTBACK";
const std::string GCUTBACK::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GCUTBACK::WATER_CUT_UPPER_LIM::itemName = "WATER_CUT_UPPER_LIM";
const std::string GCUTBACK::GAS_OIL_UPPER_LIM::itemName = "GAS_OIL_UPPER_LIM";
const std::string GCUTBACK::GAS_LIQ_UPPER_LIM::itemName = "GAS_LIQ_UPPER_LIM";
const std::string GCUTBACK::WAT_GAS_UPPER_LIM::itemName = "WAT_GAS_UPPER_LIM";
const std::string GCUTBACK::RATE_CUTBACK_FACTOR::itemName = "RATE_CUTBACK_FACTOR";
const std::string GCUTBACK::CONTROL_PHASE::itemName = "CONTROL_PHASE";


GCUTBACT::GCUTBACT( ) : ParserKeyword("GCUTBACT")
{
  setSizeType(DOUBLE_SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCUTBACT");
  setDoubleRecordsKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RATE_CUTBACK", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_RATE_LIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_RATE_LIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_CONC_LIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_CONC_LIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCUTBACT::keywordName = "GCUTBACT";
const std::string GCUTBACT::GROUP::itemName = "GROUP";
const std::string GCUTBACT::RATE_CUTBACK::itemName = "RATE_CUTBACK";
const std::string GCUTBACT::CONTROL_PHASE::itemName = "CONTROL_PHASE";
const std::string GCUTBACT::TRACER::itemName = "TRACER";
const std::string GCUTBACT::UPPER_RATE_LIM::itemName = "UPPER_RATE_LIM";
const double GCUTBACT::UPPER_RATE_LIM::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GCUTBACT::LOWER_RATE_LIM::itemName = "LOWER_RATE_LIM";
const double GCUTBACT::LOWER_RATE_LIM::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GCUTBACT::UPPER_CONC_LIM::itemName = "UPPER_CONC_LIM";
const double GCUTBACT::UPPER_CONC_LIM::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GCUTBACT::LOWER_CONC_LIM::itemName = "LOWER_CONC_LIM";
const double GCUTBACT::LOWER_CONC_LIM::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;


GCVD::GCVD( ) : ParserKeyword("GCVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCVD::keywordName = "GCVD";
const std::string GCVD::DATA::itemName = "DATA";


GDCQ::GDCQ( ) : ParserKeyword("GDCQ")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GDCQ");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INIT_DCQ", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("DCQ_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("VAR") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDCQ::keywordName = "GDCQ";
const std::string GDCQ::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GDCQ::INIT_DCQ::itemName = "INIT_DCQ";
const std::string GDCQ::DCQ_TYPE::itemName = "DCQ_TYPE";
const std::string GDCQ::DCQ_TYPE::defaultValue = "VAR";


GDCQECON::GDCQECON( ) : ParserKeyword("GDCQECON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GDCQECON");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_DCQ", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDCQECON::keywordName = "GDCQECON";
const std::string GDCQECON::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GDCQECON::MIN_DCQ::itemName = "MIN_DCQ";


GDFILE::GDFILE( ) : ParserKeyword("GDFILE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GDFILE");
  {
     ParserRecord record;
     {
        ParserItem item("filename", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("formatted", ParserItem::itype::STRING);
        item.setDefault( std::string("U") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDFILE::keywordName = "GDFILE";
const std::string GDFILE::filename::itemName = "filename";
const std::string GDFILE::formatted::itemName = "formatted";
const std::string GDFILE::formatted::defaultValue = "U";


GDIMS::GDIMS( ) : ParserKeyword("GDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_NUM_GRAD_PARAMS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDIMS::keywordName = "GDIMS";
const std::string GDIMS::MAX_NUM_GRAD_PARAMS::itemName = "MAX_NUM_GRAD_PARAMS";
const int GDIMS::MAX_NUM_GRAD_PARAMS::defaultValue = 0;


GDORIENT::GDORIENT( ) : ParserKeyword("GDORIENT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GDORIENT");
  {
     ParserRecord record;
     {
        ParserItem item("I", ParserItem::itype::STRING);
        item.setDefault( std::string("INC") );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::STRING);
        item.setDefault( std::string("INC") );
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::STRING);
        item.setDefault( std::string("INC") );
        record.addItem(item);
     }
     {
        ParserItem item("Z", ParserItem::itype::STRING);
        item.setDefault( std::string("DOWN") );
        record.addItem(item);
     }
     {
        ParserItem item("HAND", ParserItem::itype::STRING);
        item.setDefault( std::string("RIGHT") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDORIENT::keywordName = "GDORIENT";
const std::string GDORIENT::I::itemName = "I";
const std::string GDORIENT::I::defaultValue = "INC";
const std::string GDORIENT::J::itemName = "J";
const std::string GDORIENT::J::defaultValue = "INC";
const std::string GDORIENT::K::itemName = "K";
const std::string GDORIENT::K::defaultValue = "INC";
const std::string GDORIENT::Z::itemName = "Z";
const std::string GDORIENT::Z::defaultValue = "DOWN";
const std::string GDORIENT::HAND::itemName = "HAND";
const std::string GDORIENT::HAND::defaultValue = "RIGHT";


GDRILPOT::GDRILPOT( ) : ParserKeyword("GDRILPOT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GDRILPOT");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QNTY_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_POTENTIAL_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDRILPOT::keywordName = "GDRILPOT";
const std::string GDRILPOT::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GDRILPOT::QNTY_TYPE::itemName = "QNTY_TYPE";
const std::string GDRILPOT::MIN_POTENTIAL_RATE::itemName = "MIN_POTENTIAL_RATE";
const double GDRILPOT::MIN_POTENTIAL_RATE::defaultValue = 0.000000;


GECON::GECON( ) : ParserKeyword("GECON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GECON");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_OIL_RATE", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_GAS_RATE", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WCT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GOR", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WATER_GAS_RATIO", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("END_RUN", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OPEN_WELLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GECON::keywordName = "GECON";
const std::string GECON::GROUP::itemName = "GROUP";
const std::string GECON::MIN_OIL_RATE::itemName = "MIN_OIL_RATE";
const UDAValue GECON::MIN_OIL_RATE::defaultValue = UDAValue(0.000000);
const std::string GECON::MIN_GAS_RATE::itemName = "MIN_GAS_RATE";
const UDAValue GECON::MIN_GAS_RATE::defaultValue = UDAValue(0.000000);
const std::string GECON::MAX_WCT::itemName = "MAX_WCT";
const UDAValue GECON::MAX_WCT::defaultValue = UDAValue(0.000000);
const std::string GECON::MAX_GOR::itemName = "MAX_GOR";
const UDAValue GECON::MAX_GOR::defaultValue = UDAValue(0.000000);
const std::string GECON::MAX_WATER_GAS_RATIO::itemName = "MAX_WATER_GAS_RATIO";
const UDAValue GECON::MAX_WATER_GAS_RATIO::defaultValue = UDAValue(0.000000);
const std::string GECON::WORKOVER::itemName = "WORKOVER";
const std::string GECON::WORKOVER::defaultValue = "NONE";
const std::string GECON::END_RUN::itemName = "END_RUN";
const std::string GECON::END_RUN::defaultValue = "NO";
const std::string GECON::MAX_OPEN_WELLS::itemName = "MAX_OPEN_WELLS";
const int GECON::MAX_OPEN_WELLS::defaultValue = 0;


GECONT::GECONT( ) : ParserKeyword("GECONT")
{
  setSizeType(DOUBLE_SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GECONT");
  setDoubleRecordsKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("END_RUN", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OPEN_WELLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TOTAL_TRACER_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TOTAL_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FREE_TRACER_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FREE_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SOL_TRACER_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SOL_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GECONT::keywordName = "GECONT";
const std::string GECONT::GROUP::itemName = "GROUP";
const std::string GECONT::PROCEDURE::itemName = "PROCEDURE";
const std::string GECONT::PROCEDURE::defaultValue = "NONE";
const std::string GECONT::END_RUN::itemName = "END_RUN";
const std::string GECONT::END_RUN::defaultValue = "NO";
const std::string GECONT::MAX_OPEN_WELLS::itemName = "MAX_OPEN_WELLS";
const int GECONT::MAX_OPEN_WELLS::defaultValue = 0;
const std::string GECONT::TRACER::itemName = "TRACER";
const std::string GECONT::MAX_TOTAL_TRACER_RATE::itemName = "MAX_TOTAL_TRACER_RATE";
const double GECONT::MAX_TOTAL_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GECONT::MAX_TOTAL_TRACER_CONC::itemName = "MAX_TOTAL_TRACER_CONC";
const double GECONT::MAX_TOTAL_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GECONT::MAX_FREE_TRACER_RATE::itemName = "MAX_FREE_TRACER_RATE";
const double GECONT::MAX_FREE_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GECONT::MAX_FREE_TRACER_CONC::itemName = "MAX_FREE_TRACER_CONC";
const double GECONT::MAX_FREE_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GECONT::MAX_SOL_TRACER_RATE::itemName = "MAX_SOL_TRACER_RATE";
const double GECONT::MAX_SOL_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string GECONT::MAX_SOL_TRACER_CONC::itemName = "MAX_SOL_TRACER_CONC";
const double GECONT::MAX_SOL_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;


GEFAC::GEFAC( ) : ParserKeyword("GEFAC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GEFAC");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("EFFICIENCY_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRANSFER_EXT_NET", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GEFAC::keywordName = "GEFAC";
const std::string GEFAC::GROUP::itemName = "GROUP";
const std::string GEFAC::EFFICIENCY_FACTOR::itemName = "EFFICIENCY_FACTOR";
const double GEFAC::EFFICIENCY_FACTOR::defaultValue = 1.000000;
const std::string GEFAC::TRANSFER_EXT_NET::itemName = "TRANSFER_EXT_NET";
const std::string GEFAC::TRANSFER_EXT_NET::defaultValue = "YES";


GETDATA::GETDATA( ) : ParserKeyword("GETDATA")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GETDATA");
  {
     ParserRecord record;
     {
        ParserItem item("FILENAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FORMATTED", ParserItem::itype::STRING);
        item.setDefault( std::string("UNFORMATTED") );
        record.addItem(item);
     }
     {
        ParserItem item("ZNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ZALT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GETDATA::keywordName = "GETDATA";
const std::string GETDATA::FILENAME::itemName = "FILENAME";
const std::string GETDATA::FORMATTED::itemName = "FORMATTED";
const std::string GETDATA::FORMATTED::defaultValue = "UNFORMATTED";
const std::string GETDATA::ZNAME::itemName = "ZNAME";
const std::string GETDATA::ZALT::itemName = "ZALT";


GETGLOB::GETGLOB( ) : ParserKeyword("GETGLOB")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GETGLOB");
}
const std::string GETGLOB::keywordName = "GETGLOB";


GI::GI( ) : ParserKeyword("GI")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("GI");
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
const std::string GI::keywordName = "GI";
const std::string GI::data::itemName = "data";


GIALL::GIALL( ) : ParserKeyword("GIALL")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GIALL");
  {
     ParserRecord record;
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GIALL::keywordName = "GIALL";
const std::string GIALL::PRESSURE::itemName = "PRESSURE";
const std::string GIALL::TABLE::itemName = "TABLE";


GIMODEL::GIMODEL( ) : ParserKeyword("GIMODEL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GIMODEL");
}
const std::string GIMODEL::keywordName = "GIMODEL";


GINODE::GINODE( ) : ParserKeyword("GINODE")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GINODE");
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
const std::string GINODE::keywordName = "GINODE";
const std::string GINODE::DATA::itemName = "DATA";


GLIFTLIM::GLIFTLIM( ) : ParserKeyword("GLIFTLIM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GLIFTLIM");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_LIFT_CAPACITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_NUM_WELL", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GLIFTLIM::keywordName = "GLIFTLIM";
const std::string GLIFTLIM::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GLIFTLIM::MAX_LIFT_CAPACITY::itemName = "MAX_LIFT_CAPACITY";
const double GLIFTLIM::MAX_LIFT_CAPACITY::defaultValue = 0.000000;
const std::string GLIFTLIM::MAX_NUM_WELL::itemName = "MAX_NUM_WELL";
const int GLIFTLIM::MAX_NUM_WELL::defaultValue = 0;


GLIFTOPT::GLIFTOPT( ) : ParserKeyword("GLIFTOPT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GLIFTOPT");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_LIFT_GAS_SUPPLY", ParserItem::itype::DOUBLE);
        item.setDefault( double(-100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TOTAL_GAS_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(-100000000000000000000.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GLIFTOPT::keywordName = "GLIFTOPT";
const std::string GLIFTOPT::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GLIFTOPT::MAX_LIFT_GAS_SUPPLY::itemName = "MAX_LIFT_GAS_SUPPLY";
const double GLIFTOPT::MAX_LIFT_GAS_SUPPLY::defaultValue = -100000000000000000000.000000;
const std::string GLIFTOPT::MAX_TOTAL_GAS_RATE::itemName = "MAX_TOTAL_GAS_RATE";
const double GLIFTOPT::MAX_TOTAL_GAS_RATE::defaultValue = -100000000000000000000.000000;


GMWSET::GMWSET( ) : ParserKeyword("GMWSET")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("GMWSET");
}
const std::string GMWSET::keywordName = "GMWSET";


GNETDP::GNETDP( ) : ParserKeyword("GNETDP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GNETDP");
  {
     ParserRecord record;
     {
        ParserItem item("FIXED_PRESSURE_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PHASE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("GA") );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_RATE_TRIGGER", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_RATE_TRIGGER", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_INCR_SUBTRACT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_INCR_ADD", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_ALLOW_PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ALLOW_PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GNETDP::keywordName = "GNETDP";
const std::string GNETDP::FIXED_PRESSURE_GROUP::itemName = "FIXED_PRESSURE_GROUP";
const std::string GNETDP::PHASE_TYPE::itemName = "PHASE_TYPE";
const std::string GNETDP::PHASE_TYPE::defaultValue = "GA";
const std::string GNETDP::MIN_RATE_TRIGGER::itemName = "MIN_RATE_TRIGGER";
const double GNETDP::MIN_RATE_TRIGGER::defaultValue = 0.000000;
const std::string GNETDP::MAX_RATE_TRIGGER::itemName = "MAX_RATE_TRIGGER";
const double GNETDP::MAX_RATE_TRIGGER::defaultValue = 100000000000000000000.000000;
const std::string GNETDP::PRESSURE_INCR_SUBTRACT::itemName = "PRESSURE_INCR_SUBTRACT";
const double GNETDP::PRESSURE_INCR_SUBTRACT::defaultValue = 0.000000;
const std::string GNETDP::PRESSURE_INCR_ADD::itemName = "PRESSURE_INCR_ADD";
const double GNETDP::PRESSURE_INCR_ADD::defaultValue = 0.000000;
const std::string GNETDP::MIN_ALLOW_PRESSURE::itemName = "MIN_ALLOW_PRESSURE";
const double GNETDP::MIN_ALLOW_PRESSURE::defaultValue = 0.000000;
const std::string GNETDP::MAX_ALLOW_PRESSURE::itemName = "MAX_ALLOW_PRESSURE";
const double GNETDP::MAX_ALLOW_PRESSURE::defaultValue = 100000000000000000000.000000;


GNETINJE::GNETINJE( ) : ParserKeyword("GNETINJE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GNETINJE");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GNETINJE::keywordName = "GNETINJE";
const std::string GNETINJE::GROUP::itemName = "GROUP";
const std::string GNETINJE::PHASE::itemName = "PHASE";
const std::string GNETINJE::PRESSURE::itemName = "PRESSURE";
const std::string GNETINJE::VFP_TABLE::itemName = "VFP_TABLE";
const int GNETINJE::VFP_TABLE::defaultValue = 0;


GNETPUMP::GNETPUMP( ) : ParserKeyword("GNETPUMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GNETPUMP");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PROD_RATE_SWITCH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("OIL") );
        record.addItem(item);
     }
     {
        ParserItem item("NEW_VFT_TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 4 );
        record.addItem(item);
     }
     {
        ParserItem item("NEW_ARTIFICIAL_LIFT_QNTY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("NEW_GAS_CONUMPTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GNETPUMP::keywordName = "GNETPUMP";
const std::string GNETPUMP::GROUP::itemName = "GROUP";
const std::string GNETPUMP::PROD_RATE_SWITCH::itemName = "PROD_RATE_SWITCH";
const double GNETPUMP::PROD_RATE_SWITCH::defaultValue = 0.000000;
const std::string GNETPUMP::PHASE_TYPE::itemName = "PHASE_TYPE";
const std::string GNETPUMP::PHASE_TYPE::defaultValue = "OIL";
const std::string GNETPUMP::NEW_VFT_TABLE_NUM::itemName = "NEW_VFT_TABLE_NUM";
const int GNETPUMP::NEW_VFT_TABLE_NUM::defaultValue = 4;
const std::string GNETPUMP::NEW_ARTIFICIAL_LIFT_QNTY::itemName = "NEW_ARTIFICIAL_LIFT_QNTY";
const double GNETPUMP::NEW_ARTIFICIAL_LIFT_QNTY::defaultValue = 0.000000;
const std::string GNETPUMP::NEW_GAS_CONUMPTION_RATE::itemName = "NEW_GAS_CONUMPTION_RATE";
const double GNETPUMP::NEW_GAS_CONUMPTION_RATE::defaultValue = 0.000000;


GPMAINT::GPMAINT( ) : ParserKeyword("GPMAINT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GPMAINT");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_TARGET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FIP_FAMILY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_TARGET", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PROP_CONSTANT", ParserItem::itype::DOUBLE);
        item.push_backDimension("ReservoirVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TIME_CONSTANT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GPMAINT::keywordName = "GPMAINT";
const std::string GPMAINT::GROUP::itemName = "GROUP";
const std::string GPMAINT::FLOW_TARGET::itemName = "FLOW_TARGET";
const std::string GPMAINT::REGION::itemName = "REGION";
const std::string GPMAINT::FIP_FAMILY::itemName = "FIP_FAMILY";
const std::string GPMAINT::PRESSURE_TARGET::itemName = "PRESSURE_TARGET";
const std::string GPMAINT::PROP_CONSTANT::itemName = "PROP_CONSTANT";
const std::string GPMAINT::TIME_CONSTANT::itemName = "TIME_CONSTANT";


GRADGRUP::GRADGRUP( ) : ParserKeyword("GRADGRUP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRADGRUP");
  {
     ParserRecord record;
     {
        ParserItem item("MNENONIC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRADGRUP::keywordName = "GRADGRUP";
const std::string GRADGRUP::MNENONIC::itemName = "MNENONIC";


GRADRESV::GRADRESV( ) : ParserKeyword("GRADRESV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRADRESV");
  {
     ParserRecord record;
     {
        ParserItem item("MNENONIC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRADRESV::keywordName = "GRADRESV";
const std::string GRADRESV::MNENONIC::itemName = "MNENONIC";


GRADRFT::GRADRFT( ) : ParserKeyword("GRADRFT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRADRFT");
  {
     ParserRecord record;
     {
        ParserItem item("MNENONIC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRADRFT::keywordName = "GRADRFT";
const std::string GRADRFT::MNENONIC::itemName = "MNENONIC";


GRADWELL::GRADWELL( ) : ParserKeyword("GRADWELL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRADWELL");
  {
     ParserRecord record;
     {
        ParserItem item("MNENONIC", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRADWELL::keywordName = "GRADWELL";
const std::string GRADWELL::MNENONIC::itemName = "MNENONIC";


GRAVCONS::GRAVCONS( ) : ParserKeyword("GRAVCONS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GRAVCONS");
  {
     ParserRecord record;
     {
        ParserItem item("MNENONIC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Pressure/Mass");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRAVCONS::keywordName = "GRAVCONS";
const std::string GRAVCONS::MNENONIC::itemName = "MNENONIC";


GRAVDR::GRAVDR( ) : ParserKeyword("GRAVDR")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GRAVDR");
}
const std::string GRAVDR::keywordName = "GRAVDR";


GRAVDRB::GRAVDRB( ) : ParserKeyword("GRAVDRB")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GRAVDRB");
}
const std::string GRAVDRB::keywordName = "GRAVDRB";


GRAVDRM::GRAVDRM( ) : ParserKeyword("GRAVDRM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GRAVDRM");
  {
     ParserRecord record;
     {
        ParserItem item("ALLOW_RE_INFL", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRAVDRM::keywordName = "GRAVDRM";
const std::string GRAVDRM::ALLOW_RE_INFL::itemName = "ALLOW_RE_INFL";
const std::string GRAVDRM::ALLOW_RE_INFL::defaultValue = "YES";


GRAVITY::GRAVITY( ) : ParserKeyword("GRAVITY")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GRAVITY");
  {
     ParserRecord record;
     {
        ParserItem item("API_GRAVITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_SP_GRAVITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_SP_GRAVITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.777300) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRAVITY::keywordName = "GRAVITY";
const std::string GRAVITY::API_GRAVITY::itemName = "API_GRAVITY";
const std::string GRAVITY::WATER_SP_GRAVITY::itemName = "WATER_SP_GRAVITY";
const double GRAVITY::WATER_SP_GRAVITY::defaultValue = 1.000000;
const std::string GRAVITY::GAS_SP_GRAVITY::itemName = "GAS_SP_GRAVITY";
const double GRAVITY::GAS_SP_GRAVITY::defaultValue = 0.777300;


GRDREACH::GRDREACH( ) : ParserKeyword("GRDREACH")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRDREACH");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
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
        ParserItem item("BRANCH_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_TO_START", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_TO_END", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("RCH_CONNECT_TO", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PENETRATION_DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GRID_BLOCK_COORD", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CONTACT_AREA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PRODUCTIVITY_INDEX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH_DEAD_GRID_BLOCK", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION_CONNECT_REACH", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ADJUSTMENT_REACH", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("REMOVE_CAP_PRESSURE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("INFILTR_EQ", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("HYDRAULIC_CONDUCTIVITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("RIVER_BED_THICKNESS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRDREACH::keywordName = "GRDREACH";
const std::string GRDREACH::RIVER::itemName = "RIVER";
const std::string GRDREACH::I::itemName = "I";
const std::string GRDREACH::J::itemName = "J";
const std::string GRDREACH::K::itemName = "K";
const std::string GRDREACH::BRANCH_NAME::itemName = "BRANCH_NAME";
const std::string GRDREACH::DISTANCE_TO_START::itemName = "DISTANCE_TO_START";
const std::string GRDREACH::DISTANCE_TO_END::itemName = "DISTANCE_TO_END";
const std::string GRDREACH::RCH_CONNECT_TO::itemName = "RCH_CONNECT_TO";
const std::string GRDREACH::PENETRATION_DIRECTION::itemName = "PENETRATION_DIRECTION";
const std::string GRDREACH::GRID_BLOCK_COORD::itemName = "GRID_BLOCK_COORD";
const std::string GRDREACH::CONTACT_AREA::itemName = "CONTACT_AREA";
const std::string GRDREACH::TABLE_NUM::itemName = "TABLE_NUM";
const std::string GRDREACH::PRODUCTIVITY_INDEX::itemName = "PRODUCTIVITY_INDEX";
const double GRDREACH::PRODUCTIVITY_INDEX::defaultValue = 0.000000;
const std::string GRDREACH::LENGTH_DEAD_GRID_BLOCK::itemName = "LENGTH_DEAD_GRID_BLOCK";
const std::string GRDREACH::OPTION_CONNECT_REACH::itemName = "OPTION_CONNECT_REACH";
const int GRDREACH::OPTION_CONNECT_REACH::defaultValue = 0;
const std::string GRDREACH::ADJUSTMENT_REACH::itemName = "ADJUSTMENT_REACH";
const int GRDREACH::ADJUSTMENT_REACH::defaultValue = 0;
const std::string GRDREACH::REMOVE_CAP_PRESSURE::itemName = "REMOVE_CAP_PRESSURE";
const std::string GRDREACH::REMOVE_CAP_PRESSURE::defaultValue = "NO";
const std::string GRDREACH::INFILTR_EQ::itemName = "INFILTR_EQ";
const int GRDREACH::INFILTR_EQ::defaultValue = 0;
const std::string GRDREACH::HYDRAULIC_CONDUCTIVITY::itemName = "HYDRAULIC_CONDUCTIVITY";
const double GRDREACH::HYDRAULIC_CONDUCTIVITY::defaultValue = 0.000000;
const std::string GRDREACH::RIVER_BED_THICKNESS::itemName = "RIVER_BED_THICKNESS";
const double GRDREACH::RIVER_BED_THICKNESS::defaultValue = 0.000000;


GRID::GRID( ) : ParserKeyword("GRID")
{
  setFixedSize( (size_t) 0);
  clearDeckNames();
  addDeckName("GRID");
}
const std::string GRID::keywordName = "GRID";


GRIDFILE::GRIDFILE( ) : ParserKeyword("GRIDFILE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GRIDFILE");
  {
     ParserRecord record;
     {
        ParserItem item("GRID", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("EGRID", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDFILE::keywordName = "GRIDFILE";
const std::string GRIDFILE::GRID::itemName = "GRID";
const int GRIDFILE::GRID::defaultValue = 0;
const std::string GRIDFILE::EGRID::itemName = "EGRID";
const int GRIDFILE::EGRID::defaultValue = 1;


GRIDOPTS::GRIDOPTS( ) : ParserKeyword("GRIDOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GRIDOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("TRANMULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("NRMULT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NRPINC", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDOPTS::keywordName = "GRIDOPTS";
const std::string GRIDOPTS::TRANMULT::itemName = "TRANMULT";
const std::string GRIDOPTS::TRANMULT::defaultValue = "NO";
const std::string GRIDOPTS::NRMULT::itemName = "NRMULT";
const int GRIDOPTS::NRMULT::defaultValue = 0;
const std::string GRIDOPTS::NRPINC::itemName = "NRPINC";
const int GRIDOPTS::NRPINC::defaultValue = 0;


GRIDUNIT::GRIDUNIT( ) : ParserKeyword("GRIDUNIT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GRIDUNIT");
  {
     ParserRecord record;
     {
        ParserItem item("LengthUnit", ParserItem::itype::STRING);
        item.setDefault( std::string("METRES") );
        record.addItem(item);
     }
     {
        ParserItem item("MAP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDUNIT::keywordName = "GRIDUNIT";
const std::string GRIDUNIT::LengthUnit::itemName = "LengthUnit";
const std::string GRIDUNIT::LengthUnit::defaultValue = "METRES";
const std::string GRIDUNIT::MAP::itemName = "MAP";


GROUP_PROBE::GROUP_PROBE( ) : ParserKeyword("GROUP_PROBE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("GALQ");
  addDeckName("GAPI");
  addDeckName("GCIC");
  addDeckName("GCIR");
  addDeckName("GCIT");
  addDeckName("GCPC");
  addDeckName("GCPR");
  addDeckName("GCPT");
  addDeckName("GEDC");
  addDeckName("GEDCQ");
  addDeckName("GEFF");
  addDeckName("GEPR");
  addDeckName("GEPT");
  addDeckName("GESR");
  addDeckName("GEST");
  addDeckName("GFGR");
  addDeckName("GFGT");
  addDeckName("GFMF");
  addDeckName("GGCR");
  addDeckName("GGCT");
  addDeckName("GGCV");
  addDeckName("GGDC");
  addDeckName("GGDCQ");
  addDeckName("GGIGR");
  addDeckName("GGIMR");
  addDeckName("GGIMT");
  addDeckName("GGIR");
  addDeckName("GGIRH");
  addDeckName("GGIRL");
  addDeckName("GGIRNB");
  addDeckName("GGIRT");
  addDeckName("GGIT");
  addDeckName("GGITH");
  addDeckName("GGLIR");
  addDeckName("GGLR");
  addDeckName("GGLRH");
  addDeckName("GGOR");
  addDeckName("GGORH");
  addDeckName("GGPGR");
  addDeckName("GGPI");
  addDeckName("GGPI2");
  addDeckName("GGPP");
  addDeckName("GGPP2");
  addDeckName("GGPPF");
  addDeckName("GGPPF2");
  addDeckName("GGPPS");
  addDeckName("GGPPS2");
  addDeckName("GGPR");
  addDeckName("GGPRF");
  addDeckName("GGPRH");
  addDeckName("GGPRL");
  addDeckName("GGPRNB");
  addDeckName("GGPRNBFP");
  addDeckName("GGPRS");
  addDeckName("GGPRT");
  addDeckName("GGPT");
  addDeckName("GGPTF");
  addDeckName("GGPTH");
  addDeckName("GGPTS");
  addDeckName("GGQ");
  addDeckName("GGSPR");
  addDeckName("GGSR");
  addDeckName("GGSRL");
  addDeckName("GGSRU");
  addDeckName("GGSSP");
  addDeckName("GGST");
  addDeckName("GGSTP");
  addDeckName("GJPR");
  addDeckName("GJPRH");
  addDeckName("GJPRL");
  addDeckName("GJPRT");
  addDeckName("GJPT");
  addDeckName("GJPTH");
  addDeckName("GLPR");
  addDeckName("GLPRH");
  addDeckName("GLPRL");
  addDeckName("GLPRNB");
  addDeckName("GLPRT");
  addDeckName("GLPT");
  addDeckName("GLPTH");
  addDeckName("GMCPL");
  addDeckName("GMCTG");
  addDeckName("GMCTP");
  addDeckName("GMCTW");
  addDeckName("GMIR");
  addDeckName("GMIT");
  addDeckName("GMPR");
  addDeckName("GMPT");
  addDeckName("GMWDR");
  addDeckName("GMWDT");
  addDeckName("GMWIA");
  addDeckName("GMWIG");
  addDeckName("GMWIN");
  addDeckName("GMWIP");
  addDeckName("GMWIS");
  addDeckName("GMWIT");
  addDeckName("GMWIU");
  addDeckName("GMWIV");
  addDeckName("GMWPA");
  addDeckName("GMWPG");
  addDeckName("GMWPL");
  addDeckName("GMWPO");
  addDeckName("GMWPP");
  addDeckName("GMWPR");
  addDeckName("GMWPS");
  addDeckName("GMWPT");
  addDeckName("GMWPU");
  addDeckName("GMWPV");
  addDeckName("GMWWO");
  addDeckName("GMWWT");
  addDeckName("GNIR");
  addDeckName("GNIT");
  addDeckName("GNPR");
  addDeckName("GNPT");
  addDeckName("GOGR");
  addDeckName("GOGRH");
  addDeckName("GOIGR");
  addDeckName("GOIR");
  addDeckName("GOIRH");
  addDeckName("GOIRL");
  addDeckName("GOIRT");
  addDeckName("GOIT");
  addDeckName("GOITH");
  addDeckName("GOPGR");
  addDeckName("GOPI");
  addDeckName("GOPI2");
  addDeckName("GOPP");
  addDeckName("GOPP2");
  addDeckName("GOPR");
  addDeckName("GOPRF");
  addDeckName("GOPRH");
  addDeckName("GOPRL");
  addDeckName("GOPRNB");
  addDeckName("GOPRS");
  addDeckName("GOPRT");
  addDeckName("GOPT");
  addDeckName("GOPTF");
  addDeckName("GOPTH");
  addDeckName("GOPTS");
  addDeckName("GOSPR");
  addDeckName("GOSRL");
  addDeckName("GOSRU");
  addDeckName("GOSSP");
  addDeckName("GOSTP");
  addDeckName("GPR");
  addDeckName("GPRB");
  addDeckName("GPRBG");
  addDeckName("GPRBW");
  addDeckName("GPRDC");
  addDeckName("GPRFP");
  addDeckName("GPRG");
  addDeckName("GPRW");
  addDeckName("GSGR");
  addDeckName("GSGT");
  addDeckName("GSIC");
  addDeckName("GSIR");
  addDeckName("GSIT");
  addDeckName("GSMF");
  addDeckName("GSPC");
  addDeckName("GSPR");
  addDeckName("GSPT");
  addDeckName("GTICHEA");
  addDeckName("GTIRALK");
  addDeckName("GTIRANI");
  addDeckName("GTIRCAT");
  addDeckName("GTIRFOA");
  addDeckName("GTIRHEA");
  addDeckName("GTIRSUR");
  addDeckName("GTITALK");
  addDeckName("GTITANI");
  addDeckName("GTITCAT");
  addDeckName("GTITFOA");
  addDeckName("GTITHEA");
  addDeckName("GTITSUR");
  addDeckName("GTPCHEA");
  addDeckName("GTPRALK");
  addDeckName("GTPRANI");
  addDeckName("GTPRCAT");
  addDeckName("GTPRFOA");
  addDeckName("GTPRHEA");
  addDeckName("GTPRSUR");
  addDeckName("GTPTALK");
  addDeckName("GTPTANI");
  addDeckName("GTPTCAT");
  addDeckName("GTPTFOA");
  addDeckName("GTPTHEA");
  addDeckName("GTPTSUR");
  addDeckName("GVIR");
  addDeckName("GVIRL");
  addDeckName("GVIRT");
  addDeckName("GVIT");
  addDeckName("GVPGR");
  addDeckName("GVPR");
  addDeckName("GVPRL");
  addDeckName("GVPRT");
  addDeckName("GVPT");
  addDeckName("GWCT");
  addDeckName("GWCTH");
  addDeckName("GWGR");
  addDeckName("GWGRH");
  addDeckName("GWIGR");
  addDeckName("GWIR");
  addDeckName("GWIRH");
  addDeckName("GWIRL");
  addDeckName("GWIRNB");
  addDeckName("GWIRT");
  addDeckName("GWIT");
  addDeckName("GWITH");
  addDeckName("GWPGR");
  addDeckName("GWPI");
  addDeckName("GWPI2");
  addDeckName("GWPIR");
  addDeckName("GWPP");
  addDeckName("GWPP2");
  addDeckName("GWPR");
  addDeckName("GWPRH");
  addDeckName("GWPRL");
  addDeckName("GWPRNB");
  addDeckName("GWPRT");
  addDeckName("GWPT");
  addDeckName("GWPTH");
  addDeckName("GWSPR");
  addDeckName("GWSRL");
  addDeckName("GWSRU");
  addDeckName("GWSSP");
  addDeckName("GWSTP");
  setMatchRegex("GU.+|GTPR.+|GTPT.+|GTPC.+|GTIR.+|GTIT.+|GTIC.+|GTIRF.+|GTIRS.+|GTPRF.+|GTPRS.+|GTITF.+|GTITS.+|GTPTF.+|GTPTS.+|GTICF.+|GTICS.+|GTPCF.+|GTPCS.+");
  {
     ParserRecord record;
     {
        ParserItem item("GROUPS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GROUP_PROBE::keywordName = "GROUP_PROBE";
const std::string GROUP_PROBE::GROUPS::itemName = "GROUPS";


GRUPMAST::GRUPMAST( ) : ParserKeyword("GRUPMAST")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPMAST");
  {
     ParserRecord record;
     {
        ParserItem item("MASTER_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SLAVE_RESERVOIR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SLAVE_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LIMITING_FRACTION", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPMAST::keywordName = "GRUPMAST";
const std::string GRUPMAST::MASTER_GROUP::itemName = "MASTER_GROUP";
const std::string GRUPMAST::SLAVE_RESERVOIR::itemName = "SLAVE_RESERVOIR";
const std::string GRUPMAST::SLAVE_GROUP::itemName = "SLAVE_GROUP";
const std::string GRUPMAST::LIMITING_FRACTION::itemName = "LIMITING_FRACTION";


GRUPNET::GRUPNET( ) : ParserKeyword("GRUPNET")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("GRUPNET");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TERMINAL_PRESSURE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ALQ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SUB_SEA_MANIFOLD", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("LIFT_GAS_FLOW_THROUGH", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_SURFACE_EQV", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPNET::keywordName = "GRUPNET";
const std::string GRUPNET::NAME::itemName = "NAME";
const std::string GRUPNET::TERMINAL_PRESSURE::itemName = "TERMINAL_PRESSURE";
const std::string GRUPNET::VFP_TABLE::itemName = "VFP_TABLE";
const int GRUPNET::VFP_TABLE::defaultValue = 0;
const std::string GRUPNET::ALQ::itemName = "ALQ";
const double GRUPNET::ALQ::defaultValue = 0.000000;
const std::string GRUPNET::SUB_SEA_MANIFOLD::itemName = "SUB_SEA_MANIFOLD";
const std::string GRUPNET::SUB_SEA_MANIFOLD::defaultValue = "NO";
const std::string GRUPNET::LIFT_GAS_FLOW_THROUGH::itemName = "LIFT_GAS_FLOW_THROUGH";
const std::string GRUPNET::LIFT_GAS_FLOW_THROUGH::defaultValue = "NO";
const std::string GRUPNET::ALQ_SURFACE_EQV::itemName = "ALQ_SURFACE_EQV";
const std::string GRUPNET::ALQ_SURFACE_EQV::defaultValue = "NONE";


GRUPRIG::GRUPRIG( ) : ParserKeyword("GRUPRIG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPRIG");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WORKRIGNUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("DRILRIGNUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ADD", ParserItem::itype::STRING);
        item.setDefault( std::string("ADD") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPRIG::keywordName = "GRUPRIG";
const std::string GRUPRIG::GROUP_NAME::itemName = "GROUP_NAME";
const std::string GRUPRIG::WORKRIGNUM::itemName = "WORKRIGNUM";
const int GRUPRIG::WORKRIGNUM::defaultValue = 0;
const std::string GRUPRIG::DRILRIGNUM::itemName = "DRILRIGNUM";
const int GRUPRIG::DRILRIGNUM::defaultValue = 0;
const std::string GRUPRIG::ADD::itemName = "ADD";
const std::string GRUPRIG::ADD::defaultValue = "ADD";


GRUPSLAV::GRUPSLAV( ) : ParserKeyword("GRUPSLAV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPSLAV");
  {
     ParserRecord record;
     {
        ParserItem item("SLAVE_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MASTER_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OIL_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("WAT_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("FLUID_VOL_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_INJ_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("WAT_INJ_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_INJ_PROD_CONSTRAINTS", ParserItem::itype::STRING);
        item.setDefault( std::string("MAST") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPSLAV::keywordName = "GRUPSLAV";
const std::string GRUPSLAV::SLAVE_GROUP::itemName = "SLAVE_GROUP";
const std::string GRUPSLAV::MASTER_GROUP::itemName = "MASTER_GROUP";
const std::string GRUPSLAV::OIL_PROD_CONSTRAINTS::itemName = "OIL_PROD_CONSTRAINTS";
const std::string GRUPSLAV::OIL_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::WAT_PROD_CONSTRAINTS::itemName = "WAT_PROD_CONSTRAINTS";
const std::string GRUPSLAV::WAT_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::GAS_PROD_CONSTRAINTS::itemName = "GAS_PROD_CONSTRAINTS";
const std::string GRUPSLAV::GAS_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::FLUID_VOL_PROD_CONSTRAINTS::itemName = "FLUID_VOL_PROD_CONSTRAINTS";
const std::string GRUPSLAV::FLUID_VOL_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::OIL_INJ_PROD_CONSTRAINTS::itemName = "OIL_INJ_PROD_CONSTRAINTS";
const std::string GRUPSLAV::OIL_INJ_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::WAT_INJ_PROD_CONSTRAINTS::itemName = "WAT_INJ_PROD_CONSTRAINTS";
const std::string GRUPSLAV::WAT_INJ_PROD_CONSTRAINTS::defaultValue = "MAST";
const std::string GRUPSLAV::GAS_INJ_PROD_CONSTRAINTS::itemName = "GAS_INJ_PROD_CONSTRAINTS";
const std::string GRUPSLAV::GAS_INJ_PROD_CONSTRAINTS::defaultValue = "MAST";


GRUPTARG::GRUPTARG( ) : ParserKeyword("GRUPTARG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPTARG");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TARGET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(999999999999999949387135297074018866963645011013410073083904.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPTARG::keywordName = "GRUPTARG";
const std::string GRUPTARG::GROUP::itemName = "GROUP";
const std::string GRUPTARG::TARGET::itemName = "TARGET";
const std::string GRUPTARG::NEW_VALUE::itemName = "NEW_VALUE";
const double GRUPTARG::NEW_VALUE::defaultValue = 999999999999999949387135297074018866963645011013410073083904.000000;


GRUPTREE::GRUPTREE( ) : ParserKeyword("GRUPTREE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPTREE");
  {
     ParserRecord record;
     {
        ParserItem item("CHILD_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PARENT_GROUP", ParserItem::itype::STRING);
        item.setDefault( std::string("FIELD") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPTREE::keywordName = "GRUPTREE";
const std::string GRUPTREE::CHILD_GROUP::itemName = "CHILD_GROUP";
const std::string GRUPTREE::PARENT_GROUP::itemName = "PARENT_GROUP";
const std::string GRUPTREE::PARENT_GROUP::defaultValue = "FIELD";


GSATINJE::GSATINJE( ) : ParserKeyword("GSATINJE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GSATINJE");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SURF_INJ_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("RES_INJ_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MEAN_CALORIFIC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Energy/GasSurfaceVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GSATINJE::keywordName = "GSATINJE";
const std::string GSATINJE::GROUP::itemName = "GROUP";
const std::string GSATINJE::PHASE::itemName = "PHASE";
const std::string GSATINJE::SURF_INJ_RATE::itemName = "SURF_INJ_RATE";
const double GSATINJE::SURF_INJ_RATE::defaultValue = 0.000000;
const std::string GSATINJE::RES_INJ_RATE::itemName = "RES_INJ_RATE";
const double GSATINJE::RES_INJ_RATE::defaultValue = 0.000000;
const std::string GSATINJE::MEAN_CALORIFIC::itemName = "MEAN_CALORIFIC";
const double GSATINJE::MEAN_CALORIFIC::defaultValue = 0.000000;


GSATPROD::GSATPROD( ) : ParserKeyword("GSATPROD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GSATPROD");
  {
     ParserRecord record;
     {
        ParserItem item("SATELLITE_GROUP_NAME_OR_GROUP_NAME_ROOT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OIL_PRODUCTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_PRODUCTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_PRODUCTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("RES_FLUID_VOL_PRODUCTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("LIFT_GAS_SUPPLY_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MEAN_CALORIFIC_VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GSATPROD::keywordName = "GSATPROD";
const std::string GSATPROD::SATELLITE_GROUP_NAME_OR_GROUP_NAME_ROOT::itemName = "SATELLITE_GROUP_NAME_OR_GROUP_NAME_ROOT";
const std::string GSATPROD::OIL_PRODUCTION_RATE::itemName = "OIL_PRODUCTION_RATE";
const double GSATPROD::OIL_PRODUCTION_RATE::defaultValue = 0.000000;
const std::string GSATPROD::WATER_PRODUCTION_RATE::itemName = "WATER_PRODUCTION_RATE";
const double GSATPROD::WATER_PRODUCTION_RATE::defaultValue = 0.000000;
const std::string GSATPROD::GAS_PRODUCTION_RATE::itemName = "GAS_PRODUCTION_RATE";
const double GSATPROD::GAS_PRODUCTION_RATE::defaultValue = 0.000000;
const std::string GSATPROD::RES_FLUID_VOL_PRODUCTION_RATE::itemName = "RES_FLUID_VOL_PRODUCTION_RATE";
const double GSATPROD::RES_FLUID_VOL_PRODUCTION_RATE::defaultValue = 0.000000;
const std::string GSATPROD::LIFT_GAS_SUPPLY_RATE::itemName = "LIFT_GAS_SUPPLY_RATE";
const double GSATPROD::LIFT_GAS_SUPPLY_RATE::defaultValue = 0.000000;
const std::string GSATPROD::MEAN_CALORIFIC_VALUE::itemName = "MEAN_CALORIFIC_VALUE";
const double GSATPROD::MEAN_CALORIFIC_VALUE::defaultValue = 0.000000;


GSEPCOND::GSEPCOND( ) : ParserKeyword("GSEPCOND")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GSEPCOND");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEPARATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GSEPCOND::keywordName = "GSEPCOND";
const std::string GSEPCOND::GROUP::itemName = "GROUP";
const std::string GSEPCOND::SEPARATOR::itemName = "SEPARATOR";


GSSCPTST::GSSCPTST( ) : ParserKeyword("GSSCPTST")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GSSCPTST");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_MODE", ParserItem::itype::STRING);
        item.setDefault( std::string("GRAT") );
        record.addItem(item);
     }
     {
        ParserItem item("TARGET_PROD_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("TARGET_PROD_PERIOD", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PROD_RATE_FLAG", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("CONV_TOLERANCE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAT_IT", ParserItem::itype::INT);
        item.setDefault( 6 );
        record.addItem(item);
     }
     {
        ParserItem item("SUB_GRP_CONTROL_FLAG", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GSSCPTST::keywordName = "GSSCPTST";
const std::string GSSCPTST::GROUP::itemName = "GROUP";
const std::string GSSCPTST::CONTROL_MODE::itemName = "CONTROL_MODE";
const std::string GSSCPTST::CONTROL_MODE::defaultValue = "GRAT";
const std::string GSSCPTST::TARGET_PROD_RATE::itemName = "TARGET_PROD_RATE";
const double GSSCPTST::TARGET_PROD_RATE::defaultValue = 0.000000;
const std::string GSSCPTST::TARGET_PROD_PERIOD::itemName = "TARGET_PROD_PERIOD";
const double GSSCPTST::TARGET_PROD_PERIOD::defaultValue = 0.000000;
const std::string GSSCPTST::MAX_PROD_RATE_FLAG::itemName = "MAX_PROD_RATE_FLAG";
const int GSSCPTST::MAX_PROD_RATE_FLAG::defaultValue = 1;
const std::string GSSCPTST::CONV_TOLERANCE::itemName = "CONV_TOLERANCE";
const double GSSCPTST::CONV_TOLERANCE::defaultValue = 0.000000;
const std::string GSSCPTST::MAT_IT::itemName = "MAT_IT";
const int GSSCPTST::MAT_IT::defaultValue = 6;
const std::string GSSCPTST::SUB_GRP_CONTROL_FLAG::itemName = "SUB_GRP_CONTROL_FLAG";
const int GSSCPTST::SUB_GRP_CONTROL_FLAG::defaultValue = 1;


GSWINGF::GSWINGF( ) : ParserKeyword("GSWINGF")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GSWINGF");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_JAN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FEB", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_MAR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_APR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_MAY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_JUN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_JUL", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_AUG", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_SEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_OCT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_NOV", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWING_DEC", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_JAN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FEB", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_MAR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_APR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_MAY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_JUN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_JUL", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_AUG", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_SEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_OCT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_NOV", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_DEC", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GSWINGF::keywordName = "GSWINGF";
const std::string GSWINGF::GROUP::itemName = "GROUP";
const std::string GSWINGF::SWING_JAN::itemName = "SWING_JAN";
const std::string GSWINGF::SWING_FEB::itemName = "SWING_FEB";
const std::string GSWINGF::SWING_MAR::itemName = "SWING_MAR";
const std::string GSWINGF::SWING_APR::itemName = "SWING_APR";
const std::string GSWINGF::SWING_MAY::itemName = "SWING_MAY";
const std::string GSWINGF::SWING_JUN::itemName = "SWING_JUN";
const std::string GSWINGF::SWING_JUL::itemName = "SWING_JUL";
const std::string GSWINGF::SWING_AUG::itemName = "SWING_AUG";
const std::string GSWINGF::SWING_SEP::itemName = "SWING_SEP";
const std::string GSWINGF::SWING_OCT::itemName = "SWING_OCT";
const std::string GSWINGF::SWING_NOV::itemName = "SWING_NOV";
const std::string GSWINGF::SWING_DEC::itemName = "SWING_DEC";
const std::string GSWINGF::PROFILE_JAN::itemName = "PROFILE_JAN";
const std::string GSWINGF::PROFILE_FEB::itemName = "PROFILE_FEB";
const std::string GSWINGF::PROFILE_MAR::itemName = "PROFILE_MAR";
const std::string GSWINGF::PROFILE_APR::itemName = "PROFILE_APR";
const std::string GSWINGF::PROFILE_MAY::itemName = "PROFILE_MAY";
const std::string GSWINGF::PROFILE_JUN::itemName = "PROFILE_JUN";
const std::string GSWINGF::PROFILE_JUL::itemName = "PROFILE_JUL";
const std::string GSWINGF::PROFILE_AUG::itemName = "PROFILE_AUG";
const std::string GSWINGF::PROFILE_SEP::itemName = "PROFILE_SEP";
const std::string GSWINGF::PROFILE_OCT::itemName = "PROFILE_OCT";
const std::string GSWINGF::PROFILE_NOV::itemName = "PROFILE_NOV";
const std::string GSWINGF::PROFILE_DEC::itemName = "PROFILE_DEC";


GTADD::GTADD( ) : ParserKeyword("GTADD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GTADD");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TARGET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_ADDITIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GTADD::keywordName = "GTADD";
const std::string GTADD::GROUP::itemName = "GROUP";
const std::string GTADD::TARGET::itemName = "TARGET";
const std::string GTADD::QUANTITY::itemName = "QUANTITY";
const std::string GTADD::NUM_ADDITIONS::itemName = "NUM_ADDITIONS";
const int GTADD::NUM_ADDITIONS::defaultValue = 1;


GTMULT::GTMULT( ) : ParserKeyword("GTMULT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GTMULT");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TARGET", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_MULT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GTMULT::keywordName = "GTMULT";
const std::string GTMULT::GROUP::itemName = "GROUP";
const std::string GTMULT::TARGET::itemName = "TARGET";
const std::string GTMULT::QUANTITY::itemName = "QUANTITY";
const std::string GTMULT::NUM_MULT::itemName = "NUM_MULT";
const int GTMULT::NUM_MULT::defaultValue = 1;


GUIDECAL::GUIDECAL( ) : ParserKeyword("GUIDECAL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GUIDECAL");
  {
     ParserRecord record;
     {
        ParserItem item("COEFF_A", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("COEFF_B", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GUIDECAL::keywordName = "GUIDECAL";
const std::string GUIDECAL::COEFF_A::itemName = "COEFF_A";
const std::string GUIDECAL::COEFF_B::itemName = "COEFF_B";


GUIDERAT::GUIDERAT( ) : ParserKeyword("GUIDERAT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GUIDERAT");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_CALC_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("NOMINATED_PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
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
        ParserItem item("ALLOW_INCREASE", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("DAMPING_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("USE_FREE_GAS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_GUIDE_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GUIDERAT::keywordName = "GUIDERAT";
const std::string GUIDERAT::MIN_CALC_TIME::itemName = "MIN_CALC_TIME";
const double GUIDERAT::MIN_CALC_TIME::defaultValue = 0.000000;
const std::string GUIDERAT::NOMINATED_PHASE::itemName = "NOMINATED_PHASE";
const std::string GUIDERAT::NOMINATED_PHASE::defaultValue = "NONE";
const std::string GUIDERAT::A::itemName = "A";
const double GUIDERAT::A::defaultValue = 0.000000;
const std::string GUIDERAT::B::itemName = "B";
const double GUIDERAT::B::defaultValue = 0.000000;
const std::string GUIDERAT::C::itemName = "C";
const double GUIDERAT::C::defaultValue = 0.000000;
const std::string GUIDERAT::D::itemName = "D";
const double GUIDERAT::D::defaultValue = 0.000000;
const std::string GUIDERAT::E::itemName = "E";
const double GUIDERAT::E::defaultValue = 0.000000;
const std::string GUIDERAT::F::itemName = "F";
const double GUIDERAT::F::defaultValue = 0.000000;
const std::string GUIDERAT::ALLOW_INCREASE::itemName = "ALLOW_INCREASE";
const std::string GUIDERAT::ALLOW_INCREASE::defaultValue = "YES";
const std::string GUIDERAT::DAMPING_FACTOR::itemName = "DAMPING_FACTOR";
const double GUIDERAT::DAMPING_FACTOR::defaultValue = 1.000000;
const std::string GUIDERAT::USE_FREE_GAS::itemName = "USE_FREE_GAS";
const std::string GUIDERAT::USE_FREE_GAS::defaultValue = "NO";
const std::string GUIDERAT::MIN_GUIDE_RATE::itemName = "MIN_GUIDE_RATE";
const double GUIDERAT::MIN_GUIDE_RATE::defaultValue = 0.000001;


GUPFREQ::GUPFREQ( ) : ParserKeyword("GUPFREQ")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GUPFREQ");
  {
     ParserRecord record;
     {
        ParserItem item("UPDATE_FREQ_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("ALL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GUPFREQ::keywordName = "GUPFREQ";
const std::string GUPFREQ::UPDATE_FREQ_TYPE::itemName = "UPDATE_FREQ_TYPE";
const std::string GUPFREQ::UPDATE_FREQ_TYPE::defaultValue = "ALL";


GWRTWCV::GWRTWCV( ) : ParserKeyword("GWRTWCV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GWRTWCV");
  {
     ParserRecord record;
     {
        ParserItem item("WELLS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string GWRTWCV::keywordName = "GWRTWCV";
const std::string GWRTWCV::WELLS::itemName = "WELLS";


}
}
