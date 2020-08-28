#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/W.hpp>
namespace Opm {
namespace ParserKeywords {
WAGHYSTR::WAGHYSTR( ) : ParserKeyword("WAGHYSTR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("WAGHYSTR");
  {
     ParserRecord record;
     {
        ParserItem item("LANDS_PARAMETER", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SECONDARY_DRAINAGE_REDUCTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("RES_OIL", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("IMB_LINEAR_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("THREEPHASE_SAT_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("RES_OIL_MOD_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WAGHYSTR::keywordName = "WAGHYSTR";
const std::string WAGHYSTR::LANDS_PARAMETER::itemName = "LANDS_PARAMETER";
const std::string WAGHYSTR::SECONDARY_DRAINAGE_REDUCTION::itemName = "SECONDARY_DRAINAGE_REDUCTION";
const double WAGHYSTR::SECONDARY_DRAINAGE_REDUCTION::defaultValue = 0.000000;
const std::string WAGHYSTR::GAS_MODEL::itemName = "GAS_MODEL";
const std::string WAGHYSTR::GAS_MODEL::defaultValue = "YES";
const std::string WAGHYSTR::RES_OIL::itemName = "RES_OIL";
const std::string WAGHYSTR::RES_OIL::defaultValue = "YES";
const std::string WAGHYSTR::WATER_MODEL::itemName = "WATER_MODEL";
const std::string WAGHYSTR::WATER_MODEL::defaultValue = "YES";
const std::string WAGHYSTR::IMB_LINEAR_FRACTION::itemName = "IMB_LINEAR_FRACTION";
const double WAGHYSTR::IMB_LINEAR_FRACTION::defaultValue = 0.100000;
const std::string WAGHYSTR::THREEPHASE_SAT_LIMIT::itemName = "THREEPHASE_SAT_LIMIT";
const double WAGHYSTR::THREEPHASE_SAT_LIMIT::defaultValue = 0.001000;
const std::string WAGHYSTR::RES_OIL_MOD_FRACTION::itemName = "RES_OIL_MOD_FRACTION";
const double WAGHYSTR::RES_OIL_MOD_FRACTION::defaultValue = 1.000000;


WAITBAL::WAITBAL( ) : ParserKeyword("WAITBAL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WAITBAL");
  {
     ParserRecord record;
     {
        ParserItem item("WAIT_NETWORK_BALANCE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WAITBAL::keywordName = "WAITBAL";
const std::string WAITBAL::WAIT_NETWORK_BALANCE::itemName = "WAIT_NETWORK_BALANCE";
const std::string WAITBAL::WAIT_NETWORK_BALANCE::defaultValue = "NO";


WALKALIN::WALKALIN( ) : ParserKeyword("WALKALIN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WALKALIN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ALKALINE_CONCENTRAION", ParserItem::itype::UDA);
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WALKALIN::keywordName = "WALKALIN";
const std::string WALKALIN::WELL::itemName = "WELL";
const std::string WALKALIN::ALKALINE_CONCENTRAION::itemName = "ALKALINE_CONCENTRAION";


WALQCALC::WALQCALC( ) : ParserKeyword("WALQCALC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WALQCALC");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_DEF", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WALQCALC::keywordName = "WALQCALC";
const std::string WALQCALC::WELL::itemName = "WELL";
const std::string WALQCALC::ALQ_DEF::itemName = "ALQ_DEF";
const std::string WALQCALC::ALQ_DEF::defaultValue = "NONE";


WAPI::WAPI( ) : ParserKeyword("WAPI")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WAPI");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("API", ParserItem::itype::UDA);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WAPI::keywordName = "WAPI";
const std::string WAPI::WELL::itemName = "WELL";
const std::string WAPI::API::itemName = "API";


WARN::WARN( ) : ParserKeyword("WARN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("WARN");
}
const std::string WARN::keywordName = "WARN";


WATDENT::WATDENT( ) : ParserKeyword("WATDENT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("WATDENT");
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
        item.setDefault( double(0.000300) );
        item.push_backDimension("1/AbsoluteTemperature");
        record.addItem(item);
     }
     {
        ParserItem item("EXPANSION_COEFF_QUADRATIC", ParserItem::itype::DOUBLE);
        item.setDefault( double(3e-06) );
        item.push_backDimension("1/AbsoluteTemperature*AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WATDENT::keywordName = "WATDENT";
const std::string WATDENT::REFERENCE_TEMPERATURE::itemName = "REFERENCE_TEMPERATURE";
const double WATDENT::REFERENCE_TEMPERATURE::defaultValue = 293.150000;
const std::string WATDENT::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const double WATDENT::EXPANSION_COEFF_LINEAR::defaultValue = 0.000300;
const std::string WATDENT::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";
const double WATDENT::EXPANSION_COEFF_QUADRATIC::defaultValue = 0.000003;


WATER::WATER( ) : ParserKeyword("WATER")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WATER");
}
const std::string WATER::keywordName = "WATER";


WATVISCT::WATVISCT( ) : ParserKeyword("WATVISCT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("WATVISCT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WATVISCT::keywordName = "WATVISCT";
const std::string WATVISCT::DATA::itemName = "DATA";


WBHGLR::WBHGLR( ) : ParserKeyword("WBHGLR")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WBHGLR");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GLR_CUTBACK", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_GLR_CUTBACK_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("RATE_CUTBACK_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("RESV") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GLR_ELIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_ACTION", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_REMOVE_CUTBACKS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WBHGLR::keywordName = "WBHGLR";
const std::string WBHGLR::WELL::itemName = "WELL";
const std::string WBHGLR::MAX_GLR_CUTBACK::itemName = "MAX_GLR_CUTBACK";
const double WBHGLR::MAX_GLR_CUTBACK::defaultValue = 100000000000000000000.000000;
const std::string WBHGLR::MIN_GLR_CUTBACK_REVERSE::itemName = "MIN_GLR_CUTBACK_REVERSE";
const double WBHGLR::MIN_GLR_CUTBACK_REVERSE::defaultValue = 100000000000000000000.000000;
const std::string WBHGLR::RATE_CUTBACK_FACTOR::itemName = "RATE_CUTBACK_FACTOR";
const double WBHGLR::RATE_CUTBACK_FACTOR::defaultValue = 1.000000;
const std::string WBHGLR::PHASE::itemName = "PHASE";
const std::string WBHGLR::PHASE::defaultValue = "RESV";
const std::string WBHGLR::MAX_GLR_ELIMIT::itemName = "MAX_GLR_ELIMIT";
const double WBHGLR::MAX_GLR_ELIMIT::defaultValue = 100000000000000000000.000000;
const std::string WBHGLR::WORKOVER_ACTION::itemName = "WORKOVER_ACTION";
const std::string WBHGLR::WORKOVER_ACTION::defaultValue = "NONE";
const std::string WBHGLR::WORKOVER_REMOVE_CUTBACKS::itemName = "WORKOVER_REMOVE_CUTBACKS";
const std::string WBHGLR::WORKOVER_REMOVE_CUTBACKS::defaultValue = "NO";


WBOREVOL::WBOREVOL( ) : ParserKeyword("WBOREVOL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WBOREVOL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLBORE_VOL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-05) );
        item.push_backDimension("Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("START_BHP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WBOREVOL::keywordName = "WBOREVOL";
const std::string WBOREVOL::WELL::itemName = "WELL";
const std::string WBOREVOL::WELLBORE_VOL::itemName = "WELLBORE_VOL";
const double WBOREVOL::WELLBORE_VOL::defaultValue = 0.000010;
const std::string WBOREVOL::START_BHP::itemName = "START_BHP";


WCALCVAL::WCALCVAL( ) : ParserKeyword("WCALCVAL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCALCVAL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CALORIFIC_GAS_VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Energy/GasSurfaceVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCALCVAL::keywordName = "WCALCVAL";
const std::string WCALCVAL::WELL::itemName = "WELL";
const std::string WCALCVAL::CALORIFIC_GAS_VALUE::itemName = "CALORIFIC_GAS_VALUE";


WCONHIST::WCONHIST( ) : ParserKeyword("WCONHIST")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONHIST");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ORAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("WRAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GRAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("LIFT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("THP", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("BHP", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("NGLRAT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONHIST::keywordName = "WCONHIST";
const std::string WCONHIST::WELL::itemName = "WELL";
const std::string WCONHIST::STATUS::itemName = "STATUS";
const std::string WCONHIST::STATUS::defaultValue = "OPEN";
const std::string WCONHIST::CMODE::itemName = "CMODE";
const std::string WCONHIST::ORAT::itemName = "ORAT";
const UDAValue WCONHIST::ORAT::defaultValue = UDAValue(0.000000);
const std::string WCONHIST::WRAT::itemName = "WRAT";
const UDAValue WCONHIST::WRAT::defaultValue = UDAValue(0.000000);
const std::string WCONHIST::GRAT::itemName = "GRAT";
const UDAValue WCONHIST::GRAT::defaultValue = UDAValue(0.000000);
const std::string WCONHIST::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONHIST::VFP_TABLE::defaultValue = 0;
const std::string WCONHIST::LIFT::itemName = "LIFT";
const double WCONHIST::LIFT::defaultValue = 0.000000;
const std::string WCONHIST::THP::itemName = "THP";
const UDAValue WCONHIST::THP::defaultValue = UDAValue(0.000000);
const std::string WCONHIST::BHP::itemName = "BHP";
const UDAValue WCONHIST::BHP::defaultValue = UDAValue(0.000000);
const std::string WCONHIST::NGLRAT::itemName = "NGLRAT";
const double WCONHIST::NGLRAT::defaultValue = 0.000000;


WCONINJ::WCONINJ( ) : ParserKeyword("WCONINJ")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJ");
}
const std::string WCONINJ::keywordName = "WCONINJ";


WCONINJE::WCONINJE( ) : ParserKeyword("WCONINJE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJE");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RATE", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("RESV", ParserItem::itype::UDA);
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("BHP", ParserItem::itype::UDA);
        item.setDefault( UDAValue(6895.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("THP", ParserItem::itype::UDA);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VAPOIL_C", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_STEAM_RATIO", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_OIL_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_WATER_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_GAS_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_STEAM_RATIO", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONINJE::keywordName = "WCONINJE";
const std::string WCONINJE::WELL::itemName = "WELL";
const std::string WCONINJE::TYPE::itemName = "TYPE";
const std::string WCONINJE::STATUS::itemName = "STATUS";
const std::string WCONINJE::STATUS::defaultValue = "OPEN";
const std::string WCONINJE::CMODE::itemName = "CMODE";
const std::string WCONINJE::RATE::itemName = "RATE";
const std::string WCONINJE::RESV::itemName = "RESV";
const std::string WCONINJE::BHP::itemName = "BHP";
const UDAValue WCONINJE::BHP::defaultValue = UDAValue(6895.000000);
const std::string WCONINJE::THP::itemName = "THP";
const std::string WCONINJE::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONINJE::VFP_TABLE::defaultValue = 0;
const std::string WCONINJE::VAPOIL_C::itemName = "VAPOIL_C";
const double WCONINJE::VAPOIL_C::defaultValue = 0.000000;
const std::string WCONINJE::GAS_STEAM_RATIO::itemName = "GAS_STEAM_RATIO";
const double WCONINJE::GAS_STEAM_RATIO::defaultValue = 0.000000;
const std::string WCONINJE::SURFACE_OIL_FRACTION::itemName = "SURFACE_OIL_FRACTION";
const double WCONINJE::SURFACE_OIL_FRACTION::defaultValue = 0.000000;
const std::string WCONINJE::SURFACE_WATER_FRACTION::itemName = "SURFACE_WATER_FRACTION";
const double WCONINJE::SURFACE_WATER_FRACTION::defaultValue = 0.000000;
const std::string WCONINJE::SURFACE_GAS_FRACTION::itemName = "SURFACE_GAS_FRACTION";
const double WCONINJE::SURFACE_GAS_FRACTION::defaultValue = 0.000000;
const std::string WCONINJE::OIL_STEAM_RATIO::itemName = "OIL_STEAM_RATIO";
const double WCONINJE::OIL_STEAM_RATIO::defaultValue = 0.000000;


WCONINJH::WCONINJH( ) : ParserKeyword("WCONINJH")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJH");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("RATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("ContextDependent");
        record.addItem(item);
     }
     {
        ParserItem item("BHP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("THP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VAPOIL_C", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_OIL_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_WATER_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("SURFACE_GAS_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        item.setDefault( std::string("RATE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONINJH::keywordName = "WCONINJH";
const std::string WCONINJH::WELL::itemName = "WELL";
const std::string WCONINJH::TYPE::itemName = "TYPE";
const std::string WCONINJH::STATUS::itemName = "STATUS";
const std::string WCONINJH::STATUS::defaultValue = "OPEN";
const std::string WCONINJH::RATE::itemName = "RATE";
const std::string WCONINJH::BHP::itemName = "BHP";
const std::string WCONINJH::THP::itemName = "THP";
const std::string WCONINJH::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONINJH::VFP_TABLE::defaultValue = 0;
const std::string WCONINJH::VAPOIL_C::itemName = "VAPOIL_C";
const double WCONINJH::VAPOIL_C::defaultValue = 0.000000;
const std::string WCONINJH::SURFACE_OIL_FRACTION::itemName = "SURFACE_OIL_FRACTION";
const double WCONINJH::SURFACE_OIL_FRACTION::defaultValue = 0.000000;
const std::string WCONINJH::SURFACE_WATER_FRACTION::itemName = "SURFACE_WATER_FRACTION";
const double WCONINJH::SURFACE_WATER_FRACTION::defaultValue = 0.000000;
const std::string WCONINJH::SURFACE_GAS_FRACTION::itemName = "SURFACE_GAS_FRACTION";
const double WCONINJH::SURFACE_GAS_FRACTION::defaultValue = 0.000000;
const std::string WCONINJH::CMODE::itemName = "CMODE";
const std::string WCONINJH::CMODE::defaultValue = "RATE";


WCONINJP::WCONINJP( ) : ParserKeyword("WCONINJP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJP");
  {
     ParserRecord record;
     {
        ParserItem item("PATTERN_WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INJECTOR_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("BHP_MAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(6895.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("THP_MAX", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VOIDAGE_TARGET_MULTIPLIER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PROD_FRACTION", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("FIPNUM_VALUE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONINJP::keywordName = "WCONINJP";
const std::string WCONINJP::PATTERN_WELL::itemName = "PATTERN_WELL";
const std::string WCONINJP::INJECTOR_TYPE::itemName = "INJECTOR_TYPE";
const std::string WCONINJP::STATUS::itemName = "STATUS";
const std::string WCONINJP::STATUS::defaultValue = "OPEN";
const std::string WCONINJP::BHP_MAX::itemName = "BHP_MAX";
const double WCONINJP::BHP_MAX::defaultValue = 6895.000000;
const std::string WCONINJP::THP_MAX::itemName = "THP_MAX";
const std::string WCONINJP::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONINJP::VFP_TABLE::defaultValue = 0;
const std::string WCONINJP::VOIDAGE_TARGET_MULTIPLIER::itemName = "VOIDAGE_TARGET_MULTIPLIER";
const double WCONINJP::VOIDAGE_TARGET_MULTIPLIER::defaultValue = 1.000000;
const std::string WCONINJP::OIL_FRACTION::itemName = "OIL_FRACTION";
const double WCONINJP::OIL_FRACTION::defaultValue = 0.000000;
const std::string WCONINJP::WATER_FRACTION::itemName = "WATER_FRACTION";
const double WCONINJP::WATER_FRACTION::defaultValue = 0.000000;
const std::string WCONINJP::GAS_FRACTION::itemName = "GAS_FRACTION";
const double WCONINJP::GAS_FRACTION::defaultValue = 0.000000;
const std::string WCONINJP::WELL::itemName = "WELL";
const std::string WCONINJP::PROD_FRACTION::itemName = "PROD_FRACTION";
const std::string WCONINJP::FIPNUM_VALUE::itemName = "FIPNUM_VALUE";
const int WCONINJP::FIPNUM_VALUE::defaultValue = 0;


WCONPROD::WCONPROD( ) : ParserKeyword("WCONPROD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONPROD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ORAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("WRAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("GRAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("LRAT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("RESV", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("BHP", ParserItem::itype::UDA);
        item.setDefault( UDAValue(1.013250) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("THP", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("Pressure");
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
        ParserItem item("E300_ITEM13", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM14", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM15", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM16", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM17", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM18", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM19", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("E300_ITEM20", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONPROD::keywordName = "WCONPROD";
const std::string WCONPROD::WELL::itemName = "WELL";
const std::string WCONPROD::STATUS::itemName = "STATUS";
const std::string WCONPROD::STATUS::defaultValue = "OPEN";
const std::string WCONPROD::CMODE::itemName = "CMODE";
const std::string WCONPROD::ORAT::itemName = "ORAT";
const UDAValue WCONPROD::ORAT::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::WRAT::itemName = "WRAT";
const UDAValue WCONPROD::WRAT::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::GRAT::itemName = "GRAT";
const UDAValue WCONPROD::GRAT::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::LRAT::itemName = "LRAT";
const UDAValue WCONPROD::LRAT::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::RESV::itemName = "RESV";
const UDAValue WCONPROD::RESV::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::BHP::itemName = "BHP";
const UDAValue WCONPROD::BHP::defaultValue = UDAValue(1.013250);
const std::string WCONPROD::THP::itemName = "THP";
const UDAValue WCONPROD::THP::defaultValue = UDAValue(0.000000);
const std::string WCONPROD::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONPROD::VFP_TABLE::defaultValue = 0;
const std::string WCONPROD::ALQ::itemName = "ALQ";
const double WCONPROD::ALQ::defaultValue = 0.000000;
const std::string WCONPROD::E300_ITEM13::itemName = "E300_ITEM13";
const std::string WCONPROD::E300_ITEM14::itemName = "E300_ITEM14";
const std::string WCONPROD::E300_ITEM15::itemName = "E300_ITEM15";
const std::string WCONPROD::E300_ITEM16::itemName = "E300_ITEM16";
const std::string WCONPROD::E300_ITEM17::itemName = "E300_ITEM17";
const std::string WCONPROD::E300_ITEM18::itemName = "E300_ITEM18";
const std::string WCONPROD::E300_ITEM19::itemName = "E300_ITEM19";
const std::string WCONPROD::E300_ITEM20::itemName = "E300_ITEM20";


WCUTBACK::WCUTBACK( ) : ParserKeyword("WCUTBACK")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCUTBACK");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WCT_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("GOR_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("WGR_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("RATE_CUTBACK", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_LIMIT_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("WCT_LIMIT_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GOR_LIMIT_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GLR_LIMIT_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WGR_LIMIT_REVERSE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_REMOVE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCUTBACK::keywordName = "WCUTBACK";
const std::string WCUTBACK::WELL::itemName = "WELL";
const std::string WCUTBACK::WCT_LIMIT::itemName = "WCT_LIMIT";
const std::string WCUTBACK::GOR_LIMIT::itemName = "GOR_LIMIT";
const std::string WCUTBACK::WGR_LIMIT::itemName = "WGR_LIMIT";
const std::string WCUTBACK::RATE_CUTBACK::itemName = "RATE_CUTBACK";
const double WCUTBACK::RATE_CUTBACK::defaultValue = 1.000000;
const std::string WCUTBACK::PHASE::itemName = "PHASE";
const std::string WCUTBACK::PRESSURE_LIMIT::itemName = "PRESSURE_LIMIT";
const double WCUTBACK::PRESSURE_LIMIT::defaultValue = 0.000000;
const std::string WCUTBACK::PRESSURE_LIMIT_REVERSE::itemName = "PRESSURE_LIMIT_REVERSE";
const double WCUTBACK::PRESSURE_LIMIT_REVERSE::defaultValue = 0.000000;
const std::string WCUTBACK::WCT_LIMIT_REVERSE::itemName = "WCT_LIMIT_REVERSE";
const double WCUTBACK::WCT_LIMIT_REVERSE::defaultValue = 0.000000;
const std::string WCUTBACK::GOR_LIMIT_REVERSE::itemName = "GOR_LIMIT_REVERSE";
const double WCUTBACK::GOR_LIMIT_REVERSE::defaultValue = 0.000000;
const std::string WCUTBACK::GLR_LIMIT_REVERSE::itemName = "GLR_LIMIT_REVERSE";
const double WCUTBACK::GLR_LIMIT_REVERSE::defaultValue = 0.000000;
const std::string WCUTBACK::WGR_LIMIT_REVERSE::itemName = "WGR_LIMIT_REVERSE";
const double WCUTBACK::WGR_LIMIT_REVERSE::defaultValue = 0.000000;
const std::string WCUTBACK::WORKOVER_REMOVE::itemName = "WORKOVER_REMOVE";
const std::string WCUTBACK::WORKOVER_REMOVE::defaultValue = "NO";


WCUTBACT::WCUTBACT( ) : ParserKeyword("WCUTBACT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCUTBACT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RATE_CUTBACK", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_REMOVE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCUTBACT::keywordName = "WCUTBACT";
const std::string WCUTBACT::WELL::itemName = "WELL";
const std::string WCUTBACT::RATE_CUTBACK::itemName = "RATE_CUTBACK";
const double WCUTBACT::RATE_CUTBACK::defaultValue = 1.000000;
const std::string WCUTBACT::PHASE::itemName = "PHASE";
const std::string WCUTBACT::WORKOVER_REMOVE::itemName = "WORKOVER_REMOVE";
const std::string WCUTBACT::WORKOVER_REMOVE::defaultValue = "NO";


WCYCLE::WCYCLE( ) : ParserKeyword("WCYCLE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCYCLE");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ON_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("OFF_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("START_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TIMESTEP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("CONTROLLED_TIMESTEP", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCYCLE::keywordName = "WCYCLE";
const std::string WCYCLE::WELL::itemName = "WELL";
const std::string WCYCLE::ON_TIME::itemName = "ON_TIME";
const double WCYCLE::ON_TIME::defaultValue = 0.000000;
const std::string WCYCLE::OFF_TIME::itemName = "OFF_TIME";
const double WCYCLE::OFF_TIME::defaultValue = 0.000000;
const std::string WCYCLE::START_TIME::itemName = "START_TIME";
const double WCYCLE::START_TIME::defaultValue = 0.000000;
const std::string WCYCLE::MAX_TIMESTEP::itemName = "MAX_TIMESTEP";
const double WCYCLE::MAX_TIMESTEP::defaultValue = 0.000000;
const std::string WCYCLE::CONTROLLED_TIMESTEP::itemName = "CONTROLLED_TIMESTEP";
const std::string WCYCLE::CONTROLLED_TIMESTEP::defaultValue = "NO";


WDFAC::WDFAC( ) : ParserKeyword("WDFAC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WDFAC");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DFACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time/GasSurfaceVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WDFAC::keywordName = "WDFAC";
const std::string WDFAC::WELL::itemName = "WELL";
const std::string WDFAC::DFACTOR::itemName = "DFACTOR";
const double WDFAC::DFACTOR::defaultValue = 0.000000;


WDFACCOR::WDFACCOR( ) : ParserKeyword("WDFACCOR")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WDFACCOR");
  {
     ParserRecord record;
     {
        ParserItem item("WELLNAME", ParserItem::itype::STRING);
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
     addRecord( record );
  }
}
const std::string WDFACCOR::keywordName = "WDFACCOR";
const std::string WDFACCOR::WELLNAME::itemName = "WELLNAME";
const std::string WDFACCOR::A::itemName = "A";
const double WDFACCOR::A::defaultValue = 0.000000;
const std::string WDFACCOR::B::itemName = "B";
const double WDFACCOR::B::defaultValue = 0.000000;
const std::string WDFACCOR::C::itemName = "C";
const double WDFACCOR::C::defaultValue = 0.000000;


WDRILPRI::WDRILPRI( ) : ParserKeyword("WDRILPRI")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WDRILPRI");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRIORITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("DRILLING_UNIT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WDRILPRI::keywordName = "WDRILPRI";
const std::string WDRILPRI::WELL::itemName = "WELL";
const std::string WDRILPRI::PRIORITY::itemName = "PRIORITY";
const double WDRILPRI::PRIORITY::defaultValue = -1.000000;
const std::string WDRILPRI::DRILLING_UNIT::itemName = "DRILLING_UNIT";
const int WDRILPRI::DRILLING_UNIT::defaultValue = 0;


WDRILRES::WDRILRES( ) : ParserKeyword("WDRILRES")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WDRILRES");
}
const std::string WDRILRES::keywordName = "WDRILRES";


WDRILTIM::WDRILTIM( ) : ParserKeyword("WDRILTIM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WDRILTIM");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DRILL_TIME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_CLOSE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("COMPARTMENT", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WDRILTIM::keywordName = "WDRILTIM";
const std::string WDRILTIM::WELL::itemName = "WELL";
const std::string WDRILTIM::DRILL_TIME::itemName = "DRILL_TIME";
const std::string WDRILTIM::WORKOVER_CLOSE::itemName = "WORKOVER_CLOSE";
const std::string WDRILTIM::COMPARTMENT::itemName = "COMPARTMENT";


WECON::WECON( ) : ParserKeyword("WECON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WECON");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_OIL_PRODUCTION", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_GAS_PRODUCTION", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WATER_CUT", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GAS_OIL_RATIO", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WATER_GAS_RATIO", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_RATIO_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("END_RUN_FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("FOLLOW_ON_WELL", ParserItem::itype::STRING);
        item.setDefault( std::string("'") );
        record.addItem(item);
     }
     {
        ParserItem item("LIMITED_QUANTITY", ParserItem::itype::STRING);
        item.setDefault( std::string("RATE") );
        record.addItem(item);
     }
     {
        ParserItem item("SECOND_MAX_WATER_CUT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER_SECOND_WATER_CUT_LIMIT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GAS_LIQUID_RATIO", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_LIQUID_PRODCUTION_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_RES_FLUID_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WECON::keywordName = "WECON";
const std::string WECON::WELL::itemName = "WELL";
const std::string WECON::MIN_OIL_PRODUCTION::itemName = "MIN_OIL_PRODUCTION";
const UDAValue WECON::MIN_OIL_PRODUCTION::defaultValue = UDAValue(0.000000);
const std::string WECON::MIN_GAS_PRODUCTION::itemName = "MIN_GAS_PRODUCTION";
const UDAValue WECON::MIN_GAS_PRODUCTION::defaultValue = UDAValue(0.000000);
const std::string WECON::MAX_WATER_CUT::itemName = "MAX_WATER_CUT";
const UDAValue WECON::MAX_WATER_CUT::defaultValue = UDAValue(0.000000);
const std::string WECON::MAX_GAS_OIL_RATIO::itemName = "MAX_GAS_OIL_RATIO";
const UDAValue WECON::MAX_GAS_OIL_RATIO::defaultValue = UDAValue(0.000000);
const std::string WECON::MAX_WATER_GAS_RATIO::itemName = "MAX_WATER_GAS_RATIO";
const UDAValue WECON::MAX_WATER_GAS_RATIO::defaultValue = UDAValue(0.000000);
const std::string WECON::WORKOVER_RATIO_LIMIT::itemName = "WORKOVER_RATIO_LIMIT";
const std::string WECON::WORKOVER_RATIO_LIMIT::defaultValue = "NONE";
const std::string WECON::END_RUN_FLAG::itemName = "END_RUN_FLAG";
const std::string WECON::END_RUN_FLAG::defaultValue = "NO";
const std::string WECON::FOLLOW_ON_WELL::itemName = "FOLLOW_ON_WELL";
const std::string WECON::FOLLOW_ON_WELL::defaultValue = "'";
const std::string WECON::LIMITED_QUANTITY::itemName = "LIMITED_QUANTITY";
const std::string WECON::LIMITED_QUANTITY::defaultValue = "RATE";
const std::string WECON::SECOND_MAX_WATER_CUT::itemName = "SECOND_MAX_WATER_CUT";
const double WECON::SECOND_MAX_WATER_CUT::defaultValue = 0.000000;
const std::string WECON::WORKOVER_SECOND_WATER_CUT_LIMIT::itemName = "WORKOVER_SECOND_WATER_CUT_LIMIT";
const std::string WECON::MAX_GAS_LIQUID_RATIO::itemName = "MAX_GAS_LIQUID_RATIO";
const double WECON::MAX_GAS_LIQUID_RATIO::defaultValue = 0.000000;
const std::string WECON::MIN_LIQUID_PRODCUTION_RATE::itemName = "MIN_LIQUID_PRODCUTION_RATE";
const double WECON::MIN_LIQUID_PRODCUTION_RATE::defaultValue = 0.000000;
const std::string WECON::MAX_TEMP::itemName = "MAX_TEMP";
const std::string WECON::MIN_RES_FLUID_RATE::itemName = "MIN_RES_FLUID_RATE";
const double WECON::MIN_RES_FLUID_RATE::defaultValue = 0.000000;


WECONINJ::WECONINJ( ) : ParserKeyword("WECONINJ")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WECONINJ");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MIN_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        item.setDefault( std::string("RATE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WECONINJ::keywordName = "WECONINJ";
const std::string WECONINJ::WELL::itemName = "WELL";
const std::string WECONINJ::MIN_RATE::itemName = "MIN_RATE";
const double WECONINJ::MIN_RATE::defaultValue = 0.000000;
const std::string WECONINJ::QUANTITY::itemName = "QUANTITY";
const std::string WECONINJ::QUANTITY::defaultValue = "RATE";


WECONT::WECONT( ) : ParserKeyword("WECONT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WECONT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WORKOVER", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("END_RUN_FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("FOLLOW_ON_WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WECONT::keywordName = "WECONT";
const std::string WECONT::WELL::itemName = "WELL";
const std::string WECONT::WORKOVER::itemName = "WORKOVER";
const std::string WECONT::WORKOVER::defaultValue = "NONE";
const std::string WECONT::END_RUN_FLAG::itemName = "END_RUN_FLAG";
const std::string WECONT::END_RUN_FLAG::defaultValue = "NO";
const std::string WECONT::FOLLOW_ON_WELL::itemName = "FOLLOW_ON_WELL";


WEFAC::WEFAC( ) : ParserKeyword("WEFAC")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WEFAC");
  {
     ParserRecord record;
     {
        ParserItem item("WELLNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("EFFICIENCY_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("EXTENDED_NETWORK_OPT", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WEFAC::keywordName = "WEFAC";
const std::string WEFAC::WELLNAME::itemName = "WELLNAME";
const std::string WEFAC::EFFICIENCY_FACTOR::itemName = "EFFICIENCY_FACTOR";
const double WEFAC::EFFICIENCY_FACTOR::defaultValue = 1.000000;
const std::string WEFAC::EXTENDED_NETWORK_OPT::itemName = "EXTENDED_NETWORK_OPT";
const std::string WEFAC::EXTENDED_NETWORK_OPT::defaultValue = "YES";


WELCNTL::WELCNTL( ) : ParserKeyword("WELCNTL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELCNTL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELCNTL::keywordName = "WELCNTL";
const std::string WELCNTL::WELL::itemName = "WELL";
const std::string WELCNTL::CMODE::itemName = "CMODE";
const std::string WELCNTL::NEW_VALUE::itemName = "NEW_VALUE";


WELDEBUG::WELDEBUG( ) : ParserKeyword("WELDEBUG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELDEBUG");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DEBUG_FLAG", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELDEBUG::keywordName = "WELDEBUG";
const std::string WELDEBUG::WELL::itemName = "WELL";
const std::string WELDEBUG::DEBUG_FLAG::itemName = "DEBUG_FLAG";


WELDRAW::WELDRAW( ) : ParserKeyword("WELDRAW")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELDRAW");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_DRAW", ParserItem::itype::UDA);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("USE_LIMIT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("GRID_BLOCKS", ParserItem::itype::STRING);
        item.setDefault( std::string("AVG") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELDRAW::keywordName = "WELDRAW";
const std::string WELDRAW::WELL::itemName = "WELL";
const std::string WELDRAW::MAX_DRAW::itemName = "MAX_DRAW";
const std::string WELDRAW::PHASE::itemName = "PHASE";
const std::string WELDRAW::USE_LIMIT::itemName = "USE_LIMIT";
const std::string WELDRAW::USE_LIMIT::defaultValue = "NO";
const std::string WELDRAW::GRID_BLOCKS::itemName = "GRID_BLOCKS";
const std::string WELDRAW::GRID_BLOCKS::defaultValue = "AVG";


WELEVNT::WELEVNT( ) : ParserKeyword("WELEVNT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELEVNT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WPWEM_VALUE", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELEVNT::keywordName = "WELEVNT";
const std::string WELEVNT::WELL::itemName = "WELL";
const std::string WELEVNT::WPWEM_VALUE::itemName = "WPWEM_VALUE";


WELLDIMS::WELLDIMS( ) : ParserKeyword("WELLDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WELLDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAXWELLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAXCONN", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAXGROUPS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GROUPSIZE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_STAGES", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_STREAMS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_MIXTURES", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SEPARATORS", ParserItem::itype::INT);
        item.setDefault( 4 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_MIXTURE_ITEMS", ParserItem::itype::INT);
        item.setDefault( 3 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_COMPLETION_X", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WELLIST_PR_WELL", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_DYNAMIC_WELLIST", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SECONDARY_WELLS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELLDIMS::keywordName = "WELLDIMS";
const std::string WELLDIMS::MAXWELLS::itemName = "MAXWELLS";
const int WELLDIMS::MAXWELLS::defaultValue = 0;
const std::string WELLDIMS::MAXCONN::itemName = "MAXCONN";
const int WELLDIMS::MAXCONN::defaultValue = 0;
const std::string WELLDIMS::MAXGROUPS::itemName = "MAXGROUPS";
const int WELLDIMS::MAXGROUPS::defaultValue = 0;
const std::string WELLDIMS::MAX_GROUPSIZE::itemName = "MAX_GROUPSIZE";
const int WELLDIMS::MAX_GROUPSIZE::defaultValue = 0;
const std::string WELLDIMS::MAX_STAGES::itemName = "MAX_STAGES";
const int WELLDIMS::MAX_STAGES::defaultValue = 5;
const std::string WELLDIMS::MAX_STREAMS::itemName = "MAX_STREAMS";
const int WELLDIMS::MAX_STREAMS::defaultValue = 10;
const std::string WELLDIMS::MAX_MIXTURES::itemName = "MAX_MIXTURES";
const int WELLDIMS::MAX_MIXTURES::defaultValue = 5;
const std::string WELLDIMS::MAX_SEPARATORS::itemName = "MAX_SEPARATORS";
const int WELLDIMS::MAX_SEPARATORS::defaultValue = 4;
const std::string WELLDIMS::MAX_MIXTURE_ITEMS::itemName = "MAX_MIXTURE_ITEMS";
const int WELLDIMS::MAX_MIXTURE_ITEMS::defaultValue = 3;
const std::string WELLDIMS::MAX_COMPLETION_X::itemName = "MAX_COMPLETION_X";
const int WELLDIMS::MAX_COMPLETION_X::defaultValue = 0;
const std::string WELLDIMS::MAX_WELLIST_PR_WELL::itemName = "MAX_WELLIST_PR_WELL";
const int WELLDIMS::MAX_WELLIST_PR_WELL::defaultValue = 1;
const std::string WELLDIMS::MAX_DYNAMIC_WELLIST::itemName = "MAX_DYNAMIC_WELLIST";
const int WELLDIMS::MAX_DYNAMIC_WELLIST::defaultValue = 1;
const std::string WELLDIMS::MAX_SECONDARY_WELLS::itemName = "MAX_SECONDARY_WELLS";
const int WELLDIMS::MAX_SECONDARY_WELLS::defaultValue = 1;


WELL_COMPLETION_PROBE::WELL_COMPLETION_PROBE( ) : ParserKeyword("WELL_COMPLETION_PROBE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("WGFRL");
  addDeckName("WGLRL");
  addDeckName("WGORL");
  addDeckName("WGPRL");
  addDeckName("WGPTL");
  addDeckName("WLFRL");
  addDeckName("WLPTL");
  addDeckName("WOFRL");
  addDeckName("WOGRL");
  addDeckName("WOITL");
  addDeckName("WOPRL");
  addDeckName("WOPTL");
  addDeckName("WVFRL");
  addDeckName("WVIRL");
  addDeckName("WVITL");
  addDeckName("WVPTL");
  addDeckName("WWCTL");
  addDeckName("WWFRL");
  addDeckName("WWGRL");
  addDeckName("WWIRL");
  addDeckName("WWITL");
  addDeckName("WWPRL");
  addDeckName("WWPTL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("COMPLETION", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELL_COMPLETION_PROBE::keywordName = "WELL_COMPLETION_PROBE";
const std::string WELL_COMPLETION_PROBE::WELL::itemName = "WELL";
const std::string WELL_COMPLETION_PROBE::COMPLETION::itemName = "COMPLETION";


WELL_PROBE::WELL_PROBE( ) : ParserKeyword("WELL_PROBE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NGOPAS");
  addDeckName("WALQ");
  addDeckName("WAPI");
  addDeckName("WBGLR");
  addDeckName("WBHP");
  addDeckName("WBHPFP");
  addDeckName("WBHPH");
  addDeckName("WBP");
  addDeckName("WBP4");
  addDeckName("WBP5");
  addDeckName("WBP9");
  addDeckName("WCIC");
  addDeckName("WCIR");
  addDeckName("WCIT");
  addDeckName("WCPC");
  addDeckName("WCPR");
  addDeckName("WCPT");
  addDeckName("WDRPR");
  addDeckName("WEDC");
  addDeckName("WEFF");
  addDeckName("WEFFG");
  addDeckName("WEPR");
  addDeckName("WEPT");
  addDeckName("WGCV");
  addDeckName("WGDC");
  addDeckName("WGIGR");
  addDeckName("WGIP");
  addDeckName("WGIP2");
  addDeckName("WGIR");
  addDeckName("WGIRH");
  addDeckName("WGIRT");
  addDeckName("WGIT");
  addDeckName("WGITH");
  addDeckName("WGLIR");
  addDeckName("WGLR");
  addDeckName("WGLRH");
  addDeckName("WGOR");
  addDeckName("WGORH");
  addDeckName("WGORL");
  addDeckName("WGPGR");
  addDeckName("WGPI");
  addDeckName("WGPI2");
  addDeckName("WGPP");
  addDeckName("WGPP2");
  addDeckName("WGPPF");
  addDeckName("WGPPF2");
  addDeckName("WGPPS");
  addDeckName("WGPPS2");
  addDeckName("WGPR");
  addDeckName("WGPRF");
  addDeckName("WGPRFP");
  addDeckName("WGPRH");
  addDeckName("WGPRS");
  addDeckName("WGPRT");
  addDeckName("WGPT");
  addDeckName("WGPTF");
  addDeckName("WGPTH");
  addDeckName("WGPTS");
  addDeckName("WGQ");
  addDeckName("WGVIR");
  addDeckName("WGVPR");
  addDeckName("WHD");
  addDeckName("WHDF");
  addDeckName("WJPR");
  addDeckName("WJPRH");
  addDeckName("WJPRT");
  addDeckName("WJPT");
  addDeckName("WJPTH");
  addDeckName("WLPR");
  addDeckName("WLPRH");
  addDeckName("WLPRT");
  addDeckName("WLPT");
  addDeckName("WLPTH");
  addDeckName("WMCON");
  addDeckName("WMCTL");
  addDeckName("WMIR");
  addDeckName("WMIT");
  addDeckName("WMMW");
  addDeckName("WMPR");
  addDeckName("WMPT");
  addDeckName("WMVFP");
  addDeckName("WNIR");
  addDeckName("WNIT");
  addDeckName("WNPR");
  addDeckName("WNPT");
  addDeckName("WOGLR");
  addDeckName("WOGR");
  addDeckName("WOGRH");
  addDeckName("WOIGR");
  addDeckName("WOIR");
  addDeckName("WOIRH");
  addDeckName("WOIRT");
  addDeckName("WOIT");
  addDeckName("WOITH");
  addDeckName("WOPGR");
  addDeckName("WOPI");
  addDeckName("WOPI2");
  addDeckName("WOPP");
  addDeckName("WOPP2");
  addDeckName("WOPR");
  addDeckName("WOPRF");
  addDeckName("WOPRH");
  addDeckName("WOPRS");
  addDeckName("WOPRT");
  addDeckName("WOPT");
  addDeckName("WOPTF");
  addDeckName("WOPTH");
  addDeckName("WOPTS");
  addDeckName("WPI");
  addDeckName("WPI1");
  addDeckName("WPI4");
  addDeckName("WPI5");
  addDeckName("WPI9");
  addDeckName("WPIG");
  addDeckName("WPIL");
  addDeckName("WPIO");
  addDeckName("WPIW");
  addDeckName("WPWE0");
  addDeckName("WPWE1");
  addDeckName("WPWE2");
  addDeckName("WPWE3");
  addDeckName("WPWE4");
  addDeckName("WPWE5");
  addDeckName("WPWE6");
  addDeckName("WPWE7");
  addDeckName("WPWEM");
  addDeckName("WSIC");
  addDeckName("WSIR");
  addDeckName("WSIT");
  addDeckName("WSPC");
  addDeckName("WSPR");
  addDeckName("WSPT");
  addDeckName("WSTAT");
  addDeckName("WTHP");
  addDeckName("WTHPFP");
  addDeckName("WTHPH");
  addDeckName("WTHT");
  addDeckName("WTIC");
  addDeckName("WTICF");
  addDeckName("WTICHEA");
  addDeckName("WTICS");
  addDeckName("WTIR");
  addDeckName("WTIRALK");
  addDeckName("WTIRANI");
  addDeckName("WTIRCAT");
  addDeckName("WTIRF");
  addDeckName("WTIRFOA");
  addDeckName("WTIRHEA");
  addDeckName("WTIRS");
  addDeckName("WTIRSUR");
  addDeckName("WTIT");
  addDeckName("WTITALK");
  addDeckName("WTITANI");
  addDeckName("WTITCAT");
  addDeckName("WTITF");
  addDeckName("WTITFOA");
  addDeckName("WTITHEA");
  addDeckName("WTITS");
  addDeckName("WTITSUR");
  addDeckName("WTPC");
  addDeckName("WTPCF");
  addDeckName("WTPCHEA");
  addDeckName("WTPCS");
  addDeckName("WTPR");
  addDeckName("WTPRALK");
  addDeckName("WTPRANI");
  addDeckName("WTPRCAT");
  addDeckName("WTPRF");
  addDeckName("WTPRFOA");
  addDeckName("WTPRHEA");
  addDeckName("WTPRS");
  addDeckName("WTPRSUR");
  addDeckName("WTPT");
  addDeckName("WTPTALK");
  addDeckName("WTPTANI");
  addDeckName("WTPTCAT");
  addDeckName("WTPTF");
  addDeckName("WTPTFOA");
  addDeckName("WTPTHEA");
  addDeckName("WTPTS");
  addDeckName("WTPTSUR");
  addDeckName("WVIR");
  addDeckName("WVIRT");
  addDeckName("WVIT");
  addDeckName("WVPGR");
  addDeckName("WVPR");
  addDeckName("WVPRT");
  addDeckName("WVPT");
  addDeckName("WWCT");
  addDeckName("WWCTH");
  addDeckName("WWGR");
  addDeckName("WWGRH");
  addDeckName("WWIGR");
  addDeckName("WWIP");
  addDeckName("WWIP2");
  addDeckName("WWIR");
  addDeckName("WWIRH");
  addDeckName("WWIRT");
  addDeckName("WWIT");
  addDeckName("WWITH");
  addDeckName("WWPGR");
  addDeckName("WWPI");
  addDeckName("WWPI2");
  addDeckName("WWPIR");
  addDeckName("WWPP");
  addDeckName("WWPP2");
  addDeckName("WWPR");
  addDeckName("WWPRH");
  addDeckName("WWPRT");
  addDeckName("WWPT");
  addDeckName("WWPTH");
  addDeckName("WWVIR");
  setMatchRegex("WU.+|(WBHWC|WGFWC|WOFWC|WWFWC)[1-9][0-9]?|WTPR.+|WTPT.+|WTPC.+|WTIR.+|WTIT.+|WTIC.+");
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
const std::string WELL_PROBE::keywordName = "WELL_PROBE";
const std::string WELL_PROBE::WELLS::itemName = "WELLS";


WELMOVEL::WELMOVEL( ) : ParserKeyword("WELMOVEL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELMOVEL");
  {
     ParserRecord record;
     {
        ParserItem item("WELLNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LGRNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLHEAD_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("WELLHEAD_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELMOVEL::keywordName = "WELMOVEL";
const std::string WELMOVEL::WELLNAME::itemName = "WELLNAME";
const std::string WELMOVEL::LGRNAME::itemName = "LGRNAME";
const std::string WELMOVEL::WELLHEAD_I::itemName = "WELLHEAD_I";
const std::string WELMOVEL::WELLHEAD_J::itemName = "WELLHEAD_J";


WELOPEN::WELOPEN( ) : ParserKeyword("WELOPEN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELOPEN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
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
const std::string WELOPEN::keywordName = "WELOPEN";
const std::string WELOPEN::WELL::itemName = "WELL";
const std::string WELOPEN::STATUS::itemName = "STATUS";
const std::string WELOPEN::STATUS::defaultValue = "OPEN";
const std::string WELOPEN::I::itemName = "I";
const std::string WELOPEN::J::itemName = "J";
const std::string WELOPEN::K::itemName = "K";
const std::string WELOPEN::C1::itemName = "C1";
const std::string WELOPEN::C2::itemName = "C2";


WELOPENL::WELOPENL( ) : ParserKeyword("WELOPENL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELOPENL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
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
const std::string WELOPENL::keywordName = "WELOPENL";
const std::string WELOPENL::WELL::itemName = "WELL";
const std::string WELOPENL::GRID::itemName = "GRID";
const std::string WELOPENL::STATUS::itemName = "STATUS";
const std::string WELOPENL::STATUS::defaultValue = "OPEN";
const std::string WELOPENL::I::itemName = "I";
const std::string WELOPENL::J::itemName = "J";
const std::string WELOPENL::K::itemName = "K";
const std::string WELOPENL::C1::itemName = "C1";
const std::string WELOPENL::C2::itemName = "C2";


WELPI::WELPI( ) : ParserKeyword("WELPI")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELPI");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STEADY_STATE_PRODUCTIVITY_OR_INJECTIVITY_INDEX_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELPI::keywordName = "WELPI";
const std::string WELPI::WELL_NAME::itemName = "WELL_NAME";
const std::string WELPI::STEADY_STATE_PRODUCTIVITY_OR_INJECTIVITY_INDEX_VALUE::itemName = "STEADY_STATE_PRODUCTIVITY_OR_INJECTIVITY_INDEX_VALUE";


WELPRI::WELPRI( ) : ParserKeyword("WELPRI")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELPRI");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRI1", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("SCALING1", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PRI2", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("SCALING2", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELPRI::keywordName = "WELPRI";
const std::string WELPRI::WELL_NAME::itemName = "WELL_NAME";
const std::string WELPRI::PRI1::itemName = "PRI1";
const int WELPRI::PRI1::defaultValue = -1;
const std::string WELPRI::SCALING1::itemName = "SCALING1";
const double WELPRI::SCALING1::defaultValue = 1.000000;
const std::string WELPRI::PRI2::itemName = "PRI2";
const int WELPRI::PRI2::defaultValue = -1;
const std::string WELPRI::SCALING2::itemName = "SCALING2";
const double WELPRI::SCALING2::defaultValue = 1.000000;


WELSEGS::WELSEGS( ) : ParserKeyword("WELSEGS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSEGS");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("WELLBORE_VOLUME", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-05) );
        item.push_backDimension("Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("INFO_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_COMPONENTS", ParserItem::itype::STRING);
        item.setDefault( std::string("HFA") );
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("HO") );
        record.addItem(item);
     }
     {
        ParserItem item("TOP_X", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("TOP_Y", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("BRANCH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JOIN_SEGMENT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT_LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_CHANGE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DIAMETER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("AREA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("VOLUME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH_X", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH_Y", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSEGS::keywordName = "WELSEGS";
const std::string WELSEGS::WELL::itemName = "WELL";
const std::string WELSEGS::DEPTH::itemName = "DEPTH";
const std::string WELSEGS::LENGTH::itemName = "LENGTH";
const double WELSEGS::LENGTH::defaultValue = 0.000000;
const std::string WELSEGS::WELLBORE_VOLUME::itemName = "WELLBORE_VOLUME";
const double WELSEGS::WELLBORE_VOLUME::defaultValue = 0.000010;
const std::string WELSEGS::INFO_TYPE::itemName = "INFO_TYPE";
const std::string WELSEGS::PRESSURE_COMPONENTS::itemName = "PRESSURE_COMPONENTS";
const std::string WELSEGS::PRESSURE_COMPONENTS::defaultValue = "HFA";
const std::string WELSEGS::FLOW_MODEL::itemName = "FLOW_MODEL";
const std::string WELSEGS::FLOW_MODEL::defaultValue = "HO";
const std::string WELSEGS::TOP_X::itemName = "TOP_X";
const double WELSEGS::TOP_X::defaultValue = 0.000000;
const std::string WELSEGS::TOP_Y::itemName = "TOP_Y";
const double WELSEGS::TOP_Y::defaultValue = 0.000000;
const std::string WELSEGS::SEGMENT1::itemName = "SEGMENT1";
const std::string WELSEGS::SEGMENT2::itemName = "SEGMENT2";
const std::string WELSEGS::BRANCH::itemName = "BRANCH";
const std::string WELSEGS::JOIN_SEGMENT::itemName = "JOIN_SEGMENT";
const std::string WELSEGS::SEGMENT_LENGTH::itemName = "SEGMENT_LENGTH";
const std::string WELSEGS::DEPTH_CHANGE::itemName = "DEPTH_CHANGE";
const std::string WELSEGS::DIAMETER::itemName = "DIAMETER";
const std::string WELSEGS::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WELSEGS::AREA::itemName = "AREA";
const std::string WELSEGS::VOLUME::itemName = "VOLUME";
const std::string WELSEGS::LENGTH_X::itemName = "LENGTH_X";
const double WELSEGS::LENGTH_X::defaultValue = 0.000000;
const std::string WELSEGS::LENGTH_Y::itemName = "LENGTH_Y";
const double WELSEGS::LENGTH_Y::defaultValue = 0.000000;


WELSOMIN::WELSOMIN( ) : ParserKeyword("WELSOMIN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSOMIN");
  {
     ParserRecord record;
     {
        ParserItem item("SOMIN", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSOMIN::keywordName = "WELSOMIN";
const std::string WELSOMIN::SOMIN::itemName = "SOMIN";


WELSPECL::WELSPECL( ) : ParserKeyword("WELSPECL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSPECL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("HEAD_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("HEAD_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REF_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("D_RADIUS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INFLOW_EQ", ParserItem::itype::STRING);
        item.setDefault( std::string("STD") );
        record.addItem(item);
     }
     {
        ParserItem item("AUTO_SHUTIN", ParserItem::itype::STRING);
        item.setDefault( std::string("SHUT") );
        record.addItem(item);
     }
     {
        ParserItem item("CROSSFLOW", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("P_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("DENSITY_CALC", ParserItem::itype::STRING);
        item.setDefault( std::string("SEG") );
        record.addItem(item);
     }
     {
        ParserItem item("FIP_REGION", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("FRONTSIM1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FRONTSIM2", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("well_model", ParserItem::itype::STRING);
        item.setDefault( std::string("STD") );
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSPECL::keywordName = "WELSPECL";
const std::string WELSPECL::WELL::itemName = "WELL";
const std::string WELSPECL::GROUP::itemName = "GROUP";
const std::string WELSPECL::LGR::itemName = "LGR";
const std::string WELSPECL::HEAD_I::itemName = "HEAD_I";
const std::string WELSPECL::HEAD_J::itemName = "HEAD_J";
const std::string WELSPECL::REF_DEPTH::itemName = "REF_DEPTH";
const std::string WELSPECL::PHASE::itemName = "PHASE";
const std::string WELSPECL::D_RADIUS::itemName = "D_RADIUS";
const double WELSPECL::D_RADIUS::defaultValue = 0.000000;
const std::string WELSPECL::INFLOW_EQ::itemName = "INFLOW_EQ";
const std::string WELSPECL::INFLOW_EQ::defaultValue = "STD";
const std::string WELSPECL::AUTO_SHUTIN::itemName = "AUTO_SHUTIN";
const std::string WELSPECL::AUTO_SHUTIN::defaultValue = "SHUT";
const std::string WELSPECL::CROSSFLOW::itemName = "CROSSFLOW";
const std::string WELSPECL::CROSSFLOW::defaultValue = "YES";
const std::string WELSPECL::P_TABLE::itemName = "P_TABLE";
const int WELSPECL::P_TABLE::defaultValue = 0;
const std::string WELSPECL::DENSITY_CALC::itemName = "DENSITY_CALC";
const std::string WELSPECL::DENSITY_CALC::defaultValue = "SEG";
const std::string WELSPECL::FIP_REGION::itemName = "FIP_REGION";
const int WELSPECL::FIP_REGION::defaultValue = 0;
const std::string WELSPECL::FRONTSIM1::itemName = "FRONTSIM1";
const std::string WELSPECL::FRONTSIM2::itemName = "FRONTSIM2";
const std::string WELSPECL::well_model::itemName = "well_model";
const std::string WELSPECL::well_model::defaultValue = "STD";
const std::string WELSPECL::POLYMER_TABLE::itemName = "POLYMER_TABLE";
const int WELSPECL::POLYMER_TABLE::defaultValue = 0;


WELSPECS::WELSPECS( ) : ParserKeyword("WELSPECS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSPECS");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("HEAD_I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("HEAD_J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REF_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("D_RADIUS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INFLOW_EQ", ParserItem::itype::STRING);
        item.setDefault( std::string("STD") );
        record.addItem(item);
     }
     {
        ParserItem item("AUTO_SHUTIN", ParserItem::itype::STRING);
        item.setDefault( std::string("SHUT") );
        record.addItem(item);
     }
     {
        ParserItem item("CROSSFLOW", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("P_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("DENSITY_CALC", ParserItem::itype::STRING);
        item.setDefault( std::string("SEG") );
        record.addItem(item);
     }
     {
        ParserItem item("FIP_REGION", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("FRONTSIM1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FRONTSIM2", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("well_model", ParserItem::itype::STRING);
        item.setDefault( std::string("STD") );
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSPECS::keywordName = "WELSPECS";
const std::string WELSPECS::WELL::itemName = "WELL";
const std::string WELSPECS::GROUP::itemName = "GROUP";
const std::string WELSPECS::HEAD_I::itemName = "HEAD_I";
const std::string WELSPECS::HEAD_J::itemName = "HEAD_J";
const std::string WELSPECS::REF_DEPTH::itemName = "REF_DEPTH";
const std::string WELSPECS::PHASE::itemName = "PHASE";
const std::string WELSPECS::D_RADIUS::itemName = "D_RADIUS";
const double WELSPECS::D_RADIUS::defaultValue = 0.000000;
const std::string WELSPECS::INFLOW_EQ::itemName = "INFLOW_EQ";
const std::string WELSPECS::INFLOW_EQ::defaultValue = "STD";
const std::string WELSPECS::AUTO_SHUTIN::itemName = "AUTO_SHUTIN";
const std::string WELSPECS::AUTO_SHUTIN::defaultValue = "SHUT";
const std::string WELSPECS::CROSSFLOW::itemName = "CROSSFLOW";
const std::string WELSPECS::CROSSFLOW::defaultValue = "YES";
const std::string WELSPECS::P_TABLE::itemName = "P_TABLE";
const int WELSPECS::P_TABLE::defaultValue = 0;
const std::string WELSPECS::DENSITY_CALC::itemName = "DENSITY_CALC";
const std::string WELSPECS::DENSITY_CALC::defaultValue = "SEG";
const std::string WELSPECS::FIP_REGION::itemName = "FIP_REGION";
const int WELSPECS::FIP_REGION::defaultValue = 0;
const std::string WELSPECS::FRONTSIM1::itemName = "FRONTSIM1";
const std::string WELSPECS::FRONTSIM2::itemName = "FRONTSIM2";
const std::string WELSPECS::well_model::itemName = "well_model";
const std::string WELSPECS::well_model::defaultValue = "STD";
const std::string WELSPECS::POLYMER_TABLE::itemName = "POLYMER_TABLE";
const int WELSPECS::POLYMER_TABLE::defaultValue = 0;


WELTARG::WELTARG( ) : ParserKeyword("WELTARG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELTARG");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELTARG::keywordName = "WELTARG";
const std::string WELTARG::WELL::itemName = "WELL";
const std::string WELTARG::CMODE::itemName = "CMODE";
const std::string WELTARG::NEW_VALUE::itemName = "NEW_VALUE";


WFOAM::WFOAM( ) : ParserKeyword("WFOAM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WFOAM");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FOAM_CONCENTRATION", ParserItem::itype::UDA);
        item.push_backDimension("FoamDensity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WFOAM::keywordName = "WFOAM";
const std::string WFOAM::WELL::itemName = "WELL";
const std::string WFOAM::FOAM_CONCENTRATION::itemName = "FOAM_CONCENTRATION";


WFRICSEG::WFRICSEG( ) : ParserKeyword("WFRICSEG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WFRICSEG");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TUBINGD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_SCALING", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WFRICSEG::keywordName = "WFRICSEG";
const std::string WFRICSEG::WELL::itemName = "WELL";
const std::string WFRICSEG::TUBINGD::itemName = "TUBINGD";
const std::string WFRICSEG::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WFRICSEG::FLOW_SCALING::itemName = "FLOW_SCALING";
const double WFRICSEG::FLOW_SCALING::defaultValue = 1.000000;


WFRICSGL::WFRICSGL( ) : ParserKeyword("WFRICSGL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WFRICSGL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TUBINGD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_SCALING", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WFRICSGL::keywordName = "WFRICSGL";
const std::string WFRICSGL::WELL::itemName = "WELL";
const std::string WFRICSGL::TUBINGD::itemName = "TUBINGD";
const std::string WFRICSGL::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WFRICSGL::FLOW_SCALING::itemName = "FLOW_SCALING";
const double WFRICSGL::FLOW_SCALING::defaultValue = 1.000000;


WFRICTN::WFRICTN( ) : ParserKeyword("WFRICTN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WFRICTN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TUBINGD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_SCALING", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WFRICTN::keywordName = "WFRICTN";
const std::string WFRICTN::WELL::itemName = "WELL";
const std::string WFRICTN::TUBINGD::itemName = "TUBINGD";
const std::string WFRICTN::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WFRICTN::FLOW_SCALING::itemName = "FLOW_SCALING";
const double WFRICTN::FLOW_SCALING::defaultValue = 1.000000;


WFRICTNL::WFRICTNL( ) : ParserKeyword("WFRICTNL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WFRICTNL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TUBINGD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_SCALING", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WFRICTNL::keywordName = "WFRICTNL";
const std::string WFRICTNL::WELL::itemName = "WELL";
const std::string WFRICTNL::TUBINGD::itemName = "TUBINGD";
const std::string WFRICTNL::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WFRICTNL::FLOW_SCALING::itemName = "FLOW_SCALING";
const double WFRICTNL::FLOW_SCALING::defaultValue = 1.000000;


WGASPROD::WGASPROD( ) : ParserKeyword("WGASPROD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WGASPROD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INCREMENTAL_GAS_PRODUCTION_RATE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_INCREMENTS", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WGASPROD::keywordName = "WGASPROD";
const std::string WGASPROD::WELL_NAME::itemName = "WELL_NAME";
const std::string WGASPROD::INCREMENTAL_GAS_PRODUCTION_RATE::itemName = "INCREMENTAL_GAS_PRODUCTION_RATE";
const std::string WGASPROD::MAX_INCREMENTS::itemName = "MAX_INCREMENTS";


WGORPEN::WGORPEN( ) : ParserKeyword("WGORPEN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WGORPEN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("BASE_GOR", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OIL_RATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("AVG_GOR0", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WGORPEN::keywordName = "WGORPEN";
const std::string WGORPEN::WELL_NAME::itemName = "WELL_NAME";
const std::string WGORPEN::BASE_GOR::itemName = "BASE_GOR";
const std::string WGORPEN::MAX_OIL_RATE::itemName = "MAX_OIL_RATE";
const std::string WGORPEN::AVG_GOR0::itemName = "AVG_GOR0";


WGRUPCON::WGRUPCON( ) : ParserKeyword("WGRUPCON")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WGRUPCON");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_CONTROLLED", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("GUIDE_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SCALING_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WGRUPCON::keywordName = "WGRUPCON";
const std::string WGRUPCON::WELL::itemName = "WELL";
const std::string WGRUPCON::GROUP_CONTROLLED::itemName = "GROUP_CONTROLLED";
const std::string WGRUPCON::GROUP_CONTROLLED::defaultValue = "YES";
const std::string WGRUPCON::GUIDE_RATE::itemName = "GUIDE_RATE";
const double WGRUPCON::GUIDE_RATE::defaultValue = -1.000000;
const std::string WGRUPCON::PHASE::itemName = "PHASE";
const std::string WGRUPCON::SCALING_FACTOR::itemName = "SCALING_FACTOR";
const double WGRUPCON::SCALING_FACTOR::defaultValue = 1.000000;


WH2NUM::WH2NUM( ) : ParserKeyword("WH2NUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("WH2NUM");
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
const std::string WH2NUM::keywordName = "WH2NUM";
const std::string WH2NUM::data::itemName = "data";


WH3NUM::WH3NUM( ) : ParserKeyword("WH3NUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("WH3NUM");
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
const std::string WH3NUM::keywordName = "WH3NUM";
const std::string WH3NUM::data::itemName = "data";


WHEDREFD::WHEDREFD( ) : ParserKeyword("WHEDREFD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WHEDREFD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
const std::string WHEDREFD::keywordName = "WHEDREFD";
const std::string WHEDREFD::WELL::itemName = "WELL";
const std::string WHEDREFD::DEPTH::itemName = "DEPTH";


WHISTCTL::WHISTCTL( ) : ParserKeyword("WHISTCTL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WHISTCTL");
  {
     ParserRecord record;
     {
        ParserItem item("CMODE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("BPH_TERMINATE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WHISTCTL::keywordName = "WHISTCTL";
const std::string WHISTCTL::CMODE::itemName = "CMODE";
const std::string WHISTCTL::CMODE::defaultValue = "NONE";
const std::string WHISTCTL::BPH_TERMINATE::itemName = "BPH_TERMINATE";
const std::string WHISTCTL::BPH_TERMINATE::defaultValue = "NO";


WHTEMP::WHTEMP( ) : ParserKeyword("WHTEMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WHTEMP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("THP_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WHTEMP::keywordName = "WHTEMP";
const std::string WHTEMP::WELL::itemName = "WELL";
const std::string WHTEMP::VFP_TABLE::itemName = "VFP_TABLE";
const std::string WHTEMP::THP_TEMP::itemName = "THP_TEMP";


WINJMULT::WINJMULT( ) : ParserKeyword("WINJMULT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WINJMULT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FRACTURING_PRESSURE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MULTIPLIER_GRADIENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("MODE", ParserItem::itype::STRING);
        item.setDefault( std::string("WREV") );
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WINJMULT::keywordName = "WINJMULT";
const std::string WINJMULT::WELL_NAME::itemName = "WELL_NAME";
const std::string WINJMULT::FRACTURING_PRESSURE::itemName = "FRACTURING_PRESSURE";
const std::string WINJMULT::MULTIPLIER_GRADIENT::itemName = "MULTIPLIER_GRADIENT";
const double WINJMULT::MULTIPLIER_GRADIENT::defaultValue = 0.000000;
const std::string WINJMULT::MODE::itemName = "MODE";
const std::string WINJMULT::MODE::defaultValue = "WREV";
const std::string WINJMULT::I::itemName = "I";
const int WINJMULT::I::defaultValue = -1;
const std::string WINJMULT::J::itemName = "J";
const int WINJMULT::J::defaultValue = -1;
const std::string WINJMULT::K::itemName = "K";
const int WINJMULT::K::defaultValue = -1;


WINJTEMP::WINJTEMP( ) : ParserKeyword("WINJTEMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WINJTEMP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("STEAM_QUALITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(15.560000) );
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("ENTHALPY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Energy/Mass");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WINJTEMP::keywordName = "WINJTEMP";
const std::string WINJTEMP::WELL::itemName = "WELL";
const std::string WINJTEMP::STEAM_QUALITY::itemName = "STEAM_QUALITY";
const double WINJTEMP::STEAM_QUALITY::defaultValue = 1.000000;
const std::string WINJTEMP::TEMPERATURE::itemName = "TEMPERATURE";
const double WINJTEMP::TEMPERATURE::defaultValue = 15.560000;
const std::string WINJTEMP::PRESSURE::itemName = "PRESSURE";
const double WINJTEMP::PRESSURE::defaultValue = 0.000000;
const std::string WINJTEMP::ENTHALPY::itemName = "ENTHALPY";
const double WINJTEMP::ENTHALPY::defaultValue = 0.000000;


WLIFT::WLIFT( ) : ParserKeyword("WLIFT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLIFT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGER_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("TRIGGRE_PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_VFP_TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_ALQ_VALUE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_WEFAC", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("WWCT_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("NEW_THP_LIMIT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("WGOR_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_SHIFT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("THP_SHIFT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WLIFT::keywordName = "WLIFT";
const std::string WLIFT::WELL::itemName = "WELL";
const std::string WLIFT::TRIGGER_LIMIT::itemName = "TRIGGER_LIMIT";
const std::string WLIFT::TRIGGRE_PHASE::itemName = "TRIGGRE_PHASE";
const std::string WLIFT::NEW_VFP_TABLE::itemName = "NEW_VFP_TABLE";
const std::string WLIFT::NEW_ALQ_VALUE::itemName = "NEW_ALQ_VALUE";
const std::string WLIFT::NEW_WEFAC::itemName = "NEW_WEFAC";
const std::string WLIFT::WWCT_LIMIT::itemName = "WWCT_LIMIT";
const std::string WLIFT::NEW_THP_LIMIT::itemName = "NEW_THP_LIMIT";
const std::string WLIFT::WGOR_LIMIT::itemName = "WGOR_LIMIT";
const std::string WLIFT::ALQ_SHIFT::itemName = "ALQ_SHIFT";
const std::string WLIFT::THP_SHIFT::itemName = "THP_SHIFT";


WLIFTOPT::WLIFTOPT( ) : ParserKeyword("WLIFTOPT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLIFTOPT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("USE_OPTIMIZER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_LIFT_GAS_RATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("WEIGHT_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_LIFT_GAS_RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("DELTA_GAS_RATE_WEIGHT_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("ALLOCATE_EXTRA_LIFT_GAS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WLIFTOPT::keywordName = "WLIFTOPT";
const std::string WLIFTOPT::WELL::itemName = "WELL";
const std::string WLIFTOPT::USE_OPTIMIZER::itemName = "USE_OPTIMIZER";
const std::string WLIFTOPT::MAX_LIFT_GAS_RATE::itemName = "MAX_LIFT_GAS_RATE";
const std::string WLIFTOPT::WEIGHT_FACTOR::itemName = "WEIGHT_FACTOR";
const double WLIFTOPT::WEIGHT_FACTOR::defaultValue = 1.000000;
const std::string WLIFTOPT::MIN_LIFT_GAS_RATE::itemName = "MIN_LIFT_GAS_RATE";
const double WLIFTOPT::MIN_LIFT_GAS_RATE::defaultValue = 0.000000;
const std::string WLIFTOPT::DELTA_GAS_RATE_WEIGHT_FACTOR::itemName = "DELTA_GAS_RATE_WEIGHT_FACTOR";
const double WLIFTOPT::DELTA_GAS_RATE_WEIGHT_FACTOR::defaultValue = 0.000000;
const std::string WLIFTOPT::ALLOCATE_EXTRA_LIFT_GAS::itemName = "ALLOCATE_EXTRA_LIFT_GAS";
const std::string WLIFTOPT::ALLOCATE_EXTRA_LIFT_GAS::defaultValue = "NO";


WLIMTOL::WLIMTOL( ) : ParserKeyword("WLIMTOL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLIMTOL");
  {
     ParserRecord record;
     {
        ParserItem item("TOLERANCE_FRACTION", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WLIMTOL::keywordName = "WLIMTOL";
const std::string WLIMTOL::TOLERANCE_FRACTION::itemName = "TOLERANCE_FRACTION";


WLIST::WLIST( ) : ParserKeyword("WLIST")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLIST");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ACTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WLIST::keywordName = "WLIST";
const std::string WLIST::NAME::itemName = "NAME";
const std::string WLIST::ACTION::itemName = "ACTION";
const std::string WLIST::WELLS::itemName = "WELLS";


WLISTARG::WLISTARG( ) : ParserKeyword("WLISTARG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLISTARG");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL", ParserItem::itype::STRING);
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
const std::string WLISTARG::keywordName = "WLISTARG";
const std::string WLISTARG::WELL::itemName = "WELL";
const std::string WLISTARG::CONTROL::itemName = "CONTROL";
const std::string WLISTARG::VALUES::itemName = "VALUES";


WLISTNAM::WLISTNAM( ) : ParserKeyword("WLISTNAM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WLISTNAM");
  {
     ParserRecord record;
     {
        ParserItem item("LIST_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WLISTNAM::keywordName = "WLISTNAM";
const std::string WLISTNAM::LIST_NAME::itemName = "LIST_NAME";
const std::string WLISTNAM::WELLS::itemName = "WELLS";


WNETCTRL::WNETCTRL( ) : ParserKeyword("WNETCTRL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WNETCTRL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL", ParserItem::itype::STRING);
        item.setDefault( std::string("THP") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WNETCTRL::keywordName = "WNETCTRL";
const std::string WNETCTRL::WELL::itemName = "WELL";
const std::string WNETCTRL::CONTROL::itemName = "CONTROL";
const std::string WNETCTRL::CONTROL::defaultValue = "THP";


WNETDP::WNETDP( ) : ParserKeyword("WNETDP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WNETDP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WNETDP::keywordName = "WNETDP";
const std::string WNETDP::WELL::itemName = "WELL";
const std::string WNETDP::DP::itemName = "DP";
const double WNETDP::DP::defaultValue = 0.000000;


WORKLIM::WORKLIM( ) : ParserKeyword("WORKLIM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WORKLIM");
  {
     ParserRecord record;
     {
        ParserItem item("LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WORKLIM::keywordName = "WORKLIM";
const std::string WORKLIM::LIMIT::itemName = "LIMIT";


WORKTHP::WORKTHP( ) : ParserKeyword("WORKTHP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WORKTHP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WORK_OVER_PROCEDURE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WORKTHP::keywordName = "WORKTHP";
const std::string WORKTHP::WELL_NAME::itemName = "WELL_NAME";
const std::string WORKTHP::WORK_OVER_PROCEDURE::itemName = "WORK_OVER_PROCEDURE";
const std::string WORKTHP::WORK_OVER_PROCEDURE::defaultValue = "NONE";


WPAVE::WPAVE( ) : ParserKeyword("WPAVE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPAVE");
  {
     ParserRecord record;
     {
        ParserItem item("WEIGTH_FACTOR1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("WEIGTH_FACTOR2", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_CORRECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("WELL") );
        record.addItem(item);
     }
     {
        ParserItem item("CONNECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPAVE::keywordName = "WPAVE";
const std::string WPAVE::WEIGTH_FACTOR1::itemName = "WEIGTH_FACTOR1";
const double WPAVE::WEIGTH_FACTOR1::defaultValue = 0.500000;
const std::string WPAVE::WEIGTH_FACTOR2::itemName = "WEIGTH_FACTOR2";
const double WPAVE::WEIGTH_FACTOR2::defaultValue = 1.000000;
const std::string WPAVE::DEPTH_CORRECTION::itemName = "DEPTH_CORRECTION";
const std::string WPAVE::DEPTH_CORRECTION::defaultValue = "WELL";
const std::string WPAVE::CONNECTION::itemName = "CONNECTION";
const std::string WPAVE::CONNECTION::defaultValue = "OPEN";


WPAVEDEP::WPAVEDEP( ) : ParserKeyword("WPAVEDEP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPAVEDEP");
  {
     ParserRecord record;
     {
        ParserItem item("WELLNAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REFDEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPAVEDEP::keywordName = "WPAVEDEP";
const std::string WPAVEDEP::WELLNAME::itemName = "WELLNAME";
const std::string WPAVEDEP::REFDEPTH::itemName = "REFDEPTH";
const double WPAVEDEP::REFDEPTH::defaultValue = 0.000000;


WPIMULT::WPIMULT( ) : ParserKeyword("WPIMULT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPIMULT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLPI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
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
        ParserItem item("FIRST", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LAST", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPIMULT::keywordName = "WPIMULT";
const std::string WPIMULT::WELL::itemName = "WELL";
const std::string WPIMULT::WELLPI::itemName = "WELLPI";
const double WPIMULT::WELLPI::defaultValue = 1.000000;
const std::string WPIMULT::I::itemName = "I";
const std::string WPIMULT::J::itemName = "J";
const std::string WPIMULT::K::itemName = "K";
const std::string WPIMULT::FIRST::itemName = "FIRST";
const std::string WPIMULT::LAST::itemName = "LAST";


WPIMULTL::WPIMULTL( ) : ParserKeyword("WPIMULTL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPIMULTL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WELLPI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
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
        ParserItem item("FIRST", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LAST", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPIMULTL::keywordName = "WPIMULTL";
const std::string WPIMULTL::WELL::itemName = "WELL";
const std::string WPIMULTL::WELLPI::itemName = "WELLPI";
const double WPIMULTL::WELLPI::defaultValue = 1.000000;
const std::string WPIMULTL::GRID::itemName = "GRID";
const std::string WPIMULTL::GRID::defaultValue = "";
const std::string WPIMULTL::I::itemName = "I";
const std::string WPIMULTL::J::itemName = "J";
const std::string WPIMULTL::K::itemName = "K";
const std::string WPIMULTL::FIRST::itemName = "FIRST";
const std::string WPIMULTL::LAST::itemName = "LAST";


WPITAB::WPITAB( ) : ParserKeyword("WPITAB")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPITAB");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PI", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPITAB::keywordName = "WPITAB";
const std::string WPITAB::WELL::itemName = "WELL";
const std::string WPITAB::PI::itemName = "PI";
const double WPITAB::PI::defaultValue = 0.000000;


WPLUG::WPLUG( ) : ParserKeyword("WPLUG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPLUG");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH_TOP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH_BOTTOM", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SOURCE", ParserItem::itype::STRING);
        item.setDefault( std::string("WELL") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPLUG::keywordName = "WPLUG";
const std::string WPLUG::WELL::itemName = "WELL";
const std::string WPLUG::LENGTH_TOP::itemName = "LENGTH_TOP";
const std::string WPLUG::LENGTH_BOTTOM::itemName = "LENGTH_BOTTOM";
const std::string WPLUG::SOURCE::itemName = "SOURCE";
const std::string WPLUG::SOURCE::defaultValue = "WELL";


WPMITAB::WPMITAB( ) : ParserKeyword("WPMITAB")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPMITAB");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPMITAB::keywordName = "WPMITAB";
const std::string WPMITAB::WELL::itemName = "WELL";
const std::string WPMITAB::TABLE_NUMBER::itemName = "TABLE_NUMBER";


WPOLYMER::WPOLYMER( ) : ParserKeyword("WPOLYMER")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("WPOLYMER");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER_CONCENTRATION", ParserItem::itype::UDA);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     {
        ParserItem item("SALT_CONCENTRATION", ParserItem::itype::UDA);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_POLYMER_CONCENTRATION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_SALT_CONCENTRATION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPOLYMER::keywordName = "WPOLYMER";
const std::string WPOLYMER::WELL::itemName = "WELL";
const std::string WPOLYMER::POLYMER_CONCENTRATION::itemName = "POLYMER_CONCENTRATION";
const std::string WPOLYMER::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";
const std::string WPOLYMER::GROUP_POLYMER_CONCENTRATION::itemName = "GROUP_POLYMER_CONCENTRATION";
const std::string WPOLYMER::GROUP_SALT_CONCENTRATION::itemName = "GROUP_SALT_CONCENTRATION";


WPOLYRED::WPOLYRED( ) : ParserKeyword("WPOLYRED")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPOLYRED");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER_VISCOSITY_RED", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPOLYRED::keywordName = "WPOLYRED";
const std::string WPOLYRED::WELL::itemName = "WELL";
const std::string WPOLYRED::POLYMER_VISCOSITY_RED::itemName = "POLYMER_VISCOSITY_RED";
const double WPOLYRED::POLYMER_VISCOSITY_RED::defaultValue = 1.000000;


WPOTCALC::WPOTCALC( ) : ParserKeyword("WPOTCALC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WPOTCALC");
  {
     ParserRecord record;
     {
        ParserItem item("FORCE_CALC", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("USE_SEGMENT_VOLUMES", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPOTCALC::keywordName = "WPOTCALC";
const std::string WPOTCALC::FORCE_CALC::itemName = "FORCE_CALC";
const std::string WPOTCALC::FORCE_CALC::defaultValue = "YES";
const std::string WPOTCALC::USE_SEGMENT_VOLUMES::itemName = "USE_SEGMENT_VOLUMES";
const std::string WPOTCALC::USE_SEGMENT_VOLUMES::defaultValue = "NO";


WREGROUP::WREGROUP( ) : ParserKeyword("WREGROUP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WREGROUP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        item.setDefault( std::string(" ") );
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_UPPER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("UPPER_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("GROUP_LOWER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LOWER_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WREGROUP::keywordName = "WREGROUP";
const std::string WREGROUP::WELL::itemName = "WELL";
const std::string WREGROUP::QUANTITY::itemName = "QUANTITY";
const std::string WREGROUP::QUANTITY::defaultValue = " ";
const std::string WREGROUP::GROUP_UPPER::itemName = "GROUP_UPPER";
const std::string WREGROUP::UPPER_LIMIT::itemName = "UPPER_LIMIT";
const double WREGROUP::UPPER_LIMIT::defaultValue = 100000000000000000000.000000;
const std::string WREGROUP::GROUP_LOWER::itemName = "GROUP_LOWER";
const std::string WREGROUP::LOWER_LIMIT::itemName = "LOWER_LIMIT";
const double WREGROUP::LOWER_LIMIT::defaultValue = 0.000000;


WRFT::WRFT( ) : ParserKeyword("WRFT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WRFT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WRFT::keywordName = "WRFT";
const std::string WRFT::WELL::itemName = "WELL";


WRFTPLT::WRFTPLT( ) : ParserKeyword("WRFTPLT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WRFTPLT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OUTPUT_RFT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("OUTPUT_PLT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("OUTPUT_SEGMENT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WRFTPLT::keywordName = "WRFTPLT";
const std::string WRFTPLT::WELL::itemName = "WELL";
const std::string WRFTPLT::OUTPUT_RFT::itemName = "OUTPUT_RFT";
const std::string WRFTPLT::OUTPUT_RFT::defaultValue = "NO";
const std::string WRFTPLT::OUTPUT_PLT::itemName = "OUTPUT_PLT";
const std::string WRFTPLT::OUTPUT_PLT::defaultValue = "NO";
const std::string WRFTPLT::OUTPUT_SEGMENT::itemName = "OUTPUT_SEGMENT";
const std::string WRFTPLT::OUTPUT_SEGMENT::defaultValue = "NO";


WSALT::WSALT( ) : ParserKeyword("WSALT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSALT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONCENTRATION", ParserItem::itype::UDA);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSALT::keywordName = "WSALT";
const std::string WSALT::WELL::itemName = "WELL";
const std::string WSALT::CONCENTRATION::itemName = "CONCENTRATION";


WSCCLEAN::WSCCLEAN( ) : ParserKeyword("WSCCLEAN")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSCCLEAN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_MULTIPLIER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("C1", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("C2", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSCCLEAN::keywordName = "WSCCLEAN";
const std::string WSCCLEAN::WELL::itemName = "WELL";
const std::string WSCCLEAN::SCALE_MULTIPLIER::itemName = "SCALE_MULTIPLIER";
const double WSCCLEAN::SCALE_MULTIPLIER::defaultValue = 1.000000;
const std::string WSCCLEAN::I::itemName = "I";
const int WSCCLEAN::I::defaultValue = -1;
const std::string WSCCLEAN::J::itemName = "J";
const int WSCCLEAN::J::defaultValue = -1;
const std::string WSCCLEAN::K::itemName = "K";
const int WSCCLEAN::K::defaultValue = -1;
const std::string WSCCLEAN::C1::itemName = "C1";
const int WSCCLEAN::C1::defaultValue = -1;
const std::string WSCCLEAN::C2::itemName = "C2";
const int WSCCLEAN::C2::defaultValue = -1;


WSCCLENL::WSCCLENL( ) : ParserKeyword("WSCCLENL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSCCLENL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_MULTIPLIER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("I", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("C1", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("C2", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSCCLENL::keywordName = "WSCCLENL";
const std::string WSCCLENL::WELL::itemName = "WELL";
const std::string WSCCLENL::SCALE_MULTIPLIER::itemName = "SCALE_MULTIPLIER";
const double WSCCLENL::SCALE_MULTIPLIER::defaultValue = 1.000000;
const std::string WSCCLENL::LGR::itemName = "LGR";
const std::string WSCCLENL::I::itemName = "I";
const int WSCCLENL::I::defaultValue = -1;
const std::string WSCCLENL::J::itemName = "J";
const int WSCCLENL::J::defaultValue = -1;
const std::string WSCCLENL::K::itemName = "K";
const int WSCCLENL::K::defaultValue = -1;
const std::string WSCCLENL::C1::itemName = "C1";
const int WSCCLENL::C1::defaultValue = -1;
const std::string WSCCLENL::C2::itemName = "C2";
const int WSCCLENL::C2::defaultValue = -1;


WSCTAB::WSCTAB( ) : ParserKeyword("WSCTAB")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSCTAB");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_DEPOSITION_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_DAMAGE_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UNUSED", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SCALE_DISSOLUTION_TABLE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSCTAB::keywordName = "WSCTAB";
const std::string WSCTAB::WELL::itemName = "WELL";
const std::string WSCTAB::SCALE_DEPOSITION_TABLE::itemName = "SCALE_DEPOSITION_TABLE";
const int WSCTAB::SCALE_DEPOSITION_TABLE::defaultValue = 0;
const std::string WSCTAB::SCALE_DAMAGE_TABLE::itemName = "SCALE_DAMAGE_TABLE";
const int WSCTAB::SCALE_DAMAGE_TABLE::defaultValue = 0;
const std::string WSCTAB::UNUSED::itemName = "UNUSED";
const std::string WSCTAB::SCALE_DISSOLUTION_TABLE::itemName = "SCALE_DISSOLUTION_TABLE";
const int WSCTAB::SCALE_DISSOLUTION_TABLE::defaultValue = 0;


WSEGAICD::WSEGAICD( ) : ParserKeyword("WSEGAICD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGAICD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("AICD_STRENGTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("ICD_LENGTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(12.000000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("RHO", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000.250000) );
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("VISCOSITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.450000) );
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRANSITION_WIDTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.050000) );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SOMETHING", ParserItem::itype::DOUBLE);
        item.setDefault( double(5.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("SCALING_METHOD", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_QICD", ParserItem::itype::DOUBLE);
        item.push_backDimension("ReservoirVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_RATE_EXPONENT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("VISC_EXPONENT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("ICD_FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_FLOW_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_FLOW_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_FLOW_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_VSIC_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_VISC_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_VISC_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGAICD::keywordName = "WSEGAICD";
const std::string WSEGAICD::WELL::itemName = "WELL";
const std::string WSEGAICD::SEGMENT1::itemName = "SEGMENT1";
const int WSEGAICD::SEGMENT1::defaultValue = 0;
const std::string WSEGAICD::SEGMENT2::itemName = "SEGMENT2";
const int WSEGAICD::SEGMENT2::defaultValue = 0;
const std::string WSEGAICD::AICD_STRENGTH::itemName = "AICD_STRENGTH";
const double WSEGAICD::AICD_STRENGTH::defaultValue = 0.000000;
const std::string WSEGAICD::ICD_LENGTH::itemName = "ICD_LENGTH";
const double WSEGAICD::ICD_LENGTH::defaultValue = 12.000000;
const std::string WSEGAICD::RHO::itemName = "RHO";
const double WSEGAICD::RHO::defaultValue = 1000.250000;
const std::string WSEGAICD::VISCOSITY::itemName = "VISCOSITY";
const double WSEGAICD::VISCOSITY::defaultValue = 1.450000;
const std::string WSEGAICD::WATER_LIMIT::itemName = "WATER_LIMIT";
const double WSEGAICD::WATER_LIMIT::defaultValue = 0.500000;
const std::string WSEGAICD::TRANSITION_WIDTH::itemName = "TRANSITION_WIDTH";
const double WSEGAICD::TRANSITION_WIDTH::defaultValue = 0.050000;
const std::string WSEGAICD::MAX_SOMETHING::itemName = "MAX_SOMETHING";
const double WSEGAICD::MAX_SOMETHING::defaultValue = 5.000000;
const std::string WSEGAICD::SCALING_METHOD::itemName = "SCALING_METHOD";
const int WSEGAICD::SCALING_METHOD::defaultValue = -1;
const std::string WSEGAICD::MAX_QICD::itemName = "MAX_QICD";
const std::string WSEGAICD::FLOW_RATE_EXPONENT::itemName = "FLOW_RATE_EXPONENT";
const std::string WSEGAICD::VISC_EXPONENT::itemName = "VISC_EXPONENT";
const std::string WSEGAICD::ICD_FLAG::itemName = "ICD_FLAG";
const std::string WSEGAICD::ICD_FLAG::defaultValue = "OPEN";
const std::string WSEGAICD::OIL_FLOW_FRACTION::itemName = "OIL_FLOW_FRACTION";
const double WSEGAICD::OIL_FLOW_FRACTION::defaultValue = 1.000000;
const std::string WSEGAICD::WATER_FLOW_FRACTION::itemName = "WATER_FLOW_FRACTION";
const double WSEGAICD::WATER_FLOW_FRACTION::defaultValue = 1.000000;
const std::string WSEGAICD::GAS_FLOW_FRACTION::itemName = "GAS_FLOW_FRACTION";
const double WSEGAICD::GAS_FLOW_FRACTION::defaultValue = 1.000000;
const std::string WSEGAICD::OIL_VSIC_FRACTION::itemName = "OIL_VSIC_FRACTION";
const double WSEGAICD::OIL_VSIC_FRACTION::defaultValue = 1.000000;
const std::string WSEGAICD::WATER_VISC_FRACTION::itemName = "WATER_VISC_FRACTION";
const double WSEGAICD::WATER_VISC_FRACTION::defaultValue = 1.000000;
const std::string WSEGAICD::GAS_VISC_FRACTION::itemName = "GAS_VISC_FRACTION";
const double WSEGAICD::GAS_VISC_FRACTION::defaultValue = 1.000000;


WSEGDFIN::WSEGDFIN( ) : ParserKeyword("WSEGDFIN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGDFIN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGDFIN::keywordName = "WSEGDFIN";
const std::string WSEGDFIN::DATA::itemName = "DATA";


WSEGDFMD::WSEGDFMD( ) : ParserKeyword("WSEGDFMD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGDFMD");
  {
     ParserRecord record;
     {
        ParserItem item("DRIFT_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("ORIGINAL") );
        record.addItem(item);
     }
     {
        ParserItem item("INCLINATION_FACTOR", ParserItem::itype::STRING);
        item.setDefault( std::string("H-K") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_EFFECT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGDFMD::keywordName = "WSEGDFMD";
const std::string WSEGDFMD::DRIFT_MODEL::itemName = "DRIFT_MODEL";
const std::string WSEGDFMD::DRIFT_MODEL::defaultValue = "ORIGINAL";
const std::string WSEGDFMD::INCLINATION_FACTOR::itemName = "INCLINATION_FACTOR";
const std::string WSEGDFMD::INCLINATION_FACTOR::defaultValue = "H-K";
const std::string WSEGDFMD::GAS_EFFECT::itemName = "GAS_EFFECT";
const std::string WSEGDFMD::GAS_EFFECT::defaultValue = "NO";


WSEGDFPA::WSEGDFPA( ) : ParserKeyword("WSEGDFPA")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGDFPA");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_LIQUID_VD_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
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
        ParserItem item("C0_A", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.200000) );
        record.addItem(item);
     }
     {
        ParserItem item("C0_B", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("FV", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("OIL_WATER_VD_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("N", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGDFPA::keywordName = "WSEGDFPA";
const std::string WSEGDFPA::GAS_LIQUID_VD_FACTOR::itemName = "GAS_LIQUID_VD_FACTOR";
const double WSEGDFPA::GAS_LIQUID_VD_FACTOR::defaultValue = 1.000000;
const std::string WSEGDFPA::A1::itemName = "A1";
const std::string WSEGDFPA::A2::itemName = "A2";
const std::string WSEGDFPA::C0_A::itemName = "C0_A";
const double WSEGDFPA::C0_A::defaultValue = 1.200000;
const std::string WSEGDFPA::C0_B::itemName = "C0_B";
const std::string WSEGDFPA::FV::itemName = "FV";
const double WSEGDFPA::FV::defaultValue = 1.000000;
const std::string WSEGDFPA::OIL_WATER_VD_FACTOR::itemName = "OIL_WATER_VD_FACTOR";
const double WSEGDFPA::OIL_WATER_VD_FACTOR::defaultValue = -1.000000;
const std::string WSEGDFPA::A::itemName = "A";
const std::string WSEGDFPA::B1::itemName = "B1";
const std::string WSEGDFPA::B2::itemName = "B2";
const std::string WSEGDFPA::N::itemName = "N";


WSEGDIMS::WSEGDIMS( ) : ParserKeyword("WSEGDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WSEGDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NSWLMX", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NSEGMX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NLBRMX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NCRDMX", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGDIMS::keywordName = "WSEGDIMS";
const std::string WSEGDIMS::NSWLMX::itemName = "NSWLMX";
const int WSEGDIMS::NSWLMX::defaultValue = 0;
const std::string WSEGDIMS::NSEGMX::itemName = "NSEGMX";
const int WSEGDIMS::NSEGMX::defaultValue = 1;
const std::string WSEGDIMS::NLBRMX::itemName = "NLBRMX";
const int WSEGDIMS::NLBRMX::defaultValue = 1;
const std::string WSEGDIMS::NCRDMX::itemName = "NCRDMX";
const int WSEGDIMS::NCRDMX::defaultValue = 0;


WSEGEXSS::WSEGEXSS( ) : ParserKeyword("WSEGEXSS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGEXSS");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_IMPORT_RATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("R", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("LiquidSurfaceVolume/Time*Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PEXT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGEXSS::keywordName = "WSEGEXSS";
const std::string WSEGEXSS::WELL::itemName = "WELL";
const std::string WSEGEXSS::SEGMENT::itemName = "SEGMENT";
const std::string WSEGEXSS::TYPE::itemName = "TYPE";
const std::string WSEGEXSS::TYPE::defaultValue = "NONE";
const std::string WSEGEXSS::GAS_IMPORT_RATE::itemName = "GAS_IMPORT_RATE";
const std::string WSEGEXSS::R::itemName = "R";
const double WSEGEXSS::R::defaultValue = 0.000000;
const std::string WSEGEXSS::PEXT::itemName = "PEXT";
const double WSEGEXSS::PEXT::defaultValue = 0.000000;


WSEGFLIM::WSEGFLIM( ) : ParserKeyword("WSEGFLIM")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGFLIM");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("LIMITED_PHASE1", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_LIMIT1", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("A1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("LIMITED_PHASE2", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_LIMIT2", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("A2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGFLIM::keywordName = "WSEGFLIM";
const std::string WSEGFLIM::WELL::itemName = "WELL";
const std::string WSEGFLIM::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGFLIM::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGFLIM::LIMITED_PHASE1::itemName = "LIMITED_PHASE1";
const std::string WSEGFLIM::LIMITED_PHASE1::defaultValue = "NONE";
const std::string WSEGFLIM::FLOW_LIMIT1::itemName = "FLOW_LIMIT1";
const std::string WSEGFLIM::A1::itemName = "A1";
const std::string WSEGFLIM::LIMITED_PHASE2::itemName = "LIMITED_PHASE2";
const std::string WSEGFLIM::LIMITED_PHASE2::defaultValue = "NONE";
const std::string WSEGFLIM::FLOW_LIMIT2::itemName = "FLOW_LIMIT2";
const std::string WSEGFLIM::A2::itemName = "A2";
const std::string WSEGFLIM::STATUS::itemName = "STATUS";
const std::string WSEGFLIM::STATUS::defaultValue = "OPEN";


WSEGFMOD::WSEGFMOD( ) : ParserKeyword("WSEGFMOD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGFMOD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FLOW_MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("HO") );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_LIQUID_VD_FACTOR", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("FV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("B2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGFMOD::keywordName = "WSEGFMOD";
const std::string WSEGFMOD::WELL::itemName = "WELL";
const std::string WSEGFMOD::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGFMOD::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGFMOD::FLOW_MODEL::itemName = "FLOW_MODEL";
const std::string WSEGFMOD::FLOW_MODEL::defaultValue = "HO";
const std::string WSEGFMOD::GAS_LIQUID_VD_FACTOR::itemName = "GAS_LIQUID_VD_FACTOR";
const std::string WSEGFMOD::A::itemName = "A";
const std::string WSEGFMOD::B::itemName = "B";
const std::string WSEGFMOD::FV::itemName = "FV";
const std::string WSEGFMOD::B1::itemName = "B1";
const std::string WSEGFMOD::B2::itemName = "B2";


WSEGINIT::WSEGINIT( ) : ParserKeyword("WSEGINIT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGINIT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("P0", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("W0", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("G0", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("RS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("RV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("API", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/ReservoirVolume");
        record.addItem(item);
     }
     {
        ParserItem item("BRINE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/ReservoirVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGINIT::keywordName = "WSEGINIT";
const std::string WSEGINIT::WELL::itemName = "WELL";
const std::string WSEGINIT::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGINIT::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGINIT::P0::itemName = "P0";
const std::string WSEGINIT::W0::itemName = "W0";
const std::string WSEGINIT::G0::itemName = "G0";
const std::string WSEGINIT::RS::itemName = "RS";
const std::string WSEGINIT::RV::itemName = "RV";
const std::string WSEGINIT::API::itemName = "API";
const std::string WSEGINIT::POLYMER::itemName = "POLYMER";
const std::string WSEGINIT::BRINE::itemName = "BRINE";


WSEGITER::WSEGITER( ) : ParserKeyword("WSEGITER")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGITER");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_WELL_ITERATIONS", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TIMES_REDUCED", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("REDUCTION_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.300000) );
        record.addItem(item);
     }
     {
        ParserItem item("INCREASING_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(2.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGITER::keywordName = "WSEGITER";
const std::string WSEGITER::MAX_WELL_ITERATIONS::itemName = "MAX_WELL_ITERATIONS";
const int WSEGITER::MAX_WELL_ITERATIONS::defaultValue = 20;
const std::string WSEGITER::MAX_TIMES_REDUCED::itemName = "MAX_TIMES_REDUCED";
const int WSEGITER::MAX_TIMES_REDUCED::defaultValue = 5;
const std::string WSEGITER::REDUCTION_FACTOR::itemName = "REDUCTION_FACTOR";
const double WSEGITER::REDUCTION_FACTOR::defaultValue = 0.300000;
const std::string WSEGITER::INCREASING_FACTOR::itemName = "INCREASING_FACTOR";
const double WSEGITER::INCREASING_FACTOR::defaultValue = 2.000000;


WSEGLABY::WSEGLABY( ) : ParserKeyword("WSEGLABY")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGLABY");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NCONFIG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CHANNELS", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        item.setDefault( double(6e-05) );
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("L1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.186300) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("L2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.283200) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("D", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.006316) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("R", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-05) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("GAMMA_INLET", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.350000) );
        record.addItem(item);
     }
     {
        ParserItem item("GAMMA_OUTLET", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("GAMMA_LAB", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGLABY::keywordName = "WSEGLABY";
const std::string WSEGLABY::WELL::itemName = "WELL";
const std::string WSEGLABY::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGLABY::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGLABY::NCONFIG::itemName = "NCONFIG";
const std::string WSEGLABY::CHANNELS::itemName = "CHANNELS";
const int WSEGLABY::CHANNELS::defaultValue = 2;
const std::string WSEGLABY::A::itemName = "A";
const double WSEGLABY::A::defaultValue = 0.000060;
const std::string WSEGLABY::L1::itemName = "L1";
const double WSEGLABY::L1::defaultValue = 0.186300;
const std::string WSEGLABY::L2::itemName = "L2";
const double WSEGLABY::L2::defaultValue = 0.283200;
const std::string WSEGLABY::D::itemName = "D";
const double WSEGLABY::D::defaultValue = 0.006316;
const std::string WSEGLABY::R::itemName = "R";
const double WSEGLABY::R::defaultValue = 0.000010;
const std::string WSEGLABY::GAMMA_INLET::itemName = "GAMMA_INLET";
const double WSEGLABY::GAMMA_INLET::defaultValue = 0.350000;
const std::string WSEGLABY::GAMMA_OUTLET::itemName = "GAMMA_OUTLET";
const double WSEGLABY::GAMMA_OUTLET::defaultValue = 0.500000;
const std::string WSEGLABY::GAMMA_LAB::itemName = "GAMMA_LAB";
const double WSEGLABY::GAMMA_LAB::defaultValue = 0.500000;
const std::string WSEGLABY::STATUS::itemName = "STATUS";
const std::string WSEGLABY::STATUS::defaultValue = "OPEN";


WSEGLINK::WSEGLINK( ) : ParserKeyword("WSEGLINK")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGLINK");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGLINK::keywordName = "WSEGLINK";
const std::string WSEGLINK::WELL::itemName = "WELL";
const std::string WSEGLINK::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGLINK::SEGMENT2::itemName = "SEGMENT2";


WSEGMULT::WSEGMULT( ) : ParserKeyword("WSEGMULT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGMULT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
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
        ParserItem item("GOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGMULT::keywordName = "WSEGMULT";
const std::string WSEGMULT::WELL::itemName = "WELL";
const std::string WSEGMULT::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGMULT::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGMULT::A::itemName = "A";
const double WSEGMULT::A::defaultValue = 1.000000;
const std::string WSEGMULT::B::itemName = "B";
const double WSEGMULT::B::defaultValue = 0.000000;
const std::string WSEGMULT::C::itemName = "C";
const double WSEGMULT::C::defaultValue = 0.000000;
const std::string WSEGMULT::D::itemName = "D";
const double WSEGMULT::D::defaultValue = 0.000000;
const std::string WSEGMULT::E::itemName = "E";
const double WSEGMULT::E::defaultValue = 0.000000;
const std::string WSEGMULT::GOR::itemName = "GOR";
const double WSEGMULT::GOR::defaultValue = 0.000000;


WSEGPROP::WSEGPROP( ) : ParserKeyword("WSEGPROP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGPROP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("R", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("A", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("V", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("PIPEA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("CV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGPROP::keywordName = "WSEGPROP";
const std::string WSEGPROP::WELL::itemName = "WELL";
const std::string WSEGPROP::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGPROP::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGPROP::D::itemName = "D";
const std::string WSEGPROP::R::itemName = "R";
const std::string WSEGPROP::A::itemName = "A";
const std::string WSEGPROP::V::itemName = "V";
const std::string WSEGPROP::PIPEA::itemName = "PIPEA";
const std::string WSEGPROP::CV::itemName = "CV";
const std::string WSEGPROP::K::itemName = "K";


WSEGSEP::WSEGSEP( ) : ParserKeyword("WSEGSEP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGSEP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("EMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("HOLDUP_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGSEP::keywordName = "WSEGSEP";
const std::string WSEGSEP::WELL::itemName = "WELL";
const std::string WSEGSEP::SEGMENT1::itemName = "SEGMENT1";
const std::string WSEGSEP::SEGMENT2::itemName = "SEGMENT2";
const std::string WSEGSEP::PHASE::itemName = "PHASE";
const std::string WSEGSEP::PHASE::defaultValue = "NONE";
const std::string WSEGSEP::EMAX::itemName = "EMAX";
const double WSEGSEP::EMAX::defaultValue = 1.000000;
const std::string WSEGSEP::HOLDUP_FRACTION::itemName = "HOLDUP_FRACTION";
const double WSEGSEP::HOLDUP_FRACTION::defaultValue = 0.010000;


WSEGSICD::WSEGSICD( ) : ParserKeyword("WSEGSICD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGSICD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEG1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SEG2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("STRENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure*Time*Time/GeometricVolume*GeometricVolume");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(12.000000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DENSITY_CALI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000.250000) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("VISCOSITY_CALI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.450000) );
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     {
        ParserItem item("CRITICAL_VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WIDTH_TRANS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.050000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_VISC_RATIO", ParserItem::itype::DOUBLE);
        item.setDefault( double(5.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("METHOD_SCALING_FACTOR", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ABS_RATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("GeometricVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGSICD::keywordName = "WSEGSICD";
const std::string WSEGSICD::WELL::itemName = "WELL";
const std::string WSEGSICD::SEG1::itemName = "SEG1";
const std::string WSEGSICD::SEG2::itemName = "SEG2";
const std::string WSEGSICD::STRENGTH::itemName = "STRENGTH";
const std::string WSEGSICD::LENGTH::itemName = "LENGTH";
const double WSEGSICD::LENGTH::defaultValue = 12.000000;
const std::string WSEGSICD::DENSITY_CALI::itemName = "DENSITY_CALI";
const double WSEGSICD::DENSITY_CALI::defaultValue = 1000.250000;
const std::string WSEGSICD::VISCOSITY_CALI::itemName = "VISCOSITY_CALI";
const double WSEGSICD::VISCOSITY_CALI::defaultValue = 1.450000;
const std::string WSEGSICD::CRITICAL_VALUE::itemName = "CRITICAL_VALUE";
const double WSEGSICD::CRITICAL_VALUE::defaultValue = 0.500000;
const std::string WSEGSICD::WIDTH_TRANS::itemName = "WIDTH_TRANS";
const double WSEGSICD::WIDTH_TRANS::defaultValue = 0.050000;
const std::string WSEGSICD::MAX_VISC_RATIO::itemName = "MAX_VISC_RATIO";
const double WSEGSICD::MAX_VISC_RATIO::defaultValue = 5.000000;
const std::string WSEGSICD::METHOD_SCALING_FACTOR::itemName = "METHOD_SCALING_FACTOR";
const int WSEGSICD::METHOD_SCALING_FACTOR::defaultValue = -1;
const std::string WSEGSICD::MAX_ABS_RATE::itemName = "MAX_ABS_RATE";
const std::string WSEGSICD::STATUS::itemName = "STATUS";
const std::string WSEGSICD::STATUS::defaultValue = "OPEN";


WSEGSOLV::WSEGSOLV( ) : ParserKeyword("WSEGSOLV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGSOLV");
  {
     ParserRecord record;
     {
        ParserItem item("USE_ITERATIVE_SOLVER", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_LINEAR_ITER", ParserItem::itype::INT);
        item.setDefault( 30 );
        record.addItem(item);
     }
     {
        ParserItem item("CONVERGENCE_TARGET", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-10) );
        record.addItem(item);
     }
     {
        ParserItem item("PC_FILL", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("DROP_TOL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("P_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-08) );
        record.addItem(item);
     }
     {
        ParserItem item("LOOP_THRESHOLD", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Length*Length*Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("LOOP_MULTIPLIER", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGSOLV::keywordName = "WSEGSOLV";
const std::string WSEGSOLV::USE_ITERATIVE_SOLVER::itemName = "USE_ITERATIVE_SOLVER";
const std::string WSEGSOLV::USE_ITERATIVE_SOLVER::defaultValue = "NO";
const std::string WSEGSOLV::MAX_LINEAR_ITER::itemName = "MAX_LINEAR_ITER";
const int WSEGSOLV::MAX_LINEAR_ITER::defaultValue = 30;
const std::string WSEGSOLV::CONVERGENCE_TARGET::itemName = "CONVERGENCE_TARGET";
const double WSEGSOLV::CONVERGENCE_TARGET::defaultValue = 0.000000;
const std::string WSEGSOLV::PC_FILL::itemName = "PC_FILL";
const int WSEGSOLV::PC_FILL::defaultValue = 2;
const std::string WSEGSOLV::DROP_TOL::itemName = "DROP_TOL";
const double WSEGSOLV::DROP_TOL::defaultValue = 0.000000;
const std::string WSEGSOLV::P_LIMIT::itemName = "P_LIMIT";
const double WSEGSOLV::P_LIMIT::defaultValue = 0.000000;
const std::string WSEGSOLV::LOOP_THRESHOLD::itemName = "LOOP_THRESHOLD";
const double WSEGSOLV::LOOP_THRESHOLD::defaultValue = -1.000000;
const std::string WSEGSOLV::LOOP_MULTIPLIER::itemName = "LOOP_MULTIPLIER";
const double WSEGSOLV::LOOP_MULTIPLIER::defaultValue = -1.000000;


WSEGTABL::WSEGTABL( ) : ParserKeyword("WSEGTABL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGTABL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SURFACT", ParserItem::itype::UDA);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGTABL::keywordName = "WSEGTABL";
const std::string WSEGTABL::WELL::itemName = "WELL";
const std::string WSEGTABL::SURFACT::itemName = "SURFACT";


WSEGVALV::WSEGVALV( ) : ParserKeyword("WSEGVALV")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSEGVALV");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SEGMENT_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("AREA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("EXTRA_LENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PIPE_D", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PIPE_A", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("STATUS", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_A", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGVALV::keywordName = "WSEGVALV";
const std::string WSEGVALV::WELL::itemName = "WELL";
const std::string WSEGVALV::SEGMENT_NUMBER::itemName = "SEGMENT_NUMBER";
const std::string WSEGVALV::CV::itemName = "CV";
const std::string WSEGVALV::AREA::itemName = "AREA";
const std::string WSEGVALV::EXTRA_LENGTH::itemName = "EXTRA_LENGTH";
const std::string WSEGVALV::PIPE_D::itemName = "PIPE_D";
const std::string WSEGVALV::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WSEGVALV::PIPE_A::itemName = "PIPE_A";
const std::string WSEGVALV::STATUS::itemName = "STATUS";
const std::string WSEGVALV::STATUS::defaultValue = "OPEN";
const std::string WSEGVALV::MAX_A::itemName = "MAX_A";


WSKPTAB::WSKPTAB( ) : ParserKeyword("WSKPTAB")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSKPTAB");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUMBER_WATER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUMBER_POLYMER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSKPTAB::keywordName = "WSKPTAB";
const std::string WSKPTAB::WELL::itemName = "WELL";
const std::string WSKPTAB::TABLE_NUMBER_WATER::itemName = "TABLE_NUMBER_WATER";
const std::string WSKPTAB::TABLE_NUMBER_POLYMER::itemName = "TABLE_NUMBER_POLYMER";


WSOLVENT::WSOLVENT( ) : ParserKeyword("WSOLVENT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSOLVENT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SOLVENT_FRACTION", ParserItem::itype::UDA);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSOLVENT::keywordName = "WSOLVENT";
const std::string WSOLVENT::WELL::itemName = "WELL";
const std::string WSOLVENT::SOLVENT_FRACTION::itemName = "SOLVENT_FRACTION";


WTADD::WTADD( ) : ParserKeyword("WTADD")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTADD");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SHIFT", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTADD::keywordName = "WTADD";
const std::string WTADD::WELL::itemName = "WELL";
const std::string WTADD::CONTROL::itemName = "CONTROL";
const std::string WTADD::SHIFT::itemName = "SHIFT";
const std::string WTADD::NUM::itemName = "NUM";
const int WTADD::NUM::defaultValue = 1;


WTEMP::WTEMP( ) : ParserKeyword("WTEMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTEMP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
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
const std::string WTEMP::keywordName = "WTEMP";
const std::string WTEMP::WELL::itemName = "WELL";
const std::string WTEMP::TEMP::itemName = "TEMP";


WTEMPQ::WTEMPQ( ) : ParserKeyword("WTEMPQ")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTEMPQ");
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
     addRecord( record );
  }
}
const std::string WTEMPQ::keywordName = "WTEMPQ";
const std::string WTEMPQ::WELL::itemName = "WELL";
const std::string WTEMPQ::LGR::itemName = "LGR";


WTEST::WTEST( ) : ParserKeyword("WTEST")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTEST");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INTERVAL", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("REASON", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("TEST_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("START_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTEST::keywordName = "WTEST";
const std::string WTEST::WELL::itemName = "WELL";
const std::string WTEST::INTERVAL::itemName = "INTERVAL";
const std::string WTEST::REASON::itemName = "REASON";
const std::string WTEST::REASON::defaultValue = "";
const std::string WTEST::TEST_NUM::itemName = "TEST_NUM";
const int WTEST::TEST_NUM::defaultValue = 0;
const std::string WTEST::START_TIME::itemName = "START_TIME";
const double WTEST::START_TIME::defaultValue = 0.000000;


WTHPMAX::WTHPMAX( ) : ParserKeyword("WTHPMAX")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTHPMAX");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_THP_DESIGN_LIMIT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL", ParserItem::itype::STRING);
        item.setDefault( std::string("ORAT") );
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_LIMIT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("THP_OPEN_LIMIT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTHPMAX::keywordName = "WTHPMAX";
const std::string WTHPMAX::WELL::itemName = "WELL";
const std::string WTHPMAX::MAX_THP_DESIGN_LIMIT::itemName = "MAX_THP_DESIGN_LIMIT";
const std::string WTHPMAX::CONTROL::itemName = "CONTROL";
const std::string WTHPMAX::CONTROL::defaultValue = "ORAT";
const std::string WTHPMAX::CONTROL_LIMIT::itemName = "CONTROL_LIMIT";
const std::string WTHPMAX::THP_OPEN_LIMIT::itemName = "THP_OPEN_LIMIT";


WTMULT::WTMULT( ) : ParserKeyword("WTMULT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTMULT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FACTOR", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("NUM", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTMULT::keywordName = "WTMULT";
const std::string WTMULT::WELL::itemName = "WELL";
const std::string WTMULT::CONTROL::itemName = "CONTROL";
const std::string WTMULT::FACTOR::itemName = "FACTOR";
const std::string WTMULT::NUM::itemName = "NUM";
const int WTMULT::NUM::defaultValue = 1;


WTRACER::WTRACER( ) : ParserKeyword("WTRACER")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTRACER");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("CONCENTRATION", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("CUM_TRACER_FACTOR", ParserItem::itype::UDA);
        record.addItem(item);
     }
     {
        ParserItem item("PRODUCTION_GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTRACER::keywordName = "WTRACER";
const std::string WTRACER::WELL::itemName = "WELL";
const std::string WTRACER::TRACER::itemName = "TRACER";
const std::string WTRACER::CONCENTRATION::itemName = "CONCENTRATION";
const std::string WTRACER::CUM_TRACER_FACTOR::itemName = "CUM_TRACER_FACTOR";
const std::string WTRACER::PRODUCTION_GROUP::itemName = "PRODUCTION_GROUP";


WVFPDP::WVFPDP( ) : ParserKeyword("WVFPDP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WVFPDP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DELTA_P", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("LOSS_SCALING_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WVFPDP::keywordName = "WVFPDP";
const std::string WVFPDP::WELL::itemName = "WELL";
const std::string WVFPDP::DELTA_P::itemName = "DELTA_P";
const std::string WVFPDP::LOSS_SCALING_FACTOR::itemName = "LOSS_SCALING_FACTOR";
const double WVFPDP::LOSS_SCALING_FACTOR::defaultValue = 1.000000;


WVFPEXP::WVFPEXP( ) : ParserKeyword("WVFPEXP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WVFPEXP");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("EXPLICIT_IMPLICIT", ParserItem::itype::STRING);
        item.setDefault( std::string("IMP") );
        record.addItem(item);
     }
     {
        ParserItem item("CLOSE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("PREVENT_THP", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("EXTRAPOLATION_CONTROL", ParserItem::itype::STRING);
        item.setDefault( std::string("WG") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WVFPEXP::keywordName = "WVFPEXP";
const std::string WVFPEXP::WELL::itemName = "WELL";
const std::string WVFPEXP::EXPLICIT_IMPLICIT::itemName = "EXPLICIT_IMPLICIT";
const std::string WVFPEXP::EXPLICIT_IMPLICIT::defaultValue = "IMP";
const std::string WVFPEXP::CLOSE::itemName = "CLOSE";
const std::string WVFPEXP::CLOSE::defaultValue = "NO";
const std::string WVFPEXP::PREVENT_THP::itemName = "PREVENT_THP";
const std::string WVFPEXP::PREVENT_THP::defaultValue = "NO";
const std::string WVFPEXP::EXTRAPOLATION_CONTROL::itemName = "EXTRAPOLATION_CONTROL";
const std::string WVFPEXP::EXTRAPOLATION_CONTROL::defaultValue = "WG";


WWPAVE::WWPAVE( ) : ParserKeyword("WWPAVE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WWPAVE");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("F1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("F2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_CORRECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("WELL") );
        record.addItem(item);
     }
     {
        ParserItem item("WELL_CONNECTION", ParserItem::itype::STRING);
        item.setDefault( std::string("OPEN") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string WWPAVE::keywordName = "WWPAVE";
const std::string WWPAVE::WELL::itemName = "WELL";
const std::string WWPAVE::F1::itemName = "F1";
const double WWPAVE::F1::defaultValue = 0.500000;
const std::string WWPAVE::F2::itemName = "F2";
const double WWPAVE::F2::defaultValue = 0.500000;
const std::string WWPAVE::DEPTH_CORRECTION::itemName = "DEPTH_CORRECTION";
const std::string WWPAVE::DEPTH_CORRECTION::defaultValue = "WELL";
const std::string WWPAVE::WELL_CONNECTION::itemName = "WELL_CONNECTION";
const std::string WWPAVE::WELL_CONNECTION::defaultValue = "OPEN";


}
}
