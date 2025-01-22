
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/T.hpp>
namespace Opm {
namespace ParserKeywords {
TABDIMS::TABDIMS() : ParserKeyword("TABDIMS", KeywordSize(1, false)) {
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
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("NTENDP", ParserItem::itype::INT);
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
        item.setDefault( 1 );
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
const std::string TABDIMS::NTPVT::itemName = "NTPVT";
const std::string TABDIMS::NSSFUN::itemName = "NSSFUN";
const std::string TABDIMS::NPPVT::itemName = "NPPVT";
const std::string TABDIMS::NTFIP::itemName = "NTFIP";
const std::string TABDIMS::NRPVT::itemName = "NRPVT";
const std::string TABDIMS::MAX_RV_NODES::itemName = "MAX_RV_NODES";
const std::string TABDIMS::NTENDP::itemName = "NTENDP";
const std::string TABDIMS::NUM_EOS_RES::itemName = "NUM_EOS_RES";
const std::string TABDIMS::NUM_EOS_SURFACE::itemName = "NUM_EOS_SURFACE";
const std::string TABDIMS::MAX_FLUX_REGIONS::itemName = "MAX_FLUX_REGIONS";
const std::string TABDIMS::MAX_THERMAL_REGIONS::itemName = "MAX_THERMAL_REGIONS";
const std::string TABDIMS::NTROCC::itemName = "NTROCC";
const std::string TABDIMS::MAX_PRESSURE_MAINTAINANCE_REGIONS::itemName = "MAX_PRESSURE_MAINTAINANCE_REGIONS";
const std::string TABDIMS::MAX_KVALUE_TABLES::itemName = "MAX_KVALUE_TABLES";
const std::string TABDIMS::NTALPHA::itemName = "NTALPHA";
const std::string TABDIMS::ASPHALTENE_ASPKDAM_MAX_ROWS::itemName = "ASPHALTENE_ASPKDAM_MAX_ROWS";
const std::string TABDIMS::ASPHALTENE_ASPREWG_MAX_ROWS::itemName = "ASPHALTENE_ASPREWG_MAX_ROWS";
const std::string TABDIMS::ASPHALTENE_ASPVISO_MAX_ROWS::itemName = "ASPHALTENE_ASPVISO_MAX_ROWS";
const std::string TABDIMS::ITEM20_NOT_USED::itemName = "ITEM20_NOT_USED";
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_COLUMNS::itemName = "ASPHALTENE_ASPPW2D_MAX_COLUMNS";
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_ROWS::itemName = "ASPHALTENE_ASPPW2D_MAX_ROWS";
const std::string TABDIMS::ASPHALTENE_ASPWETF_MAX_ROWS::itemName = "ASPHALTENE_ASPWETF_MAX_ROWS";
const std::string TABDIMS::NUM_KVALUE_TABLES::itemName = "NUM_KVALUE_TABLES";
const std::string TABDIMS::RESERVED::itemName = "RESERVED";


TBLK::TBLK() : ParserKeyword("TBLK", KeywordSize(1, false)) {
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


TCRIT::TCRIT() : ParserKeyword("TCRIT", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TCRIT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TCRIT::keywordName = "TCRIT";
const std::string TCRIT::DATA::itemName = "DATA";


TEMP::TEMP() : ParserKeyword("TEMP", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TEMP");
}
const std::string TEMP::keywordName = "TEMP";


TEMPI::TEMPI() : ParserKeyword("TEMPI", KeywordSize(1, false)) {
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


TEMPNODE::TEMPNODE() : ParserKeyword("TEMPNODE", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
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


TEMPTVD::TEMPTVD() : ParserKeyword("TEMPTVD", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TEMPTVD");
}
const std::string TEMPTVD::keywordName = "TEMPTVD";


TEMPVD::TEMPVD() : ParserKeyword("TEMPVD", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  setProhibitedKeywords({
    "RTEMPVD",
  });
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


THCGAS::THCGAS() : ParserKeyword("THCGAS", KeywordSize(1, false)) {
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


THCO2MIX::THCO2MIX() : ParserKeyword("THCO2MIX", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("THCO2MIX");
  {
     ParserRecord record;
     {
        ParserItem item("MIXING_MODEL_SALT", ParserItem::itype::STRING);
        item.setDefault( std::string("MICHAELIDES") );
        record.addItem(item);
     }
     {
        ParserItem item("MIXING_MODEL_LIQUID", ParserItem::itype::STRING);
        item.setDefault( std::string("DUANSUN") );
        record.addItem(item);
     }
     {
        ParserItem item("MIXING_MODEL_GAS", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string THCO2MIX::keywordName = "THCO2MIX";
const std::string THCO2MIX::MIXING_MODEL_SALT::itemName = "MIXING_MODEL_SALT";
const std::string THCO2MIX::MIXING_MODEL_SALT::defaultValue = "MICHAELIDES";
const std::string THCO2MIX::MIXING_MODEL_LIQUID::itemName = "MIXING_MODEL_LIQUID";
const std::string THCO2MIX::MIXING_MODEL_LIQUID::defaultValue = "DUANSUN";
const std::string THCO2MIX::MIXING_MODEL_GAS::itemName = "MIXING_MODEL_GAS";
const std::string THCO2MIX::MIXING_MODEL_GAS::defaultValue = "NONE";


THCOIL::THCOIL() : ParserKeyword("THCOIL", KeywordSize(1, false)) {
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


THCONR::THCONR() : ParserKeyword("THCONR", KeywordSize(1, false)) {
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


THCONSF::THCONSF() : ParserKeyword("THCONSF", KeywordSize(1, false)) {
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


THCROCK::THCROCK() : ParserKeyword("THCROCK", KeywordSize(1, false)) {
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


THCWATER::THCWATER() : ParserKeyword("THCWATER", KeywordSize(1, false)) {
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


THELCOEF::THELCOEF() : ParserKeyword("THELCOEF", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THELCOEF");
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
const std::string THELCOEF::keywordName = "THELCOEF";
const std::string THELCOEF::data::itemName = "data";


THERMAL::THERMAL() : ParserKeyword("THERMAL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("THERMAL");
}
const std::string THERMAL::keywordName = "THERMAL";


THERMEXR::THERMEXR() : ParserKeyword("THERMEXR", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THERMEXR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1/AbsoluteTemperature");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THERMEXR::keywordName = "THERMEXR";
const std::string THERMEXR::data::itemName = "data";


THPRES::THPRES() : ParserKeyword("THPRES", KeywordSize(SLASH_TERMINATED)) {
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


THPRESFT::THPRESFT() : ParserKeyword("THPRESFT", KeywordSize(SLASH_TERMINATED)) {
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


TIGHTEN::TIGHTEN() : ParserKeyword("TIGHTEN", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TIGHTEN");
  {
     ParserRecord record;
     {
        ParserItem item("FACTOR", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string TIGHTEN::keywordName = "TIGHTEN";
const std::string TIGHTEN::FACTOR::itemName = "FACTOR";


TIGHTENP::TIGHTENP() : ParserKeyword("TIGHTENP", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TIGHTENP");
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
const std::string TIGHTENP::keywordName = "TIGHTENP";
const std::string TIGHTENP::LINEAR_FACTOR::itemName = "LINEAR_FACTOR";
const std::string TIGHTENP::MAX_LINEAR::itemName = "MAX_LINEAR";
const std::string TIGHTENP::NONLINEAR_FACTOR::itemName = "NONLINEAR_FACTOR";
const std::string TIGHTENP::MAX_NONLINEAR::itemName = "MAX_NONLINEAR";


TIME::TIME() : ParserKeyword("TIME", KeywordSize(1, false)) {
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


TITLE::TITLE() : ParserKeyword("TITLE", KeywordSize(1, false)) {
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


TLMIXPAR::TLMIXPAR() : ParserKeyword("TLMIXPAR", KeywordSize("MISCIBLE", "NTMISC", false, 0)) {
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


TLPMIXPA::TLPMIXPA() : ParserKeyword("TLPMIXPA", KeywordSize("MISCIBLE", "NTMISC", false, 0)) {
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


TNUM::TNUM() : ParserKeyword("TNUM", KeywordSize(1, false)) {
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


TOLCRIT::TOLCRIT() : ParserKeyword("TOLCRIT", KeywordSize(1, false)) {
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


TOPS::TOPS() : ParserKeyword("TOPS", KeywordSize(1, false)) {
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


TPAMEPS::TPAMEPS() : ParserKeyword("TPAMEPS", KeywordSize("REGDIMS", "NTCREG", false, 0)) {
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


TPAMEPSS::TPAMEPSS() : ParserKeyword("TPAMEPSS", KeywordSize("REGDIMS", "NTCREG", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TPAMEPSS");
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
const std::string TPAMEPSS::keywordName = "TPAMEPSS";
const std::string TPAMEPSS::DATA::itemName = "DATA";


TRACER::TRACER() : ParserKeyword("TRACER", KeywordSize(SLASH_TERMINATED)) {
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


TRACERKM::TRACERKM() : ParserKeyword("TRACERKM", KeywordSize("PARTTRAC", "NKPTMX", false, 2)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "PARTTRAC",
  });
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


TRACERKP::TRACERKP() : ParserKeyword("TRACERKP", KeywordSize("PARTTRAC", "NKPTMX", false, 0)) {
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


TRACERS::TRACERS() : ParserKeyword("TRACERS", KeywordSize(1, false)) {
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
const std::string TRACERS::MAX_WATER_TRACERS::itemName = "MAX_WATER_TRACERS";
const std::string TRACERS::MAX_GAS_TRACERS::itemName = "MAX_GAS_TRACERS";
const std::string TRACERS::MAX_ENV_TRACERS::itemName = "MAX_ENV_TRACERS";
const std::string TRACERS::NUMERIC_DIFF::itemName = "NUMERIC_DIFF";
const std::string TRACERS::NUMERIC_DIFF::defaultValue = "NODIFF";
const std::string TRACERS::MAX_ITER::itemName = "MAX_ITER";
const std::string TRACERS::MIN_ITER::itemName = "MIN_ITER";
const std::string TRACERS::PASSIVE_NONLINEAR::itemName = "PASSIVE_NONLINEAR";
const std::string TRACERS::PASSIVE_NONLINEAR::defaultValue = "NO";
const std::string TRACERS::ONEOFF_LIN_TIGHT::itemName = "ONEOFF_LIN_TIGHT";
const std::string TRACERS::ONEOFF_NLIN_TIGHT::itemName = "ONEOFF_NLIN_TIGHT";
const std::string TRACERS::TIGHTENING_FACTORS::itemName = "TIGHTENING_FACTORS";
const std::string TRACERS::NTIGHTFACTORS::itemName = "NTIGHTFACTORS";


TRACITVD::TRACITVD() : ParserKeyword("TRACITVD", KeywordSize(1, false)) {
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
const std::string TRACITVD::BOTH_TIMESTEP::itemName = "BOTH_TIMESTEP";
const std::string TRACITVD::BOTH_TIMESTEP::defaultValue = "YES";


TRACTVD::TRACTVD() : ParserKeyword("TRACTVD", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACTVD");
}
const std::string TRACTVD::keywordName = "TRACTVD";


TRADS::TRADS() : ParserKeyword("TRADS", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
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


TRANGL::TRANGL() : ParserKeyword("TRANGL", KeywordSize(SLASH_TERMINATED)) {
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


TRANR::TRANR() : ParserKeyword("TRANR", KeywordSize(1, false)) {
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


TRANTHT::TRANTHT() : ParserKeyword("TRANTHT", KeywordSize(1, false)) {
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


TRANX::TRANX() : ParserKeyword("TRANX", KeywordSize(1, false)) {
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


TRANY::TRANY() : ParserKeyword("TRANY", KeywordSize(1, false)) {
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


TRANZ::TRANZ() : ParserKeyword("TRANZ", KeywordSize(1, false)) {
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


TRDCY::TRDCY() : ParserKeyword("TRDCY", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
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


TRDIF::TRDIF() : ParserKeyword("TRDIF", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
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


TRDIS::TRDIS() : ParserKeyword("TRDIS", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
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


TREF::TREF() : ParserKeyword("TREF", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
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


TREFS::TREFS() : ParserKeyword("TREFS", KeywordSize("TABDIMS", "NUM_EOS_SURFACE", false, 0)) {
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


TRKPF::TRKPF() : ParserKeyword("TRKPF", KeywordSize(1, false)) {
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


TRNHD::TRNHD() : ParserKeyword("TRNHD", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  setMatchRegex("TRNHD.+");
}
const std::string TRNHD::keywordName = "TRNHD";


TRPLPORO::TRPLPORO() : ParserKeyword("TRPLPORO", KeywordSize(1, false)) {
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


TRROCK::TRROCK() : ParserKeyword("TRROCK", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
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


TSTEP::TSTEP() : ParserKeyword("TSTEP", KeywordSize(1, false)) {
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


TUNING::TUNING() : ParserKeyword("TUNING", KeywordSize(3, false)) {
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
const std::string TUNING::TSMAXZ::itemName = "TSMAXZ";
const std::string TUNING::TSMINZ::itemName = "TSMINZ";
const std::string TUNING::TSMCHP::itemName = "TSMCHP";
const std::string TUNING::TSFMAX::itemName = "TSFMAX";
const std::string TUNING::TSFMIN::itemName = "TSFMIN";
const std::string TUNING::TSFCNV::itemName = "TSFCNV";
const std::string TUNING::TFDIFF::itemName = "TFDIFF";
const std::string TUNING::THRUPT::itemName = "THRUPT";
const std::string TUNING::TMAXWC::itemName = "TMAXWC";
const std::string TUNING::TRGTTE::itemName = "TRGTTE";
const std::string TUNING::TRGCNV::itemName = "TRGCNV";
const std::string TUNING::TRGMBE::itemName = "TRGMBE";
const std::string TUNING::TRGLCV::itemName = "TRGLCV";
const std::string TUNING::XXXTTE::itemName = "XXXTTE";
const std::string TUNING::XXXCNV::itemName = "XXXCNV";
const std::string TUNING::XXXMBE::itemName = "XXXMBE";
const std::string TUNING::XXXLCV::itemName = "XXXLCV";
const std::string TUNING::XXXWFL::itemName = "XXXWFL";
const std::string TUNING::TRGFIP::itemName = "TRGFIP";
const std::string TUNING::TRGSFT::itemName = "TRGSFT";
const std::string TUNING::THIONX::itemName = "THIONX";
const std::string TUNING::TRWGHT::itemName = "TRWGHT";
const std::string TUNING::NEWTMX::itemName = "NEWTMX";
const std::string TUNING::NEWTMN::itemName = "NEWTMN";
const std::string TUNING::LITMAX::itemName = "LITMAX";
const std::string TUNING::LITMIN::itemName = "LITMIN";
const std::string TUNING::MXWSIT::itemName = "MXWSIT";
const std::string TUNING::MXWPIT::itemName = "MXWPIT";
const std::string TUNING::DDPLIM::itemName = "DDPLIM";
const std::string TUNING::DDSLIM::itemName = "DDSLIM";
const std::string TUNING::TRGDPR::itemName = "TRGDPR";
const std::string TUNING::XXXDPR::itemName = "XXXDPR";


TUNINGDP::TUNINGDP() : ParserKeyword("TUNINGDP", KeywordSize(1, false)) {
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


TUNINGH::TUNINGH() : ParserKeyword("TUNINGH", KeywordSize(1, false)) {
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
const std::string TUNINGH::GXXLCV::itemName = "GXXLCV";
const std::string TUNINGH::GMSLCV::itemName = "GMSLCV";
const std::string TUNINGH::LGTMIN::itemName = "LGTMIN";
const std::string TUNINGH::LGTMAX::itemName = "LGTMAX";


TUNINGL::TUNINGL() : ParserKeyword("TUNINGL", KeywordSize(3, false)) {
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
const std::string TUNINGL::TSMAXZ::itemName = "TSMAXZ";
const std::string TUNINGL::TSMINZ::itemName = "TSMINZ";
const std::string TUNINGL::TSMCHP::itemName = "TSMCHP";
const std::string TUNINGL::TSFMAX::itemName = "TSFMAX";
const std::string TUNINGL::TSFMIN::itemName = "TSFMIN";
const std::string TUNINGL::TSFCNV::itemName = "TSFCNV";
const std::string TUNINGL::TFDIFF::itemName = "TFDIFF";
const std::string TUNINGL::THRUPT::itemName = "THRUPT";
const std::string TUNINGL::TMAXWC::itemName = "TMAXWC";
const std::string TUNINGL::TRGTTE::itemName = "TRGTTE";
const std::string TUNINGL::TRGCNV::itemName = "TRGCNV";
const std::string TUNINGL::TRGMBE::itemName = "TRGMBE";
const std::string TUNINGL::TRGLCV::itemName = "TRGLCV";
const std::string TUNINGL::XXXTTE::itemName = "XXXTTE";
const std::string TUNINGL::XXXCNV::itemName = "XXXCNV";
const std::string TUNINGL::XXXMBE::itemName = "XXXMBE";
const std::string TUNINGL::XXXLCV::itemName = "XXXLCV";
const std::string TUNINGL::XXXWFL::itemName = "XXXWFL";
const std::string TUNINGL::TRGFIP::itemName = "TRGFIP";
const std::string TUNINGL::TRGSFT::itemName = "TRGSFT";
const std::string TUNINGL::THIONX::itemName = "THIONX";
const std::string TUNINGL::TRWGHT::itemName = "TRWGHT";
const std::string TUNINGL::NEWTMX::itemName = "NEWTMX";
const std::string TUNINGL::NEWTMN::itemName = "NEWTMN";
const std::string TUNINGL::LITMAX::itemName = "LITMAX";
const std::string TUNINGL::LITMIN::itemName = "LITMIN";
const std::string TUNINGL::MXWSIT::itemName = "MXWSIT";
const std::string TUNINGL::MXWPIT::itemName = "MXWPIT";
const std::string TUNINGL::DDPLIM::itemName = "DDPLIM";
const std::string TUNINGL::DDSLIM::itemName = "DDSLIM";
const std::string TUNINGL::TRGDPR::itemName = "TRGDPR";
const std::string TUNINGL::XXXDPR::itemName = "XXXDPR";


TUNINGS::TUNINGS() : ParserKeyword("TUNINGS", KeywordSize(4, false)) {
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
const std::string TUNINGS::TSMAXZ::itemName = "TSMAXZ";
const std::string TUNINGS::TSMINZ::itemName = "TSMINZ";
const std::string TUNINGS::TSMCHP::itemName = "TSMCHP";
const std::string TUNINGS::TSFMAX::itemName = "TSFMAX";
const std::string TUNINGS::TSFMIN::itemName = "TSFMIN";
const std::string TUNINGS::TSFCNV::itemName = "TSFCNV";
const std::string TUNINGS::TFDIFF::itemName = "TFDIFF";
const std::string TUNINGS::THRUPT::itemName = "THRUPT";
const std::string TUNINGS::TMAXWC::itemName = "TMAXWC";
const std::string TUNINGS::TRGTTE::itemName = "TRGTTE";
const std::string TUNINGS::TRGCNV::itemName = "TRGCNV";
const std::string TUNINGS::TRGMBE::itemName = "TRGMBE";
const std::string TUNINGS::TRGLCV::itemName = "TRGLCV";
const std::string TUNINGS::XXXTTE::itemName = "XXXTTE";
const std::string TUNINGS::XXXCNV::itemName = "XXXCNV";
const std::string TUNINGS::XXXMBE::itemName = "XXXMBE";
const std::string TUNINGS::XXXLCV::itemName = "XXXLCV";
const std::string TUNINGS::XXXWFL::itemName = "XXXWFL";
const std::string TUNINGS::TRGFIP::itemName = "TRGFIP";
const std::string TUNINGS::TRGSFT::itemName = "TRGSFT";
const std::string TUNINGS::THIONX::itemName = "THIONX";
const std::string TUNINGS::TRWGHT::itemName = "TRWGHT";
const std::string TUNINGS::NEWTMX::itemName = "NEWTMX";
const std::string TUNINGS::NEWTMN::itemName = "NEWTMN";
const std::string TUNINGS::LITMAX::itemName = "LITMAX";
const std::string TUNINGS::LITMIN::itemName = "LITMIN";
const std::string TUNINGS::MXWSIT::itemName = "MXWSIT";
const std::string TUNINGS::MXWPIT::itemName = "MXWPIT";
const std::string TUNINGS::DDPLIM::itemName = "DDPLIM";
const std::string TUNINGS::DDSLIM::itemName = "DDSLIM";
const std::string TUNINGS::TRGDPR::itemName = "TRGDPR";
const std::string TUNINGS::XXXDPR::itemName = "XXXDPR";


TVDP::TVDP() : ParserKeyword("TVDP", KeywordSize("EQLDIMS", "NTTRVD", false, 0)) {
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


TZONE::TZONE() : ParserKeyword("TZONE", KeywordSize(1, false)) {
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
