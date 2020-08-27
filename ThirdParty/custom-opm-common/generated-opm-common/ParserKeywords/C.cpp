#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/C.hpp>
namespace Opm {
namespace ParserKeywords {
CALTRAC::CALTRAC( ) : ParserKeyword("CALTRAC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CALTRAC");
  {
     ParserRecord record;
     {
        ParserItem item("IX1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CALTRAC::keywordName = "CALTRAC";
const std::string CALTRAC::IX1::itemName = "IX1";


CARFIN::CARFIN( ) : ParserKeyword("CARFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("CARFIN");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
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
     {
        ParserItem item("NWMAX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PARENT", ParserItem::itype::STRING);
        item.setDefault( std::string("GLOBAL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CARFIN::keywordName = "CARFIN";
const std::string CARFIN::NAME::itemName = "NAME";
const std::string CARFIN::I1::itemName = "I1";
const std::string CARFIN::I2::itemName = "I2";
const std::string CARFIN::J1::itemName = "J1";
const std::string CARFIN::J2::itemName = "J2";
const std::string CARFIN::K1::itemName = "K1";
const std::string CARFIN::K2::itemName = "K2";
const std::string CARFIN::NX::itemName = "NX";
const std::string CARFIN::NY::itemName = "NY";
const std::string CARFIN::NZ::itemName = "NZ";
const std::string CARFIN::NWMAX::itemName = "NWMAX";
const std::string CARFIN::PARENT::itemName = "PARENT";
const std::string CARFIN::PARENT::defaultValue = "GLOBAL";


CART::CART( ) : ParserKeyword("CART")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("CART");
}
const std::string CART::keywordName = "CART";


CBMOPTS::CBMOPTS( ) : ParserKeyword("CBMOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SRUNSPEC");
  clearDeckNames();
  addDeckName("CBMOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("ADSORPTION_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("TIMEDEP") );
        record.addItem(item);
     }
     {
        ParserItem item("ALLOW_WATER_FLOW", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("ALLOW_PERMEAB", ParserItem::itype::STRING);
        item.setDefault( std::string("NOKRMIX") );
        record.addItem(item);
     }
     {
        ParserItem item("COUNT_PASSES", ParserItem::itype::STRING);
        item.setDefault( std::string("NOPMREB") );
        record.addItem(item);
     }
     {
        ParserItem item("METHOD", ParserItem::itype::STRING);
        item.setDefault( std::string("PMSTD") );
        record.addItem(item);
     }
     {
        ParserItem item("SCALING_VALUE", ParserItem::itype::STRING);
        item.setDefault( std::string("PMSCAL") );
        record.addItem(item);
     }
     {
        ParserItem item("APPLICATION", ParserItem::itype::STRING);
        item.setDefault( std::string("PMPVK") );
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_CHOP", ParserItem::itype::STRING);
        item.setDefault( std::string("NOPMPCHP") );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_PORE_VOLUME", ParserItem::itype::DOUBLE);
        item.setDefault( double(5e-06) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CBMOPTS::keywordName = "CBMOPTS";
const std::string CBMOPTS::ADSORPTION_MODEL::itemName = "ADSORPTION_MODEL";
const std::string CBMOPTS::ADSORPTION_MODEL::defaultValue = "TIMEDEP";
const std::string CBMOPTS::ALLOW_WATER_FLOW::itemName = "ALLOW_WATER_FLOW";
const std::string CBMOPTS::ALLOW_WATER_FLOW::defaultValue = "YES";
const std::string CBMOPTS::ALLOW_PERMEAB::itemName = "ALLOW_PERMEAB";
const std::string CBMOPTS::ALLOW_PERMEAB::defaultValue = "NOKRMIX";
const std::string CBMOPTS::COUNT_PASSES::itemName = "COUNT_PASSES";
const std::string CBMOPTS::COUNT_PASSES::defaultValue = "NOPMREB";
const std::string CBMOPTS::METHOD::itemName = "METHOD";
const std::string CBMOPTS::METHOD::defaultValue = "PMSTD";
const std::string CBMOPTS::SCALING_VALUE::itemName = "SCALING_VALUE";
const std::string CBMOPTS::SCALING_VALUE::defaultValue = "PMSCAL";
const std::string CBMOPTS::APPLICATION::itemName = "APPLICATION";
const std::string CBMOPTS::APPLICATION::defaultValue = "PMPVK";
const std::string CBMOPTS::PRESSURE_CHOP::itemName = "PRESSURE_CHOP";
const std::string CBMOPTS::PRESSURE_CHOP::defaultValue = "NOPMPCHP";
const std::string CBMOPTS::MIN_PORE_VOLUME::itemName = "MIN_PORE_VOLUME";
const double CBMOPTS::MIN_PORE_VOLUME::defaultValue = 0.000005;


CECON::CECON( ) : ParserKeyword("CECON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CECON");
  {
     ParserRecord record;
     {
        ParserItem item("WELLNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WCUT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WGR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_PROCEDURE", ParserItem::itype::STRING);
        item.setDefault( std::string("CON") );
        record.addItem(item);
     }
     {
        ParserItem item("CHECK_STOPPED", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(-100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(-100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("FOLLOW_ON_WELL", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CECON::keywordName = "CECON";
const std::string CECON::WELLNAME::itemName = "WELLNAME";
const std::string CECON::I::itemName = "I";
const int CECON::I::defaultValue = 0;
const std::string CECON::J::itemName = "J";
const int CECON::J::defaultValue = 0;
const std::string CECON::K1::itemName = "K1";
const int CECON::K1::defaultValue = 0;
const std::string CECON::K2::itemName = "K2";
const int CECON::K2::defaultValue = 0;
const std::string CECON::MAX_WCUT::itemName = "MAX_WCUT";
const double CECON::MAX_WCUT::defaultValue = 0.000000;
const std::string CECON::MAX_GOR::itemName = "MAX_GOR";
const double CECON::MAX_GOR::defaultValue = 0.000000;
const std::string CECON::MAX_WGR::itemName = "MAX_WGR";
const double CECON::MAX_WGR::defaultValue = 0.000000;
const std::string CECON::WORKOVER_PROCEDURE::itemName = "WORKOVER_PROCEDURE";
const std::string CECON::WORKOVER_PROCEDURE::defaultValue = "CON";
const std::string CECON::CHECK_STOPPED::itemName = "CHECK_STOPPED";
const std::string CECON::CHECK_STOPPED::defaultValue = "NO";
const std::string CECON::MIN_OIL::itemName = "MIN_OIL";
const double CECON::MIN_OIL::defaultValue = -100000000000000000000.000000;
const std::string CECON::MIN_GAS::itemName = "MIN_GAS";
const double CECON::MIN_GAS::defaultValue = -100000000000000000000.000000;
const std::string CECON::FOLLOW_ON_WELL::itemName = "FOLLOW_ON_WELL";
const std::string CECON::FOLLOW_ON_WELL::defaultValue = "";


CECONT::CECONT( ) : ParserKeyword("CECONT")
{
  setSizeType(DOUBLE_SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CECONT");
  setDoubleRecordsKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("PROCEDURE", ParserItem::itype::STRING);
        item.setDefault( std::string("CON") );
        record.addItem(item);
     }
     {
        ParserItem item("CHECK_STOPPED_WELLS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
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
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TOTAL_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FREE_TRACER_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FREE_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SOL_TRACER_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SOL_TRACER_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CECONT::keywordName = "CECONT";
const std::string CECONT::WELL::itemName = "WELL";
const std::string CECONT::I::itemName = "I";
const int CECONT::I::defaultValue = 0;
const std::string CECONT::J::itemName = "J";
const int CECONT::J::defaultValue = 0;
const std::string CECONT::K_UPPER::itemName = "K_UPPER";
const int CECONT::K_UPPER::defaultValue = 0;
const std::string CECONT::K_LOWER::itemName = "K_LOWER";
const int CECONT::K_LOWER::defaultValue = 0;
const std::string CECONT::PROCEDURE::itemName = "PROCEDURE";
const std::string CECONT::PROCEDURE::defaultValue = "CON";
const std::string CECONT::CHECK_STOPPED_WELLS::itemName = "CHECK_STOPPED_WELLS";
const std::string CECONT::CHECK_STOPPED_WELLS::defaultValue = "NO";
const std::string CECONT::TRACER::itemName = "TRACER";
const std::string CECONT::MAX_TOTAL_TRACER_RATE::itemName = "MAX_TOTAL_TRACER_RATE";
const double CECONT::MAX_TOTAL_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string CECONT::MAX_TOTAL_TRACER_CONC::itemName = "MAX_TOTAL_TRACER_CONC";
const double CECONT::MAX_TOTAL_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string CECONT::MAX_FREE_TRACER_RATE::itemName = "MAX_FREE_TRACER_RATE";
const double CECONT::MAX_FREE_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string CECONT::MAX_FREE_TRACER_CONC::itemName = "MAX_FREE_TRACER_CONC";
const double CECONT::MAX_FREE_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string CECONT::MAX_SOL_TRACER_RATE::itemName = "MAX_SOL_TRACER_RATE";
const double CECONT::MAX_SOL_TRACER_RATE::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;
const std::string CECONT::MAX_SOL_TRACER_CONC::itemName = "MAX_SOL_TRACER_CONC";
const double CECONT::MAX_SOL_TRACER_CONC::defaultValue = 99999999999999996973312221251036165947450327545502362648241750950346848435554075534196338404706251868027512415973882408182135734368278484639385041047239877871023591066789981811181813306167128854888448.000000;


CIRCLE::CIRCLE( ) : ParserKeyword("CIRCLE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("CIRCLE");
}
const std::string CIRCLE::keywordName = "CIRCLE";


COAL::COAL( ) : ParserKeyword("COAL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("COAL");
}
const std::string COAL::keywordName = "COAL";


COALADS::COALADS( ) : ParserKeyword("COALADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NTCREG",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("COALADS");
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
const std::string COALADS::keywordName = "COALADS";
const std::string COALADS::DATA::itemName = "DATA";


COALNUM::COALNUM( ) : ParserKeyword("COALNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COALNUM");
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
const std::string COALNUM::keywordName = "COALNUM";
const std::string COALNUM::data::itemName = "data";


COALPP::COALPP( ) : ParserKeyword("COALPP")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NTCREG",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("COALPP");
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
const std::string COALPP::keywordName = "COALPP";
const std::string COALPP::DATA::itemName = "DATA";


COARSEN::COARSEN( ) : ParserKeyword("COARSEN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COARSEN");
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
const std::string COARSEN::keywordName = "COARSEN";
const std::string COARSEN::I1::itemName = "I1";
const std::string COARSEN::I2::itemName = "I2";
const std::string COARSEN::J1::itemName = "J1";
const std::string COARSEN::J2::itemName = "J2";
const std::string COARSEN::K1::itemName = "K1";
const std::string COARSEN::K2::itemName = "K2";
const std::string COARSEN::NX::itemName = "NX";
const std::string COARSEN::NY::itemName = "NY";
const std::string COARSEN::NZ::itemName = "NZ";


COLLAPSE::COLLAPSE( ) : ParserKeyword("COLLAPSE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COLLAPSE");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COLLAPSE::keywordName = "COLLAPSE";
const std::string COLLAPSE::VALUE::itemName = "VALUE";
const int COLLAPSE::VALUE::defaultValue = 1;


COLUMNS::COLUMNS( ) : ParserKeyword("COLUMNS")
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
  addDeckName("COLUMNS");
  {
     ParserRecord record;
     {
        ParserItem item("LEFT_MARGIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("RIGHT_MARGIN", ParserItem::itype::INT);
        item.setDefault( 132 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COLUMNS::keywordName = "COLUMNS";
const std::string COLUMNS::LEFT_MARGIN::itemName = "LEFT_MARGIN";
const int COLUMNS::LEFT_MARGIN::defaultValue = 1;
const std::string COLUMNS::RIGHT_MARGIN::itemName = "RIGHT_MARGIN";
const int COLUMNS::RIGHT_MARGIN::defaultValue = 132;


COMPDAT::COMPDAT( ) : ParserKeyword("COMPDAT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPDAT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
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
        ParserItem item("STATE", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("CONNECTION_TRANSMISSIBILITY_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Viscosity*ReservoirVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DIAMETER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("Kh", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Permeability*Length");
        record.addItem(item);
     }
     {
        ParserItem item("SKIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("D_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DIR", ParserItem::itype::STRING);
        item.setDefault( std::string("Z") );
        record.addItem(item);
     }
     {
        ParserItem item("PR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPDAT::keywordName = "COMPDAT";
const std::string COMPDAT::WELL::itemName = "WELL";
const std::string COMPDAT::I::itemName = "I";
const int COMPDAT::I::defaultValue = 0;
const std::string COMPDAT::J::itemName = "J";
const int COMPDAT::J::defaultValue = 0;
const std::string COMPDAT::K1::itemName = "K1";
const std::string COMPDAT::K2::itemName = "K2";
const std::string COMPDAT::STATE::itemName = "STATE";
const std::string COMPDAT::STATE::defaultValue = "OPEN";
const std::string COMPDAT::SAT_TABLE::itemName = "SAT_TABLE";
const int COMPDAT::SAT_TABLE::defaultValue = 0;
const std::string COMPDAT::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName = "CONNECTION_TRANSMISSIBILITY_FACTOR";
const std::string COMPDAT::DIAMETER::itemName = "DIAMETER";
const std::string COMPDAT::Kh::itemName = "Kh";
const double COMPDAT::Kh::defaultValue = -1.000000;
const std::string COMPDAT::SKIN::itemName = "SKIN";
const double COMPDAT::SKIN::defaultValue = 0.000000;
const std::string COMPDAT::D_FACTOR::itemName = "D_FACTOR";
const std::string COMPDAT::DIR::itemName = "DIR";
const std::string COMPDAT::DIR::defaultValue = "Z";
const std::string COMPDAT::PR::itemName = "PR";


COMPDATX::COMPDATX( ) : ParserKeyword("COMPDATX")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPDATL");
  addDeckName("COMPDATM");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
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
        ParserItem item("STATE", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("CONNECTION_TRANSMISSIBILITY_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Viscosity*ReservoirVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DIAMETER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("Kh", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Permeability*Length");
        record.addItem(item);
     }
     {
        ParserItem item("SKIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("D_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DIR", ParserItem::itype::STRING);
        item.setDefault( std::string("Z") );
        record.addItem(item);
     }
     {
        ParserItem item("PR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPDATX::keywordName = "COMPDATX";
const std::string COMPDATX::WELL::itemName = "WELL";
const std::string COMPDATX::LGR::itemName = "LGR";
const std::string COMPDATX::I::itemName = "I";
const int COMPDATX::I::defaultValue = 0;
const std::string COMPDATX::J::itemName = "J";
const int COMPDATX::J::defaultValue = 0;
const std::string COMPDATX::K1::itemName = "K1";
const std::string COMPDATX::K2::itemName = "K2";
const std::string COMPDATX::STATE::itemName = "STATE";
const std::string COMPDATX::STATE::defaultValue = "OPEN";
const std::string COMPDATX::SAT_TABLE::itemName = "SAT_TABLE";
const int COMPDATX::SAT_TABLE::defaultValue = 0;
const std::string COMPDATX::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName = "CONNECTION_TRANSMISSIBILITY_FACTOR";
const std::string COMPDATX::DIAMETER::itemName = "DIAMETER";
const std::string COMPDATX::Kh::itemName = "Kh";
const double COMPDATX::Kh::defaultValue = -1.000000;
const std::string COMPDATX::SKIN::itemName = "SKIN";
const double COMPDATX::SKIN::defaultValue = 0.000000;
const std::string COMPDATX::D_FACTOR::itemName = "D_FACTOR";
const std::string COMPDATX::DIR::itemName = "DIR";
const std::string COMPDATX::DIR::defaultValue = "Z";
const std::string COMPDATX::PR::itemName = "PR";


COMPFLSH::COMPFLSH( ) : ParserKeyword("COMPFLSH")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPFLSH");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_K", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_K", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("F1", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("F2", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("FLASH_PVTNUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPFLSH::keywordName = "COMPFLSH";
const std::string COMPFLSH::WELL::itemName = "WELL";
const std::string COMPFLSH::I::itemName = "I";
const int COMPFLSH::I::defaultValue = 0;
const std::string COMPFLSH::J::itemName = "J";
const int COMPFLSH::J::defaultValue = 0;
const std::string COMPFLSH::UPPER_K::itemName = "UPPER_K";
const int COMPFLSH::UPPER_K::defaultValue = 0;
const std::string COMPFLSH::LOWER_K::itemName = "LOWER_K";
const int COMPFLSH::LOWER_K::defaultValue = 0;
const std::string COMPFLSH::F1::itemName = "F1";
const double COMPFLSH::F1::defaultValue = 1.000000;
const std::string COMPFLSH::F2::itemName = "F2";
const double COMPFLSH::F2::defaultValue = 1.000000;
const std::string COMPFLSH::FLASH_PVTNUM::itemName = "FLASH_PVTNUM";
const int COMPFLSH::FLASH_PVTNUM::defaultValue = 0;


COMPIMB::COMPIMB( ) : ParserKeyword("COMPIMB")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPIMB");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
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
        ParserItem item("SAT_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPIMB::keywordName = "COMPIMB";
const std::string COMPIMB::WELL::itemName = "WELL";
const std::string COMPIMB::I::itemName = "I";
const int COMPIMB::I::defaultValue = 0;
const std::string COMPIMB::J::itemName = "J";
const int COMPIMB::J::defaultValue = 0;
const std::string COMPIMB::K1::itemName = "K1";
const std::string COMPIMB::K2::itemName = "K2";
const std::string COMPIMB::SAT_TABLE::itemName = "SAT_TABLE";
const int COMPIMB::SAT_TABLE::defaultValue = 0;


COMPINJK::COMPINJK( ) : ParserKeyword("COMPINJK")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPINJK");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("REL_PERM", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPINJK::keywordName = "COMPINJK";
const std::string COMPINJK::WELL::itemName = "WELL";
const std::string COMPINJK::I::itemName = "I";
const int COMPINJK::I::defaultValue = 0;
const std::string COMPINJK::J::itemName = "J";
const int COMPINJK::J::defaultValue = 0;
const std::string COMPINJK::K_UPPER::itemName = "K_UPPER";
const int COMPINJK::K_UPPER::defaultValue = 0;
const std::string COMPINJK::K_LOWER::itemName = "K_LOWER";
const int COMPINJK::K_LOWER::defaultValue = 0;
const std::string COMPINJK::REL_PERM::itemName = "REL_PERM";
const double COMPINJK::REL_PERM::defaultValue = 0.000000;


COMPLMPL::COMPLMPL( ) : ParserKeyword("COMPLMPL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPLMPL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_K", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_K", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("COMPLETION_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPLMPL::keywordName = "COMPLMPL";
const std::string COMPLMPL::WELL::itemName = "WELL";
const std::string COMPLMPL::GRID::itemName = "GRID";
const std::string COMPLMPL::GRID::defaultValue = "";
const std::string COMPLMPL::I::itemName = "I";
const int COMPLMPL::I::defaultValue = 0;
const std::string COMPLMPL::J::itemName = "J";
const int COMPLMPL::J::defaultValue = 0;
const std::string COMPLMPL::UPPER_K::itemName = "UPPER_K";
const int COMPLMPL::UPPER_K::defaultValue = 0;
const std::string COMPLMPL::LOWER_K::itemName = "LOWER_K";
const int COMPLMPL::LOWER_K::defaultValue = 0;
const std::string COMPLMPL::COMPLETION_NUMBER::itemName = "COMPLETION_NUMBER";


COMPLUMP::COMPLUMP( ) : ParserKeyword("COMPLUMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPLUMP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("N", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPLUMP::keywordName = "COMPLUMP";
const std::string COMPLUMP::WELL::itemName = "WELL";
const std::string COMPLUMP::I::itemName = "I";
const std::string COMPLUMP::J::itemName = "J";
const std::string COMPLUMP::K1::itemName = "K1";
const std::string COMPLUMP::K2::itemName = "K2";
const std::string COMPLUMP::N::itemName = "N";


COMPOFF::COMPOFF( ) : ParserKeyword("COMPOFF")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPOFF");
}
const std::string COMPOFF::keywordName = "COMPOFF";


COMPORD::COMPORD( ) : ParserKeyword("COMPORD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPORD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ORDER_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("TRACK") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPORD::keywordName = "COMPORD";
const std::string COMPORD::WELL::itemName = "WELL";
const std::string COMPORD::ORDER_TYPE::itemName = "ORDER_TYPE";
const std::string COMPORD::ORDER_TYPE::defaultValue = "TRACK";


COMPRIV::COMPRIV( ) : ParserKeyword("COMPRIV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPRIV");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
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
     addRecord( record );
  }
}
const std::string COMPRIV::keywordName = "COMPRIV";
const std::string COMPRIV::RIVER::itemName = "RIVER";
const std::string COMPRIV::I::itemName = "I";
const std::string COMPRIV::J::itemName = "J";
const std::string COMPRIV::K::itemName = "K";


COMPRP::COMPRP( ) : ParserKeyword("COMPRP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPRP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SWMIN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWMAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SGMIN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SGMAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPRP::keywordName = "COMPRP";
const std::string COMPRP::WELL::itemName = "WELL";
const std::string COMPRP::I::itemName = "I";
const int COMPRP::I::defaultValue = 0;
const std::string COMPRP::J::itemName = "J";
const int COMPRP::J::defaultValue = 0;
const std::string COMPRP::K_UPPER::itemName = "K_UPPER";
const int COMPRP::K_UPPER::defaultValue = 0;
const std::string COMPRP::K_LOWER::itemName = "K_LOWER";
const int COMPRP::K_LOWER::defaultValue = 0;
const std::string COMPRP::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";
const std::string COMPRP::SWMIN::itemName = "SWMIN";
const std::string COMPRP::SWMAX::itemName = "SWMAX";
const std::string COMPRP::SGMIN::itemName = "SGMIN";
const std::string COMPRP::SGMAX::itemName = "SGMAX";


COMPRPL::COMPRPL( ) : ParserKeyword("COMPRPL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPRPL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LOCAL_GRID", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SWMIN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SWMAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SGMIN", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SGMAX", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPRPL::keywordName = "COMPRPL";
const std::string COMPRPL::WELL::itemName = "WELL";
const std::string COMPRPL::LOCAL_GRID::itemName = "LOCAL_GRID";
const std::string COMPRPL::I::itemName = "I";
const int COMPRPL::I::defaultValue = 0;
const std::string COMPRPL::J::itemName = "J";
const int COMPRPL::J::defaultValue = 0;
const std::string COMPRPL::K_UPPER::itemName = "K_UPPER";
const int COMPRPL::K_UPPER::defaultValue = 0;
const std::string COMPRPL::K_LOWER::itemName = "K_LOWER";
const int COMPRPL::K_LOWER::defaultValue = 0;
const std::string COMPRPL::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";
const std::string COMPRPL::SWMIN::itemName = "SWMIN";
const std::string COMPRPL::SWMAX::itemName = "SWMAX";
const std::string COMPRPL::SGMIN::itemName = "SGMIN";
const std::string COMPRPL::SGMAX::itemName = "SGMAX";


COMPS::COMPS( ) : ParserKeyword("COMPS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("COMPS");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_COMPS", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPS::keywordName = "COMPS";
const std::string COMPS::NUM_COMPS::itemName = "NUM_COMPS";


COMPSEGL::COMPSEGL( ) : ParserKeyword("COMPSEGL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPSEGL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
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
        ParserItem item("BRANCH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_START", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_END", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("END_IJK", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CENTER_DEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("THERMAL_LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPSEGL::keywordName = "COMPSEGL";
const std::string COMPSEGL::WELL::itemName = "WELL";
const std::string COMPSEGL::GRID::itemName = "GRID";
const std::string COMPSEGL::I::itemName = "I";
const std::string COMPSEGL::J::itemName = "J";
const std::string COMPSEGL::K::itemName = "K";
const std::string COMPSEGL::BRANCH::itemName = "BRANCH";
const std::string COMPSEGL::DISTANCE_START::itemName = "DISTANCE_START";
const std::string COMPSEGL::DISTANCE_END::itemName = "DISTANCE_END";
const std::string COMPSEGL::DIRECTION::itemName = "DIRECTION";
const std::string COMPSEGL::END_IJK::itemName = "END_IJK";
const std::string COMPSEGL::CENTER_DEPTH::itemName = "CENTER_DEPTH";
const double COMPSEGL::CENTER_DEPTH::defaultValue = 0.000000;
const std::string COMPSEGL::THERMAL_LENGTH::itemName = "THERMAL_LENGTH";
const std::string COMPSEGL::SEGMENT_NUMBER::itemName = "SEGMENT_NUMBER";


COMPSEGS::COMPSEGS( ) : ParserKeyword("COMPSEGS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPSEGS");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
        ParserItem item("BRANCH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_START", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DISTANCE_END", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("END_IJK", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CENTER_DEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("THERMAL_LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPSEGS::keywordName = "COMPSEGS";
const std::string COMPSEGS::WELL::itemName = "WELL";
const std::string COMPSEGS::I::itemName = "I";
const std::string COMPSEGS::J::itemName = "J";
const std::string COMPSEGS::K::itemName = "K";
const std::string COMPSEGS::BRANCH::itemName = "BRANCH";
const std::string COMPSEGS::DISTANCE_START::itemName = "DISTANCE_START";
const std::string COMPSEGS::DISTANCE_END::itemName = "DISTANCE_END";
const std::string COMPSEGS::DIRECTION::itemName = "DIRECTION";
const std::string COMPSEGS::END_IJK::itemName = "END_IJK";
const std::string COMPSEGS::CENTER_DEPTH::itemName = "CENTER_DEPTH";
const double COMPSEGS::CENTER_DEPTH::defaultValue = 0.000000;
const std::string COMPSEGS::THERMAL_LENGTH::itemName = "THERMAL_LENGTH";
const std::string COMPSEGS::SEGMENT_NUMBER::itemName = "SEGMENT_NUMBER";


COMPVE::COMPVE( ) : ParserKeyword("COMPVE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPVE");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CVEFRAC", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DTOP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DBOT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("S_D", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GTOP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("GBOT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPVE::keywordName = "COMPVE";
const std::string COMPVE::WELL::itemName = "WELL";
const std::string COMPVE::I::itemName = "I";
const int COMPVE::I::defaultValue = 0;
const std::string COMPVE::J::itemName = "J";
const int COMPVE::J::defaultValue = 0;
const std::string COMPVE::K_UPPER::itemName = "K_UPPER";
const int COMPVE::K_UPPER::defaultValue = 0;
const std::string COMPVE::K_LOWER::itemName = "K_LOWER";
const int COMPVE::K_LOWER::defaultValue = 0;
const std::string COMPVE::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";
const std::string COMPVE::CVEFRAC::itemName = "CVEFRAC";
const std::string COMPVE::DTOP::itemName = "DTOP";
const std::string COMPVE::DBOT::itemName = "DBOT";
const std::string COMPVE::FLAG::itemName = "FLAG";
const std::string COMPVE::FLAG::defaultValue = "NO";
const std::string COMPVE::S_D::itemName = "S_D";
const double COMPVE::S_D::defaultValue = 0.000000;
const std::string COMPVE::GTOP::itemName = "GTOP";
const std::string COMPVE::GBOT::itemName = "GBOT";


COMPVEL::COMPVEL( ) : ParserKeyword("COMPVEL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPVEL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LOCAL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CVEFRAC", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DTOP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DBOT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("S_D", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GTOP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("GBOT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPVEL::keywordName = "COMPVEL";
const std::string COMPVEL::WELL::itemName = "WELL";
const std::string COMPVEL::LOCAL::itemName = "LOCAL";
const std::string COMPVEL::I::itemName = "I";
const int COMPVEL::I::defaultValue = 0;
const std::string COMPVEL::J::itemName = "J";
const int COMPVEL::J::defaultValue = 0;
const std::string COMPVEL::K_UPPER::itemName = "K_UPPER";
const int COMPVEL::K_UPPER::defaultValue = 0;
const std::string COMPVEL::K_LOWER::itemName = "K_LOWER";
const int COMPVEL::K_LOWER::defaultValue = 0;
const std::string COMPVEL::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";
const std::string COMPVEL::CVEFRAC::itemName = "CVEFRAC";
const std::string COMPVEL::DTOP::itemName = "DTOP";
const std::string COMPVEL::DBOT::itemName = "DBOT";
const std::string COMPVEL::FLAG::itemName = "FLAG";
const std::string COMPVEL::FLAG::defaultValue = "NO";
const std::string COMPVEL::S_D::itemName = "S_D";
const double COMPVEL::S_D::defaultValue = 0.000000;
const std::string COMPVEL::GTOP::itemName = "GTOP";
const std::string COMPVEL::GBOT::itemName = "GBOT";


CONNECTION_PROBE::CONNECTION_PROBE( ) : ParserKeyword("CONNECTION_PROBE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("CAPI");
  addDeckName("CCFR");
  addDeckName("CCIC");
  addDeckName("CCIR");
  addDeckName("CCIT");
  addDeckName("CCPC");
  addDeckName("CCPT");
  addDeckName("CDBF");
  addDeckName("CDFAC");
  addDeckName("CDSF");
  addDeckName("CDSM");
  addDeckName("CDSML");
  addDeckName("CGFR");
  addDeckName("CGFRF");
  addDeckName("CGFRL");
  addDeckName("CGFRS");
  addDeckName("CGFRU");
  addDeckName("CGIR");
  addDeckName("CGIRL");
  addDeckName("CGIT");
  addDeckName("CGITL");
  addDeckName("CGLR");
  addDeckName("CGLRL");
  addDeckName("CGOR");
  addDeckName("CGORL");
  addDeckName("CGPI");
  addDeckName("CGPP");
  addDeckName("CGPR");
  addDeckName("CGPRL");
  addDeckName("CGPT");
  addDeckName("CGPTF");
  addDeckName("CGPTL");
  addDeckName("CGPTS");
  addDeckName("CGQ");
  addDeckName("CLFR");
  addDeckName("CLFRL");
  addDeckName("CLPT");
  addDeckName("CLPTL");
  addDeckName("CNFR");
  addDeckName("CNIT");
  addDeckName("CNPT");
  addDeckName("COFR");
  addDeckName("COFRF");
  addDeckName("COFRL");
  addDeckName("COFRS");
  addDeckName("COFRU");
  addDeckName("COGR");
  addDeckName("COGRL");
  addDeckName("COIT");
  addDeckName("COITL");
  addDeckName("COPI");
  addDeckName("COPP");
  addDeckName("COPR");
  addDeckName("COPRL");
  addDeckName("COPT");
  addDeckName("COPTF");
  addDeckName("COPTL");
  addDeckName("COPTS");
  addDeckName("CPI");
  addDeckName("CPRL");
  addDeckName("CSFR");
  addDeckName("CSIC");
  addDeckName("CSIR");
  addDeckName("CSIT");
  addDeckName("CSPC");
  addDeckName("CSPR");
  addDeckName("CSPT");
  addDeckName("CTFAC");
  addDeckName("CTFRALK");
  addDeckName("CTFRANI");
  addDeckName("CTFRCAT");
  addDeckName("CTFRFOA");
  addDeckName("CTFRSUR");
  addDeckName("CTITALK");
  addDeckName("CTITANI");
  addDeckName("CTITCAT");
  addDeckName("CTITFOA");
  addDeckName("CTITSUR");
  addDeckName("CTPTALK");
  addDeckName("CTPTANI");
  addDeckName("CTPTCAT");
  addDeckName("CTPTFOA");
  addDeckName("CTPTSUR");
  addDeckName("CVFR");
  addDeckName("CVFRL");
  addDeckName("CVIR");
  addDeckName("CVIRL");
  addDeckName("CVIT");
  addDeckName("CVITL");
  addDeckName("CVPT");
  addDeckName("CVPTL");
  addDeckName("CWCT");
  addDeckName("CWCTL");
  addDeckName("CWFR");
  addDeckName("CWFRL");
  addDeckName("CWFRU");
  addDeckName("CWGR");
  addDeckName("CWGRL");
  addDeckName("CWIR");
  addDeckName("CWIRL");
  addDeckName("CWIT");
  addDeckName("CWITL");
  addDeckName("CWPI");
  addDeckName("CWPP");
  addDeckName("CWPR");
  addDeckName("CWPRL");
  addDeckName("CWPT");
  addDeckName("CWPTL");
  addDeckName("LCGFRU");
  addDeckName("LCOFRU");
  addDeckName("LCWFRU");
  addDeckName("WGIRL");
  addDeckName("WGITL");
  setMatchRegex("CU.+|CTFR.+|CTPR.+|CTPT.+|CTPC.+|CTIR.+|CTIT.+|CTIC.+|CTFR.+|CTPR.+|CTPT.+|CTPC.+|CTIR.+|CTIT.+|CTIC.+|CTIRF.+|CTIRS.+|CTPRF.+|CTPRS.+|CTITF.+|CTITS.+|CTPTF.+|CTPTS.+|CTICF.+|CTICS.+|CTPCF.+|CTPCS");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
     addRecord( record );
  }
}
const std::string CONNECTION_PROBE::keywordName = "CONNECTION_PROBE";
const std::string CONNECTION_PROBE::WELL::itemName = "WELL";
const std::string CONNECTION_PROBE::I::itemName = "I";
const std::string CONNECTION_PROBE::J::itemName = "J";
const std::string CONNECTION_PROBE::K::itemName = "K";


COORD::COORD( ) : ParserKeyword("COORD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COORD");
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
const std::string COORD::keywordName = "COORD";
const std::string COORD::data::itemName = "data";


COORDSYS::COORDSYS( ) : ParserKeyword("COORDSYS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("NUMRES","num",0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COORDSYS");
  {
     ParserRecord record;
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CIRCLE_COMPLETION", ParserItem::itype::STRING);
        item.setDefault( std::string("INCOMP") );
        record.addItem(item);
     }
     {
        ParserItem item("CONNECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("SEPARATE") );
        record.addItem(item);
     }
     {
        ParserItem item("R1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("R2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COORDSYS::keywordName = "COORDSYS";
const std::string COORDSYS::K1::itemName = "K1";
const std::string COORDSYS::K2::itemName = "K2";
const std::string COORDSYS::CIRCLE_COMPLETION::itemName = "CIRCLE_COMPLETION";
const std::string COORDSYS::CIRCLE_COMPLETION::defaultValue = "INCOMP";
const std::string COORDSYS::CONNECTION::itemName = "CONNECTION";
const std::string COORDSYS::CONNECTION::defaultValue = "SEPARATE";
const std::string COORDSYS::R1::itemName = "R1";
const int COORDSYS::R1::defaultValue = 0;
const std::string COORDSYS::R2::itemName = "R2";
const int COORDSYS::R2::defaultValue = 0;


COPY::COPY( ) : ParserKeyword("COPY")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("COPY");
  {
     ParserRecord record;
     {
        ParserItem item("src", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("target", ParserItem::itype::STRING);
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
const std::string COPY::keywordName = "COPY";
const std::string COPY::src::itemName = "src";
const std::string COPY::target::itemName = "target";
const std::string COPY::I1::itemName = "I1";
const std::string COPY::I2::itemName = "I2";
const std::string COPY::J1::itemName = "J1";
const std::string COPY::J2::itemName = "J2";
const std::string COPY::K1::itemName = "K1";
const std::string COPY::K2::itemName = "K2";


COPYBOX::COPYBOX( ) : ParserKeyword("COPYBOX")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("COPYBOX");
  {
     ParserRecord record;
     {
        ParserItem item("ARRAY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("IX1S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KY1S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KY2S", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX1D", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2D", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1D", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2D", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KY1D", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KY2D", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string COPYBOX::keywordName = "COPYBOX";
const std::string COPYBOX::ARRAY::itemName = "ARRAY";
const std::string COPYBOX::IX1S::itemName = "IX1S";
const std::string COPYBOX::IX2S::itemName = "IX2S";
const std::string COPYBOX::JY1S::itemName = "JY1S";
const std::string COPYBOX::JY2S::itemName = "JY2S";
const std::string COPYBOX::KY1S::itemName = "KY1S";
const std::string COPYBOX::KY2S::itemName = "KY2S";
const std::string COPYBOX::IX1D::itemName = "IX1D";
const std::string COPYBOX::IX2D::itemName = "IX2D";
const std::string COPYBOX::JY1D::itemName = "JY1D";
const std::string COPYBOX::JY2D::itemName = "JY2D";
const std::string COPYBOX::KY1D::itemName = "KY1D";
const std::string COPYBOX::KY2D::itemName = "KY2D";


COPYREG::COPYREG( ) : ParserKeyword("COPYREG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("COPYREG");
  {
     ParserRecord record;
     {
        ParserItem item("SRC_ARRAY", ParserItem::itype::STRING);
        item.setDescription("The 3D array we will fetch from");
        record.addItem(item);
     }
     {
        ParserItem item("TARGET_ARRAY", ParserItem::itype::STRING);
        item.setDescription("The name of the array which will be set");
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
const std::string COPYREG::keywordName = "COPYREG";
const std::string COPYREG::SRC_ARRAY::itemName = "SRC_ARRAY";
const std::string COPYREG::TARGET_ARRAY::itemName = "TARGET_ARRAY";
const std::string COPYREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string COPYREG::REGION_NAME::itemName = "REGION_NAME";
const std::string COPYREG::REGION_NAME::defaultValue = "M";


CPIFACT::CPIFACT( ) : ParserKeyword("CPIFACT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CPIFACT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MULT", ParserItem::itype::UDA);
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
        ParserItem item("C1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("C2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CPIFACT::keywordName = "CPIFACT";
const std::string CPIFACT::WELL::itemName = "WELL";
const std::string CPIFACT::MULT::itemName = "MULT";
const std::string CPIFACT::I::itemName = "I";
const std::string CPIFACT::J::itemName = "J";
const std::string CPIFACT::K::itemName = "K";
const std::string CPIFACT::C1::itemName = "C1";
const std::string CPIFACT::C2::itemName = "C2";


CPIFACTL::CPIFACTL( ) : ParserKeyword("CPIFACTL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CPIFACTL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MULT", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
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
        ParserItem item("C1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("C2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CPIFACTL::keywordName = "CPIFACTL";
const std::string CPIFACTL::WELL::itemName = "WELL";
const std::string CPIFACTL::MULT::itemName = "MULT";
const std::string CPIFACTL::LGR::itemName = "LGR";
const std::string CPIFACTL::I::itemName = "I";
const std::string CPIFACTL::J::itemName = "J";
const std::string CPIFACTL::K::itemName = "K";
const std::string CPIFACTL::C1::itemName = "C1";
const std::string CPIFACTL::C2::itemName = "C2";


CPR::CPR( ) : ParserKeyword("CPR")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("CPR");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
     addRecord( record );
  }
}
const std::string CPR::keywordName = "CPR";
const std::string CPR::WELL::itemName = "WELL";
const std::string CPR::I::itemName = "I";
const std::string CPR::J::itemName = "J";
const std::string CPR::K::itemName = "K";


CREF::CREF( ) : ParserKeyword("CREF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("CREF");
  {
     ParserRecord record;
     {
        ParserItem item("COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CREF::keywordName = "CREF";
const std::string CREF::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";


CREFS::CREFS( ) : ParserKeyword("CREFS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("CREFS");
  {
     ParserRecord record;
     {
        ParserItem item("COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CREFS::keywordName = "CREFS";
const std::string CREFS::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";


CRITPERM::CRITPERM( ) : ParserKeyword("CRITPERM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("CRITPERM");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Permeability");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CRITPERM::keywordName = "CRITPERM";
const std::string CRITPERM::VALUE::itemName = "VALUE";


CSKIN::CSKIN( ) : ParserKeyword("CSKIN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("CSKIN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_UPPER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_LOWER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("CONNECTION_SKIN_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string CSKIN::keywordName = "CSKIN";
const std::string CSKIN::WELL::itemName = "WELL";
const std::string CSKIN::I::itemName = "I";
const int CSKIN::I::defaultValue = 0;
const std::string CSKIN::J::itemName = "J";
const int CSKIN::J::defaultValue = 0;
const std::string CSKIN::K_UPPER::itemName = "K_UPPER";
const int CSKIN::K_UPPER::defaultValue = 0;
const std::string CSKIN::K_LOWER::itemName = "K_LOWER";
const int CSKIN::K_LOWER::defaultValue = 0;
const std::string CSKIN::CONNECTION_SKIN_FACTOR::itemName = "CONNECTION_SKIN_FACTOR";
const double CSKIN::CONNECTION_SKIN_FACTOR::defaultValue = 0.000000;


}
}
