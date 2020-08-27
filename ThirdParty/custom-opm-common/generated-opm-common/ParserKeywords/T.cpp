#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/T.hpp>
namespace Opm {
namespace ParserKeywords {
TABDIMS::TABDIMS( ) : ParserKeyword("TABDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TABDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTSFUN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NTPVT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NSSFUN", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("NPPVT", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("NTFIP", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NRPVT", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_RV_NODES", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NTENDP", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NUM_STATE_EQ", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NUM_EOS_RES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NUM_EOS_SURFACE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FLUX_REGIONS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_THERMAL_REGIONS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NTROCC", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PRESSURE_MAINTAINANCE_REGIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_KVALUE_TABLES", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NTALPHA", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPKDAM_MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPREWG_MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPVISO_MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM20_NOT_USED", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPPW2D_MAX_COLUMNS", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPPW2D_MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("ASPHALTENE_ASPWETF_MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 5 );
        record.addItem(item);
     }
     {
        ParserItem item("NUM_KVALUE_TABLES", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("RESERVED", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TABDIMS::keywordName = "TABDIMS";
const std::string TABDIMS::NTSFUN::itemName = "NTSFUN";
const int TABDIMS::NTSFUN::defaultValue = 1;
const std::string TABDIMS::NTPVT::itemName = "NTPVT";
const int TABDIMS::NTPVT::defaultValue = 1;
const std::string TABDIMS::NSSFUN::itemName = "NSSFUN";
const int TABDIMS::NSSFUN::defaultValue = 20;
const std::string TABDIMS::NPPVT::itemName = "NPPVT";
const int TABDIMS::NPPVT::defaultValue = 20;
const std::string TABDIMS::NTFIP::itemName = "NTFIP";
const int TABDIMS::NTFIP::defaultValue = 1;
const std::string TABDIMS::NRPVT::itemName = "NRPVT";
const int TABDIMS::NRPVT::defaultValue = 20;
const std::string TABDIMS::MAX_RV_NODES::itemName = "MAX_RV_NODES";
const std::string TABDIMS::NTENDP::itemName = "NTENDP";
const int TABDIMS::NTENDP::defaultValue = 1;
const std::string TABDIMS::NUM_STATE_EQ::itemName = "NUM_STATE_EQ";
const int TABDIMS::NUM_STATE_EQ::defaultValue = 1;
const std::string TABDIMS::NUM_EOS_RES::itemName = "NUM_EOS_RES";
const int TABDIMS::NUM_EOS_RES::defaultValue = 1;
const std::string TABDIMS::NUM_EOS_SURFACE::itemName = "NUM_EOS_SURFACE";
const std::string TABDIMS::MAX_FLUX_REGIONS::itemName = "MAX_FLUX_REGIONS";
const int TABDIMS::MAX_FLUX_REGIONS::defaultValue = 10;
const std::string TABDIMS::MAX_THERMAL_REGIONS::itemName = "MAX_THERMAL_REGIONS";
const int TABDIMS::MAX_THERMAL_REGIONS::defaultValue = 1;
const std::string TABDIMS::NTROCC::itemName = "NTROCC";
const std::string TABDIMS::MAX_PRESSURE_MAINTAINANCE_REGIONS::itemName = "MAX_PRESSURE_MAINTAINANCE_REGIONS";
const int TABDIMS::MAX_PRESSURE_MAINTAINANCE_REGIONS::defaultValue = 0;
const std::string TABDIMS::MAX_KVALUE_TABLES::itemName = "MAX_KVALUE_TABLES";
const int TABDIMS::MAX_KVALUE_TABLES::defaultValue = 0;
const std::string TABDIMS::NTALPHA::itemName = "NTALPHA";
const std::string TABDIMS::ASPHALTENE_ASPKDAM_MAX_ROWS::itemName = "ASPHALTENE_ASPKDAM_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPKDAM_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ASPHALTENE_ASPREWG_MAX_ROWS::itemName = "ASPHALTENE_ASPREWG_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPREWG_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ASPHALTENE_ASPVISO_MAX_ROWS::itemName = "ASPHALTENE_ASPVISO_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPVISO_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ITEM20_NOT_USED::itemName = "ITEM20_NOT_USED";
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_COLUMNS::itemName = "ASPHALTENE_ASPPW2D_MAX_COLUMNS";
const int TABDIMS::ASPHALTENE_ASPPW2D_MAX_COLUMNS::defaultValue = 5;
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_ROWS::itemName = "ASPHALTENE_ASPPW2D_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPPW2D_MAX_ROWS::defaultValue = 5;
const std::string TABDIMS::ASPHALTENE_ASPWETF_MAX_ROWS::itemName = "ASPHALTENE_ASPWETF_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPWETF_MAX_ROWS::defaultValue = 5;
const std::string TABDIMS::NUM_KVALUE_TABLES::itemName = "NUM_KVALUE_TABLES";
const int TABDIMS::NUM_KVALUE_TABLES::defaultValue = 0;
const std::string TABDIMS::RESERVED::itemName = "RESERVED";


TBLK::TBLK( ) : ParserKeyword("TBLK")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  setMatchRegex("TBLK(F|S).{1,3}");
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
const std::string TBLK::keywordName = "TBLK";
const std::string TBLK::data::itemName = "data";


TEMP::TEMP( ) : ParserKeyword("TEMP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TEMP");
}
const std::string TEMP::keywordName = "TEMP";


TEMPI::TEMPI( ) : ParserKeyword("TEMPI")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("TEMPI");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TEMPI::keywordName = "TEMPI";
const std::string TEMPI::data::itemName = "data";


TEMPNODE::TEMPNODE( ) : ParserKeyword("TEMPNODE")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NPPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TEMPNODE");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_DATA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TEMPNODE::keywordName = "TEMPNODE";
const std::string TEMPNODE::TABLE_DATA::itemName = "TABLE_DATA";


TEMPTVD::TEMPTVD( ) : ParserKeyword("TEMPTVD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TEMPTVD");
}
const std::string TEMPTVD::keywordName = "TEMPTVD";


TEMPVD::TEMPVD( ) : ParserKeyword("TEMPVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("TEMPVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TEMPVD::keywordName = "TEMPVD";
const std::string TEMPVD::DATA::itemName = "DATA";


THCGAS::THCGAS( ) : ParserKeyword("THCGAS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCGAS");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/AbsoluteTemperature*Length*Time");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCGAS::keywordName = "THCGAS";
const std::string THCGAS::data::itemName = "data";


THCOIL::THCOIL( ) : ParserKeyword("THCOIL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCOIL");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/AbsoluteTemperature*Length*Time");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCOIL::keywordName = "THCOIL";
const std::string THCOIL::data::itemName = "data";


THCONR::THCONR( ) : ParserKeyword("THCONR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCONR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/AbsoluteTemperature*Length*Time");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCONR::keywordName = "THCONR";
const std::string THCONR::data::itemName = "data";


THCONSF::THCONSF( ) : ParserKeyword("THCONSF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCONSF");
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
const std::string THCONSF::keywordName = "THCONSF";
const std::string THCONSF::data::itemName = "data";


THCROCK::THCROCK( ) : ParserKeyword("THCROCK")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCROCK");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/AbsoluteTemperature*Length*Time");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCROCK::keywordName = "THCROCK";
const std::string THCROCK::data::itemName = "data";


THCWATER::THCWATER( ) : ParserKeyword("THCWATER")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCWATER");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/AbsoluteTemperature*Length*Time");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCWATER::keywordName = "THCWATER";
const std::string THCWATER::data::itemName = "data";


THERMAL::THERMAL( ) : ParserKeyword("THERMAL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("THERMAL");
}
const std::string THERMAL::keywordName = "THERMAL";


THPRES::THPRES( ) : ParserKeyword("THPRES")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("THPRES");
  {
     ParserRecord record;
     {
        ParserItem item("REGION1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REGION2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string THPRES::keywordName = "THPRES";
const std::string THPRES::REGION1::itemName = "REGION1";
const std::string THPRES::REGION2::itemName = "REGION2";
const std::string THPRES::VALUE::itemName = "VALUE";


THPRESFT::THPRESFT( ) : ParserKeyword("THPRESFT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THPRESFT");
  {
     ParserRecord record;
     {
        ParserItem item("FAULT_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string THPRESFT::keywordName = "THPRESFT";
const std::string THPRESFT::FAULT_NAME::itemName = "FAULT_NAME";
const std::string THPRESFT::VALUE::itemName = "VALUE";


TIGHTEN::TIGHTEN( ) : ParserKeyword("TIGHTEN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TIGHTEN");
  {
     ParserRecord record;
     {
        ParserItem item("LINEAR_FACTOR", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_LINEAR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NONLINEAR_FACTOR", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_NONLINEAR", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TIGHTEN::keywordName = "TIGHTEN";
const std::string TIGHTEN::LINEAR_FACTOR::itemName = "LINEAR_FACTOR";
const std::string TIGHTEN::MAX_LINEAR::itemName = "MAX_LINEAR";
const std::string TIGHTEN::NONLINEAR_FACTOR::itemName = "NONLINEAR_FACTOR";
const std::string TIGHTEN::MAX_NONLINEAR::itemName = "MAX_NONLINEAR";


TIME::TIME( ) : ParserKeyword("TIME")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TIME");
  {
     ParserRecord record;
     {
        ParserItem item("step_list", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TIME::keywordName = "TIME";
const std::string TIME::step_list::itemName = "step_list";


TITLE::TITLE( ) : ParserKeyword("TITLE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TITLE");
  {
     ParserRecord record;
     {
        ParserItem item("TitleText", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TITLE::keywordName = "TITLE";
const std::string TITLE::TitleText::itemName = "TitleText";


TLMIXPAR::TLMIXPAR( ) : ParserKeyword("TLMIXPAR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TLMIXPAR");
  {
     ParserRecord record;
     {
        ParserItem item("TL_VISCOSITY_PARAMETER", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("TL_DENSITY_PARAMETER", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TLMIXPAR::keywordName = "TLMIXPAR";
const std::string TLMIXPAR::TL_VISCOSITY_PARAMETER::itemName = "TL_VISCOSITY_PARAMETER";
const std::string TLMIXPAR::TL_DENSITY_PARAMETER::itemName = "TL_DENSITY_PARAMETER";


TLPMIXPA::TLPMIXPA( ) : ParserKeyword("TLPMIXPA")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TLPMIXPA");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TLPMIXPA::keywordName = "TLPMIXPA";
const std::string TLPMIXPA::DATA::itemName = "DATA";


TNUM::TNUM( ) : ParserKeyword("TNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  setMatchRegex("TNUM(F|S).{1,3}");
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
const std::string TNUM::keywordName = "TNUM";
const std::string TNUM::data::itemName = "data";


TOLCRIT::TOLCRIT( ) : ParserKeyword("TOLCRIT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TOLCRIT");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TOLCRIT::keywordName = "TOLCRIT";
const std::string TOLCRIT::VALUE::itemName = "VALUE";
const double TOLCRIT::VALUE::defaultValue = 0.000001;


TOPS::TOPS( ) : ParserKeyword("TOPS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("TOPS");
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
const std::string TOPS::keywordName = "TOPS";
const std::string TOPS::data::itemName = "data";


TPAMEPS::TPAMEPS( ) : ParserKeyword("TPAMEPS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NTCREG",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TPAMEPS");
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
const std::string TPAMEPS::keywordName = "TPAMEPS";
const std::string TPAMEPS::DATA::itemName = "DATA";


TRACER::TRACER( ) : ParserKeyword("TRACER")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACER");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FLUID", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("UNIT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SOLUTION_PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_PART_TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ADSORB_PHASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACER::keywordName = "TRACER";
const std::string TRACER::NAME::itemName = "NAME";
const std::string TRACER::FLUID::itemName = "FLUID";
const std::string TRACER::UNIT::itemName = "UNIT";
const std::string TRACER::SOLUTION_PHASE::itemName = "SOLUTION_PHASE";
const std::string TRACER::NUM_PART_TABLE::itemName = "NUM_PART_TABLE";
const std::string TRACER::ADSORB_PHASE::itemName = "ADSORB_PHASE";


TRACERKM::TRACERKM( ) : ParserKeyword("TRACERKM")
{
  setSizeType(UNKNOWN);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACERKM");
  {
     ParserRecord record;
     {
        ParserItem item("TRACER_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PARTITION_FUNCTION_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("STANDARD") );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("PHASES", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TABLE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACERKM::keywordName = "TRACERKM";
const std::string TRACERKM::TRACER_NAME::itemName = "TRACER_NAME";
const std::string TRACERKM::PARTITION_FUNCTION_TYPE::itemName = "PARTITION_FUNCTION_TYPE";
const std::string TRACERKM::PARTITION_FUNCTION_TYPE::defaultValue = "STANDARD";
const std::string TRACERKM::PHASES::itemName = "PHASES";
const std::string TRACERKM::TABLE::itemName = "TABLE";


TRACERKP::TRACERKP( ) : ParserKeyword("TRACERKP")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("PARTTRAC","NKPTMX",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACERKP");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACERKP::keywordName = "TRACERKP";
const std::string TRACERKP::TABLE::itemName = "TABLE";


TRACERS::TRACERS( ) : ParserKeyword("TRACERS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TRACERS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_OIL_TRACERS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WATER_TRACERS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GAS_TRACERS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ENV_TRACERS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NUMERIC_DIFF", ParserItem::itype::STRING);
        item.setDefault( std::string("NODIFF") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITER", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("MIN_ITER", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("PASSIVE_NONLINEAR", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("ONEOFF_LIN_TIGHT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ONEOFF_NLIN_TIGHT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TIGHTENING_FACTORS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("NTIGHTFACTORS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACERS::keywordName = "TRACERS";
const std::string TRACERS::MAX_OIL_TRACERS::itemName = "MAX_OIL_TRACERS";
const int TRACERS::MAX_OIL_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_WATER_TRACERS::itemName = "MAX_WATER_TRACERS";
const int TRACERS::MAX_WATER_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_GAS_TRACERS::itemName = "MAX_GAS_TRACERS";
const int TRACERS::MAX_GAS_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_ENV_TRACERS::itemName = "MAX_ENV_TRACERS";
const int TRACERS::MAX_ENV_TRACERS::defaultValue = 0;
const std::string TRACERS::NUMERIC_DIFF::itemName = "NUMERIC_DIFF";
const std::string TRACERS::NUMERIC_DIFF::defaultValue = "NODIFF";
const std::string TRACERS::MAX_ITER::itemName = "MAX_ITER";
const int TRACERS::MAX_ITER::defaultValue = 12;
const std::string TRACERS::MIN_ITER::itemName = "MIN_ITER";
const int TRACERS::MIN_ITER::defaultValue = 1;
const std::string TRACERS::PASSIVE_NONLINEAR::itemName = "PASSIVE_NONLINEAR";
const std::string TRACERS::PASSIVE_NONLINEAR::defaultValue = "NO";
const std::string TRACERS::ONEOFF_LIN_TIGHT::itemName = "ONEOFF_LIN_TIGHT";
const std::string TRACERS::ONEOFF_NLIN_TIGHT::itemName = "ONEOFF_NLIN_TIGHT";
const std::string TRACERS::TIGHTENING_FACTORS::itemName = "TIGHTENING_FACTORS";
const double TRACERS::TIGHTENING_FACTORS::defaultValue = 1.000000;
const std::string TRACERS::NTIGHTFACTORS::itemName = "NTIGHTFACTORS";
const int TRACERS::NTIGHTFACTORS::defaultValue = 0;


TRACITVD::TRACITVD( ) : ParserKeyword("TRACITVD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACITVD");
  {
     ParserRecord record;
     {
        ParserItem item("FLUX_LIMITER", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("BOTH_TIMESTEP", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACITVD::keywordName = "TRACITVD";
const std::string TRACITVD::FLUX_LIMITER::itemName = "FLUX_LIMITER";
const int TRACITVD::FLUX_LIMITER::defaultValue = 1;
const std::string TRACITVD::BOTH_TIMESTEP::itemName = "BOTH_TIMESTEP";
const std::string TRACITVD::BOTH_TIMESTEP::defaultValue = "YES";


TRACTVD::TRACTVD( ) : ParserKeyword("TRACTVD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACTVD");
}
const std::string TRACTVD::keywordName = "TRACTVD";


TRADS::TRADS( ) : ParserKeyword("TRADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRADS");
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
const std::string TRADS::keywordName = "TRADS";
const std::string TRADS::DATA::itemName = "DATA";


TRANGL::TRANGL( ) : ParserKeyword("TRANGL")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("TRANGL");
  {
     ParserRecord record;
     {
        ParserItem item("IL", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JL", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KL", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRAN", ParserItem::itype::DOUBLE);
        item.push_backDimension("Transmissibility");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRANGL::keywordName = "TRANGL";
const std::string TRANGL::IL::itemName = "IL";
const std::string TRANGL::JL::itemName = "JL";
const std::string TRANGL::KL::itemName = "KL";
const std::string TRANGL::IG::itemName = "IG";
const std::string TRANGL::JG::itemName = "JG";
const std::string TRANGL::KG::itemName = "KG";
const std::string TRANGL::TRAN::itemName = "TRAN";


TRANR::TRANR( ) : ParserKeyword("TRANR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Transmissibility");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANR::keywordName = "TRANR";
const std::string TRANR::data::itemName = "data";


TRANTHT::TRANTHT( ) : ParserKeyword("TRANTHT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANTHT");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Transmissibility");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANTHT::keywordName = "TRANTHT";
const std::string TRANTHT::data::itemName = "data";


TRANX::TRANX( ) : ParserKeyword("TRANX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Transmissibility");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANX::keywordName = "TRANX";
const std::string TRANX::data::itemName = "data";


TRANY::TRANY( ) : ParserKeyword("TRANY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Transmissibility");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANY::keywordName = "TRANY";
const std::string TRANY::data::itemName = "data";


TRANZ::TRANZ( ) : ParserKeyword("TRANZ")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Transmissibility");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANZ::keywordName = "TRANZ";
const std::string TRANZ::data::itemName = "data";


TRDCY::TRDCY( ) : ParserKeyword("TRDCY")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  setMatchRegex("TRDCY.+");
  {
     ParserRecord record;
     {
        ParserItem item("HALF_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRDCY::keywordName = "TRDCY";
const std::string TRDCY::HALF_TIME::itemName = "HALF_TIME";
const double TRDCY::HALF_TIME::defaultValue = 100000000000000000000.000000;


TRDIF::TRDIF( ) : ParserKeyword("TRDIF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  setMatchRegex("TRDIF.+");
  {
     ParserRecord record;
     {
        ParserItem item("HALF_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRDIF::keywordName = "TRDIF";
const std::string TRDIF::HALF_TIME::itemName = "HALF_TIME";
const double TRDIF::HALF_TIME::defaultValue = 100000000000000000000.000000;


TRDIS::TRDIS( ) : ParserKeyword("TRDIS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  setMatchRegex("TRDIS.+");
  {
     ParserRecord record;
     {
        ParserItem item("D1TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D2TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D3TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D4TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D5TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D6TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D7TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D8TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("D9TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRDIS::keywordName = "TRDIS";
const std::string TRDIS::D1TABLE::itemName = "D1TABLE";
const std::string TRDIS::D2TABLE::itemName = "D2TABLE";
const std::string TRDIS::D3TABLE::itemName = "D3TABLE";
const std::string TRDIS::D4TABLE::itemName = "D4TABLE";
const std::string TRDIS::D5TABLE::itemName = "D5TABLE";
const std::string TRDIS::D6TABLE::itemName = "D6TABLE";
const std::string TRDIS::D7TABLE::itemName = "D7TABLE";
const std::string TRDIS::D8TABLE::itemName = "D8TABLE";
const std::string TRDIS::D9TABLE::itemName = "D9TABLE";


TREF::TREF( ) : ParserKeyword("TREF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TREF");
  {
     ParserRecord record;
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TREF::keywordName = "TREF";
const std::string TREF::TEMPERATURE::itemName = "TEMPERATURE";


TREFS::TREFS( ) : ParserKeyword("TREFS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TREFS");
  {
     ParserRecord record;
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TREFS::keywordName = "TREFS";
const std::string TREFS::TEMPERATURE::itemName = "TEMPERATURE";


TRKPF::TRKPF( ) : ParserKeyword("TRKPF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  setMatchRegex("TRKPF.+");
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
const std::string TRKPF::keywordName = "TRKPF";
const std::string TRKPF::data::itemName = "data";


TRNHD::TRNHD( ) : ParserKeyword("TRNHD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  setMatchRegex("TRNHD.+");
}
const std::string TRNHD::keywordName = "TRNHD";


TRPLPORO::TRPLPORO( ) : ParserKeyword("TRPLPORO")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TRPLPORO");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_MATRIX", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRPLPORO::keywordName = "TRPLPORO";
const std::string TRPLPORO::NUM_MATRIX::itemName = "NUM_MATRIX";


TRROCK::TRROCK( ) : ParserKeyword("TRROCK")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRROCK");
  {
     ParserRecord record;
     {
        ParserItem item("ADSORPTION_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MASS_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("INIT_MODEL", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRROCK::keywordName = "TRROCK";
const std::string TRROCK::ADSORPTION_INDEX::itemName = "ADSORPTION_INDEX";
const std::string TRROCK::MASS_DENSITY::itemName = "MASS_DENSITY";
const std::string TRROCK::INIT_MODEL::itemName = "INIT_MODEL";
const int TRROCK::INIT_MODEL::defaultValue = 1;


TSTEP::TSTEP( ) : ParserKeyword("TSTEP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TSTEP");
  {
     ParserRecord record;
     {
        ParserItem item("step_list", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Timestep");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TSTEP::keywordName = "TSTEP";
const std::string TSTEP::step_list::itemName = "step_list";


TUNING::TUNING( ) : ParserKeyword("TUNING")
{
  setFixedSize( (size_t) 3);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNING");
  {
     ParserRecord record;
     {
        ParserItem item("TSINIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMAXZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(365.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMINZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMCHP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.150000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSFMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(3.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFMIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.300000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TFDIFF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.250000) );
        record.addItem(item);
     }
     {
        ParserItem item("THRUPT", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TMAXWC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TRGTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-07) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.000100) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(10.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXWFL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGFIP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.025000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGSFT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("THIONX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRWGHT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("NEWTMX", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("NEWTMN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMAX", ParserItem::itype::INT);
        item.setDefault( 25 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWSIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWPIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("DDPLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DDSLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGDPR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("XXXDPR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNING::keywordName = "TUNING";
const std::string TUNING::TSINIT::itemName = "TSINIT";
const double TUNING::TSINIT::defaultValue = 1.000000;
const std::string TUNING::TSMAXZ::itemName = "TSMAXZ";
const double TUNING::TSMAXZ::defaultValue = 365.000000;
const std::string TUNING::TSMINZ::itemName = "TSMINZ";
const double TUNING::TSMINZ::defaultValue = 0.100000;
const std::string TUNING::TSMCHP::itemName = "TSMCHP";
const double TUNING::TSMCHP::defaultValue = 0.150000;
const std::string TUNING::TSFMAX::itemName = "TSFMAX";
const double TUNING::TSFMAX::defaultValue = 3.000000;
const std::string TUNING::TSFMIN::itemName = "TSFMIN";
const double TUNING::TSFMIN::defaultValue = 0.300000;
const std::string TUNING::TSFCNV::itemName = "TSFCNV";
const double TUNING::TSFCNV::defaultValue = 0.100000;
const std::string TUNING::TFDIFF::itemName = "TFDIFF";
const double TUNING::TFDIFF::defaultValue = 1.250000;
const std::string TUNING::THRUPT::itemName = "THRUPT";
const double TUNING::THRUPT::defaultValue = 100000000000000000000.000000;
const std::string TUNING::TMAXWC::itemName = "TMAXWC";
const std::string TUNING::TRGTTE::itemName = "TRGTTE";
const double TUNING::TRGTTE::defaultValue = 0.100000;
const std::string TUNING::TRGCNV::itemName = "TRGCNV";
const double TUNING::TRGCNV::defaultValue = 0.001000;
const std::string TUNING::TRGMBE::itemName = "TRGMBE";
const double TUNING::TRGMBE::defaultValue = 0.000000;
const std::string TUNING::TRGLCV::itemName = "TRGLCV";
const double TUNING::TRGLCV::defaultValue = 0.000100;
const std::string TUNING::XXXTTE::itemName = "XXXTTE";
const double TUNING::XXXTTE::defaultValue = 10.000000;
const std::string TUNING::XXXCNV::itemName = "XXXCNV";
const double TUNING::XXXCNV::defaultValue = 0.010000;
const std::string TUNING::XXXMBE::itemName = "XXXMBE";
const double TUNING::XXXMBE::defaultValue = 0.000001;
const std::string TUNING::XXXLCV::itemName = "XXXLCV";
const double TUNING::XXXLCV::defaultValue = 0.001000;
const std::string TUNING::XXXWFL::itemName = "XXXWFL";
const double TUNING::XXXWFL::defaultValue = 0.001000;
const std::string TUNING::TRGFIP::itemName = "TRGFIP";
const double TUNING::TRGFIP::defaultValue = 0.025000;
const std::string TUNING::TRGSFT::itemName = "TRGSFT";
const std::string TUNING::THIONX::itemName = "THIONX";
const double TUNING::THIONX::defaultValue = 0.010000;
const std::string TUNING::TRWGHT::itemName = "TRWGHT";
const int TUNING::TRWGHT::defaultValue = 1;
const std::string TUNING::NEWTMX::itemName = "NEWTMX";
const int TUNING::NEWTMX::defaultValue = 12;
const std::string TUNING::NEWTMN::itemName = "NEWTMN";
const int TUNING::NEWTMN::defaultValue = 1;
const std::string TUNING::LITMAX::itemName = "LITMAX";
const int TUNING::LITMAX::defaultValue = 25;
const std::string TUNING::LITMIN::itemName = "LITMIN";
const int TUNING::LITMIN::defaultValue = 1;
const std::string TUNING::MXWSIT::itemName = "MXWSIT";
const int TUNING::MXWSIT::defaultValue = 8;
const std::string TUNING::MXWPIT::itemName = "MXWPIT";
const int TUNING::MXWPIT::defaultValue = 8;
const std::string TUNING::DDPLIM::itemName = "DDPLIM";
const double TUNING::DDPLIM::defaultValue = 1000000.000000;
const std::string TUNING::DDSLIM::itemName = "DDSLIM";
const double TUNING::DDSLIM::defaultValue = 1000000.000000;
const std::string TUNING::TRGDPR::itemName = "TRGDPR";
const double TUNING::TRGDPR::defaultValue = 1000000.000000;
const std::string TUNING::XXXDPR::itemName = "XXXDPR";


TUNINGDP::TUNINGDP( ) : ParserKeyword("TUNINGDP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNINGDP");
  {
     ParserRecord record;
     {
        ParserItem item("TRGLCV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("XXXLCV", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("TRGDDP", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("TRGDDS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNINGDP::keywordName = "TUNINGDP";
const std::string TUNINGDP::TRGLCV::itemName = "TRGLCV";
const std::string TUNINGDP::XXXLCV::itemName = "XXXLCV";
const std::string TUNINGDP::TRGDDP::itemName = "TRGDDP";
const std::string TUNINGDP::TRGDDS::itemName = "TRGDDS";


TUNINGH::TUNINGH( ) : ParserKeyword("TUNINGH")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNINGH");
  {
     ParserRecord record;
     {
        ParserItem item("GRGLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.000100) );
        record.addItem(item);
     }
     {
        ParserItem item("GXXLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("GMSLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-20) );
        record.addItem(item);
     }
     {
        ParserItem item("LGTMIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("LGTMAX", ParserItem::itype::INT);
        item.setDefault( 25 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNINGH::keywordName = "TUNINGH";
const std::string TUNINGH::GRGLCV::itemName = "GRGLCV";
const double TUNINGH::GRGLCV::defaultValue = 0.000100;
const std::string TUNINGH::GXXLCV::itemName = "GXXLCV";
const double TUNINGH::GXXLCV::defaultValue = 0.001000;
const std::string TUNINGH::GMSLCV::itemName = "GMSLCV";
const double TUNINGH::GMSLCV::defaultValue = 0.000000;
const std::string TUNINGH::LGTMIN::itemName = "LGTMIN";
const int TUNINGH::LGTMIN::defaultValue = 1;
const std::string TUNINGH::LGTMAX::itemName = "LGTMAX";
const int TUNINGH::LGTMAX::defaultValue = 25;


TUNINGL::TUNINGL( ) : ParserKeyword("TUNINGL")
{
  setFixedSize( (size_t) 3);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNINGL");
  {
     ParserRecord record;
     {
        ParserItem item("TSINIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMAXZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(365.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMINZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMCHP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.150000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSFMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(3.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFMIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.300000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TFDIFF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.250000) );
        record.addItem(item);
     }
     {
        ParserItem item("THRUPT", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TMAXWC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TRGTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-07) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.000100) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(10.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXWFL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGFIP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.025000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGSFT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("THIONX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRWGHT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("NEWTMX", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("NEWTMN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMAX", ParserItem::itype::INT);
        item.setDefault( 25 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWSIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWPIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("DDPLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DDSLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGDPR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("XXXDPR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNINGL::keywordName = "TUNINGL";
const std::string TUNINGL::TSINIT::itemName = "TSINIT";
const double TUNINGL::TSINIT::defaultValue = 1.000000;
const std::string TUNINGL::TSMAXZ::itemName = "TSMAXZ";
const double TUNINGL::TSMAXZ::defaultValue = 365.000000;
const std::string TUNINGL::TSMINZ::itemName = "TSMINZ";
const double TUNINGL::TSMINZ::defaultValue = 0.100000;
const std::string TUNINGL::TSMCHP::itemName = "TSMCHP";
const double TUNINGL::TSMCHP::defaultValue = 0.150000;
const std::string TUNINGL::TSFMAX::itemName = "TSFMAX";
const double TUNINGL::TSFMAX::defaultValue = 3.000000;
const std::string TUNINGL::TSFMIN::itemName = "TSFMIN";
const double TUNINGL::TSFMIN::defaultValue = 0.300000;
const std::string TUNINGL::TSFCNV::itemName = "TSFCNV";
const double TUNINGL::TSFCNV::defaultValue = 0.100000;
const std::string TUNINGL::TFDIFF::itemName = "TFDIFF";
const double TUNINGL::TFDIFF::defaultValue = 1.250000;
const std::string TUNINGL::THRUPT::itemName = "THRUPT";
const double TUNINGL::THRUPT::defaultValue = 100000000000000000000.000000;
const std::string TUNINGL::TMAXWC::itemName = "TMAXWC";
const std::string TUNINGL::TRGTTE::itemName = "TRGTTE";
const double TUNINGL::TRGTTE::defaultValue = 0.100000;
const std::string TUNINGL::TRGCNV::itemName = "TRGCNV";
const double TUNINGL::TRGCNV::defaultValue = 0.001000;
const std::string TUNINGL::TRGMBE::itemName = "TRGMBE";
const double TUNINGL::TRGMBE::defaultValue = 0.000000;
const std::string TUNINGL::TRGLCV::itemName = "TRGLCV";
const double TUNINGL::TRGLCV::defaultValue = 0.000100;
const std::string TUNINGL::XXXTTE::itemName = "XXXTTE";
const double TUNINGL::XXXTTE::defaultValue = 10.000000;
const std::string TUNINGL::XXXCNV::itemName = "XXXCNV";
const double TUNINGL::XXXCNV::defaultValue = 0.010000;
const std::string TUNINGL::XXXMBE::itemName = "XXXMBE";
const double TUNINGL::XXXMBE::defaultValue = 0.000001;
const std::string TUNINGL::XXXLCV::itemName = "XXXLCV";
const double TUNINGL::XXXLCV::defaultValue = 0.001000;
const std::string TUNINGL::XXXWFL::itemName = "XXXWFL";
const double TUNINGL::XXXWFL::defaultValue = 0.001000;
const std::string TUNINGL::TRGFIP::itemName = "TRGFIP";
const double TUNINGL::TRGFIP::defaultValue = 0.025000;
const std::string TUNINGL::TRGSFT::itemName = "TRGSFT";
const std::string TUNINGL::THIONX::itemName = "THIONX";
const double TUNINGL::THIONX::defaultValue = 0.010000;
const std::string TUNINGL::TRWGHT::itemName = "TRWGHT";
const int TUNINGL::TRWGHT::defaultValue = 1;
const std::string TUNINGL::NEWTMX::itemName = "NEWTMX";
const int TUNINGL::NEWTMX::defaultValue = 12;
const std::string TUNINGL::NEWTMN::itemName = "NEWTMN";
const int TUNINGL::NEWTMN::defaultValue = 1;
const std::string TUNINGL::LITMAX::itemName = "LITMAX";
const int TUNINGL::LITMAX::defaultValue = 25;
const std::string TUNINGL::LITMIN::itemName = "LITMIN";
const int TUNINGL::LITMIN::defaultValue = 1;
const std::string TUNINGL::MXWSIT::itemName = "MXWSIT";
const int TUNINGL::MXWSIT::defaultValue = 8;
const std::string TUNINGL::MXWPIT::itemName = "MXWPIT";
const int TUNINGL::MXWPIT::defaultValue = 8;
const std::string TUNINGL::DDPLIM::itemName = "DDPLIM";
const double TUNINGL::DDPLIM::defaultValue = 1000000.000000;
const std::string TUNINGL::DDSLIM::itemName = "DDSLIM";
const double TUNINGL::DDSLIM::defaultValue = 1000000.000000;
const std::string TUNINGL::TRGDPR::itemName = "TRGDPR";
const double TUNINGL::TRGDPR::defaultValue = 1000000.000000;
const std::string TUNINGL::XXXDPR::itemName = "XXXDPR";


TUNINGS::TUNINGS( ) : ParserKeyword("TUNINGS")
{
  setFixedSize( (size_t) 4);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNINGS");
  {
     ParserRecord record;
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TSINIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMAXZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(365.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMINZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSMCHP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.150000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("TSFMAX", ParserItem::itype::DOUBLE);
        item.setDefault( double(3.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFMIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.300000) );
        record.addItem(item);
     }
     {
        ParserItem item("TSFCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TFDIFF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.250000) );
        record.addItem(item);
     }
     {
        ParserItem item("THRUPT", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TMAXWC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TRGTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-07) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.000100) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXTTE", ParserItem::itype::DOUBLE);
        item.setDefault( double(10.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXCNV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXMBE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXLCV", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("XXXWFL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGFIP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.025000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGSFT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("THIONX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRWGHT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("NEWTMX", ParserItem::itype::INT);
        item.setDefault( 12 );
        record.addItem(item);
     }
     {
        ParserItem item("NEWTMN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMAX", ParserItem::itype::INT);
        item.setDefault( 25 );
        record.addItem(item);
     }
     {
        ParserItem item("LITMIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWSIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("MXWPIT", ParserItem::itype::INT);
        item.setDefault( 8 );
        record.addItem(item);
     }
     {
        ParserItem item("DDPLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DDSLIM", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("TRGDPR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("XXXDPR", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNINGS::keywordName = "TUNINGS";
const std::string TUNINGS::LGR::itemName = "LGR";
const std::string TUNINGS::TSINIT::itemName = "TSINIT";
const double TUNINGS::TSINIT::defaultValue = 1.000000;
const std::string TUNINGS::TSMAXZ::itemName = "TSMAXZ";
const double TUNINGS::TSMAXZ::defaultValue = 365.000000;
const std::string TUNINGS::TSMINZ::itemName = "TSMINZ";
const double TUNINGS::TSMINZ::defaultValue = 0.100000;
const std::string TUNINGS::TSMCHP::itemName = "TSMCHP";
const double TUNINGS::TSMCHP::defaultValue = 0.150000;
const std::string TUNINGS::TSFMAX::itemName = "TSFMAX";
const double TUNINGS::TSFMAX::defaultValue = 3.000000;
const std::string TUNINGS::TSFMIN::itemName = "TSFMIN";
const double TUNINGS::TSFMIN::defaultValue = 0.300000;
const std::string TUNINGS::TSFCNV::itemName = "TSFCNV";
const double TUNINGS::TSFCNV::defaultValue = 0.100000;
const std::string TUNINGS::TFDIFF::itemName = "TFDIFF";
const double TUNINGS::TFDIFF::defaultValue = 1.250000;
const std::string TUNINGS::THRUPT::itemName = "THRUPT";
const double TUNINGS::THRUPT::defaultValue = 100000000000000000000.000000;
const std::string TUNINGS::TMAXWC::itemName = "TMAXWC";
const std::string TUNINGS::TRGTTE::itemName = "TRGTTE";
const double TUNINGS::TRGTTE::defaultValue = 0.100000;
const std::string TUNINGS::TRGCNV::itemName = "TRGCNV";
const double TUNINGS::TRGCNV::defaultValue = 0.001000;
const std::string TUNINGS::TRGMBE::itemName = "TRGMBE";
const double TUNINGS::TRGMBE::defaultValue = 0.000000;
const std::string TUNINGS::TRGLCV::itemName = "TRGLCV";
const double TUNINGS::TRGLCV::defaultValue = 0.000100;
const std::string TUNINGS::XXXTTE::itemName = "XXXTTE";
const double TUNINGS::XXXTTE::defaultValue = 10.000000;
const std::string TUNINGS::XXXCNV::itemName = "XXXCNV";
const double TUNINGS::XXXCNV::defaultValue = 0.010000;
const std::string TUNINGS::XXXMBE::itemName = "XXXMBE";
const double TUNINGS::XXXMBE::defaultValue = 0.000001;
const std::string TUNINGS::XXXLCV::itemName = "XXXLCV";
const double TUNINGS::XXXLCV::defaultValue = 0.001000;
const std::string TUNINGS::XXXWFL::itemName = "XXXWFL";
const double TUNINGS::XXXWFL::defaultValue = 0.001000;
const std::string TUNINGS::TRGFIP::itemName = "TRGFIP";
const double TUNINGS::TRGFIP::defaultValue = 0.025000;
const std::string TUNINGS::TRGSFT::itemName = "TRGSFT";
const std::string TUNINGS::THIONX::itemName = "THIONX";
const double TUNINGS::THIONX::defaultValue = 0.010000;
const std::string TUNINGS::TRWGHT::itemName = "TRWGHT";
const int TUNINGS::TRWGHT::defaultValue = 1;
const std::string TUNINGS::NEWTMX::itemName = "NEWTMX";
const int TUNINGS::NEWTMX::defaultValue = 12;
const std::string TUNINGS::NEWTMN::itemName = "NEWTMN";
const int TUNINGS::NEWTMN::defaultValue = 1;
const std::string TUNINGS::LITMAX::itemName = "LITMAX";
const int TUNINGS::LITMAX::defaultValue = 25;
const std::string TUNINGS::LITMIN::itemName = "LITMIN";
const int TUNINGS::LITMIN::defaultValue = 1;
const std::string TUNINGS::MXWSIT::itemName = "MXWSIT";
const int TUNINGS::MXWSIT::defaultValue = 8;
const std::string TUNINGS::MXWPIT::itemName = "MXWPIT";
const int TUNINGS::MXWPIT::defaultValue = 8;
const std::string TUNINGS::DDPLIM::itemName = "DDPLIM";
const double TUNINGS::DDPLIM::defaultValue = 1000000.000000;
const std::string TUNINGS::DDSLIM::itemName = "DDSLIM";
const double TUNINGS::DDSLIM::defaultValue = 1000000.000000;
const std::string TUNINGS::TRGDPR::itemName = "TRGDPR";
const double TUNINGS::TRGDPR::defaultValue = 1000000.000000;
const std::string TUNINGS::XXXDPR::itemName = "XXXDPR";


TVDP::TVDP( ) : ParserKeyword("TVDP")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTTRVD",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  setMatchRegex("TVDP.+");
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
const std::string TVDP::keywordName = "TVDP";
const std::string TVDP::DATA::itemName = "DATA";


TZONE::TZONE( ) : ParserKeyword("TZONE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TZONE");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_SWITCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("WATER_SWITCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_SWITCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TZONE::keywordName = "TZONE";
const std::string TZONE::OIL_SWITCH::itemName = "OIL_SWITCH";
const std::string TZONE::WATER_SWITCH::itemName = "WATER_SWITCH";
const std::string TZONE::GAS_SWITCH::itemName = "GAS_SWITCH";


}
}
