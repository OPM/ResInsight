#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/V.hpp>
namespace Opm {
namespace ParserKeywords {
VAPOIL::VAPOIL( ) : ParserKeyword("VAPOIL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VAPOIL");
}
const std::string VAPOIL::keywordName = "VAPOIL";


VAPPARS::VAPPARS( ) : ParserKeyword("VAPPARS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("VAPPARS");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_VAP_PROPENSITY", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("OIL_DENSITY_PROPENSITY", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VAPPARS::keywordName = "VAPPARS";
const std::string VAPPARS::OIL_VAP_PROPENSITY::itemName = "OIL_VAP_PROPENSITY";
const std::string VAPPARS::OIL_DENSITY_PROPENSITY::itemName = "OIL_DENSITY_PROPENSITY";


VAPWAT::VAPWAT( ) : ParserKeyword("VAPWAT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VAPWAT");
}
const std::string VAPWAT::keywordName = "VAPWAT";


VDFLOW::VDFLOW( ) : ParserKeyword("VDFLOW")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VDFLOW");
  {
     ParserRecord record;
     {
        ParserItem item("BETA", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VDFLOW::keywordName = "VDFLOW";
const std::string VDFLOW::BETA::itemName = "BETA";


VDFLOWR::VDFLOWR( ) : ParserKeyword("VDFLOWR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VDFLOWR");
  {
     ParserRecord record;
     {
        ParserItem item("BETA", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VDFLOWR::keywordName = "VDFLOWR";
const std::string VDFLOWR::BETA::itemName = "BETA";


VE::VE( ) : ParserKeyword("VE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VE");
  {
     ParserRecord record;
     {
        ParserItem item("MODEL_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("NOCOMP") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VE::keywordName = "VE";
const std::string VE::MODEL_TYPE::itemName = "MODEL_TYPE";
const std::string VE::MODEL_TYPE::defaultValue = "NOCOMP";


VEDEBUG::VEDEBUG( ) : ParserKeyword("VEDEBUG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("VEDEBUG");
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
        ParserItem item("DEBUG_LEVEL", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        item.setDefault( std::string(" ") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VEDEBUG::keywordName = "VEDEBUG";
const std::string VEDEBUG::I1::itemName = "I1";
const std::string VEDEBUG::I2::itemName = "I2";
const std::string VEDEBUG::J1::itemName = "J1";
const std::string VEDEBUG::J2::itemName = "J2";
const std::string VEDEBUG::K1::itemName = "K1";
const std::string VEDEBUG::K2::itemName = "K2";
const std::string VEDEBUG::DEBUG_LEVEL::itemName = "DEBUG_LEVEL";
const int VEDEBUG::DEBUG_LEVEL::defaultValue = 0;
const std::string VEDEBUG::LGR::itemName = "LGR";
const std::string VEDEBUG::LGR::defaultValue = " ";


VEFIN::VEFIN( ) : ParserKeyword("VEFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("VEFIN");
  {
     ParserRecord record;
     {
        ParserItem item("VE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("NVEPT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VEFIN::keywordName = "VEFIN";
const std::string VEFIN::VE::itemName = "VE";
const std::string VEFIN::VE::defaultValue = "NO";
const std::string VEFIN::NVEPT::itemName = "NVEPT";
const int VEFIN::NVEPT::defaultValue = 0;


VEFRAC::VEFRAC( ) : ParserKeyword("VEFRAC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VEFRAC");
  {
     ParserRecord record;
     {
        ParserItem item("FRAC", ParserItem::itype::DOUBLE);
        item.setDefault( double(10.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VEFRAC::keywordName = "VEFRAC";
const std::string VEFRAC::FRAC::itemName = "FRAC";
const double VEFRAC::FRAC::defaultValue = 10.000000;


VEFRACP::VEFRACP( ) : ParserKeyword("VEFRACP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VEFRACP");
  {
     ParserRecord record;
     {
        ParserItem item("FRAC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VEFRACP::keywordName = "VEFRACP";
const std::string VEFRACP::FRAC::itemName = "FRAC";
const double VEFRACP::FRAC::defaultValue = 1.000000;


VEFRACPV::VEFRACPV( ) : ParserKeyword("VEFRACPV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VEFRACPV");
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
const std::string VEFRACPV::keywordName = "VEFRACPV";
const std::string VEFRACPV::data::itemName = "data";


VEFRACV::VEFRACV( ) : ParserKeyword("VEFRACV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VEFRACV");
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
const std::string VEFRACV::keywordName = "VEFRACV";
const std::string VEFRACV::data::itemName = "data";


VFPCHK::VFPCHK( ) : ParserKeyword("VFPCHK")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPCHK");
  {
     ParserRecord record;
     {
        ParserItem item("BHP_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(10000000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPCHK::keywordName = "VFPCHK";
const std::string VFPCHK::BHP_LIMIT::itemName = "BHP_LIMIT";
const double VFPCHK::BHP_LIMIT::defaultValue = 10000000000.000000;


VFPIDIMS::VFPIDIMS( ) : ParserKeyword("VFPIDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VFPIDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_FLOW_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_THP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_INJ_VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPIDIMS::keywordName = "VFPIDIMS";
const std::string VFPIDIMS::MAX_FLOW_TABLE::itemName = "MAX_FLOW_TABLE";
const int VFPIDIMS::MAX_FLOW_TABLE::defaultValue = 0;
const std::string VFPIDIMS::MAX_THP_TABLE::itemName = "MAX_THP_TABLE";
const int VFPIDIMS::MAX_THP_TABLE::defaultValue = 0;
const std::string VFPIDIMS::MAX_INJ_VFP_TABLE::itemName = "MAX_INJ_VFP_TABLE";
const int VFPIDIMS::MAX_INJ_VFP_TABLE::defaultValue = 0;


VFPINJ::VFPINJ( ) : ParserKeyword("VFPINJ")
{
  setSizeType(UNKNOWN);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPINJ");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("RATE_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("THP") );
        record.addItem(item);
     }
     {
        ParserItem item("UNITS", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("BODY_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("BHP") );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("FLOW_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("THP_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("THP_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPINJ::keywordName = "VFPINJ";
const std::string VFPINJ::TABLE::itemName = "TABLE";
const std::string VFPINJ::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string VFPINJ::RATE_TYPE::itemName = "RATE_TYPE";
const std::string VFPINJ::PRESSURE_DEF::itemName = "PRESSURE_DEF";
const std::string VFPINJ::PRESSURE_DEF::defaultValue = "THP";
const std::string VFPINJ::UNITS::itemName = "UNITS";
const std::string VFPINJ::BODY_DEF::itemName = "BODY_DEF";
const std::string VFPINJ::BODY_DEF::defaultValue = "BHP";
const std::string VFPINJ::FLOW_VALUES::itemName = "FLOW_VALUES";
const std::string VFPINJ::THP_VALUES::itemName = "THP_VALUES";
const std::string VFPINJ::THP_INDEX::itemName = "THP_INDEX";
const std::string VFPINJ::VALUES::itemName = "VALUES";


VFPPDIMS::VFPPDIMS( ) : ParserKeyword("VFPPDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VFPPDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_FLOW_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_THP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WCT_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GCT_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ALQ_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PROD_VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPPDIMS::keywordName = "VFPPDIMS";
const std::string VFPPDIMS::MAX_FLOW_TABLE::itemName = "MAX_FLOW_TABLE";
const int VFPPDIMS::MAX_FLOW_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_THP_TABLE::itemName = "MAX_THP_TABLE";
const int VFPPDIMS::MAX_THP_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_WCT_TABLE::itemName = "MAX_WCT_TABLE";
const int VFPPDIMS::MAX_WCT_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_GCT_TABLE::itemName = "MAX_GCT_TABLE";
const int VFPPDIMS::MAX_GCT_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_ALQ_TABLE::itemName = "MAX_ALQ_TABLE";
const int VFPPDIMS::MAX_ALQ_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_PROD_VFP_TABLE::itemName = "MAX_PROD_VFP_TABLE";
const int VFPPDIMS::MAX_PROD_VFP_TABLE::defaultValue = 0;


VFPPROD::VFPPROD( ) : ParserKeyword("VFPPROD")
{
  setSizeType(UNKNOWN);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPPROD");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("RATE_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WFR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GFR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("THP") );
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_DEF", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("UNITS", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("BODY_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("BHP") );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("FLOW_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("THP_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("WFR_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("GFR_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("ALQ_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("THP_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("WFR_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("GFR_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPPROD::keywordName = "VFPPROD";
const std::string VFPPROD::TABLE::itemName = "TABLE";
const std::string VFPPROD::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string VFPPROD::RATE_TYPE::itemName = "RATE_TYPE";
const std::string VFPPROD::WFR::itemName = "WFR";
const std::string VFPPROD::GFR::itemName = "GFR";
const std::string VFPPROD::PRESSURE_DEF::itemName = "PRESSURE_DEF";
const std::string VFPPROD::PRESSURE_DEF::defaultValue = "THP";
const std::string VFPPROD::ALQ_DEF::itemName = "ALQ_DEF";
const std::string VFPPROD::UNITS::itemName = "UNITS";
const std::string VFPPROD::BODY_DEF::itemName = "BODY_DEF";
const std::string VFPPROD::BODY_DEF::defaultValue = "BHP";
const std::string VFPPROD::FLOW_VALUES::itemName = "FLOW_VALUES";
const std::string VFPPROD::THP_VALUES::itemName = "THP_VALUES";
const std::string VFPPROD::WFR_VALUES::itemName = "WFR_VALUES";
const std::string VFPPROD::GFR_VALUES::itemName = "GFR_VALUES";
const std::string VFPPROD::ALQ_VALUES::itemName = "ALQ_VALUES";
const std::string VFPPROD::THP_INDEX::itemName = "THP_INDEX";
const std::string VFPPROD::WFR_INDEX::itemName = "WFR_INDEX";
const std::string VFPPROD::GFR_INDEX::itemName = "GFR_INDEX";
const std::string VFPPROD::ALQ_INDEX::itemName = "ALQ_INDEX";
const std::string VFPPROD::VALUES::itemName = "VALUES";


VFPTABL::VFPTABL( ) : ParserKeyword("VFPTABL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPTABL");
  {
     ParserRecord record;
     {
        ParserItem item("METHOD", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPTABL::keywordName = "VFPTABL";
const std::string VFPTABL::METHOD::itemName = "METHOD";


VISAGE::VISAGE( ) : ParserKeyword("VISAGE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VISAGE");
}
const std::string VISAGE::keywordName = "VISAGE";


VISCD::VISCD( ) : ParserKeyword("VISCD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VISCD");
}
const std::string VISCD::keywordName = "VISCD";


VISCREF::VISCREF( ) : ParserKeyword("VISCREF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VISCREF");
  {
     ParserRecord record;
     {
        ParserItem item("REFERENCE_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("REFERENCE_RS", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VISCREF::keywordName = "VISCREF";
const std::string VISCREF::REFERENCE_PRESSURE::itemName = "REFERENCE_PRESSURE";
const std::string VISCREF::REFERENCE_RS::itemName = "REFERENCE_RS";


VISDATES::VISDATES( ) : ParserKeyword("VISDATES")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VISDATES");
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
        ParserItem item("TIMESTAMP", ParserItem::itype::STRING);
        item.setDefault( std::string("00:00:00") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VISDATES::keywordName = "VISDATES";
const std::string VISDATES::DAY::itemName = "DAY";
const std::string VISDATES::MONTH::itemName = "MONTH";
const std::string VISDATES::YEAR::itemName = "YEAR";
const std::string VISDATES::TIMESTAMP::itemName = "TIMESTAMP";
const std::string VISDATES::TIMESTAMP::defaultValue = "00:00:00";


VISOPTS::VISOPTS( ) : ParserKeyword("VISOPTS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("VISOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("INIT_RUN", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("EXIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("ACTIVE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("REL_TOL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.050000) );
        record.addItem(item);
     }
     {
        ParserItem item("UNUSED", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RETAIN_RESTART_FREQUENCY", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("RETAIN_RESTART_CONTENT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("ERROR", ParserItem::itype::STRING);
        item.setDefault( std::string("ERROR") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string VISOPTS::keywordName = "VISOPTS";
const std::string VISOPTS::INIT_RUN::itemName = "INIT_RUN";
const std::string VISOPTS::INIT_RUN::defaultValue = "NO";
const std::string VISOPTS::EXIT::itemName = "EXIT";
const std::string VISOPTS::EXIT::defaultValue = "NO";
const std::string VISOPTS::ACTIVE::itemName = "ACTIVE";
const std::string VISOPTS::ACTIVE::defaultValue = "NO";
const std::string VISOPTS::REL_TOL::itemName = "REL_TOL";
const double VISOPTS::REL_TOL::defaultValue = 0.050000;
const std::string VISOPTS::UNUSED::itemName = "UNUSED";
const std::string VISOPTS::RETAIN_RESTART_FREQUENCY::itemName = "RETAIN_RESTART_FREQUENCY";
const std::string VISOPTS::RETAIN_RESTART_FREQUENCY::defaultValue = "NO";
const std::string VISOPTS::RETAIN_RESTART_CONTENT::itemName = "RETAIN_RESTART_CONTENT";
const std::string VISOPTS::RETAIN_RESTART_CONTENT::defaultValue = "NO";
const std::string VISOPTS::ERROR::itemName = "ERROR";
const std::string VISOPTS::ERROR::defaultValue = "ERROR";


}
}
