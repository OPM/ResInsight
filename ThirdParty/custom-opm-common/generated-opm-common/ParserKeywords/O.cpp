
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/O.hpp>
namespace Opm {
namespace ParserKeywords {
OCOMPIDX::OCOMPIDX() : ParserKeyword("OCOMPIDX", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("OCOMPIDX");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_COMPONENT_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OCOMPIDX::keywordName = "OCOMPIDX";
const std::string OCOMPIDX::OIL_COMPONENT_INDEX::itemName = "OIL_COMPONENT_INDEX";


OFM::OFM() : ParserKeyword("OFM", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("OFM");
}
const std::string OFM::keywordName = "OFM";


OIL::OIL() : ParserKeyword("OIL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("OIL");
}
const std::string OIL::keywordName = "OIL";


OILAPI::OILAPI() : ParserKeyword("OILAPI", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("OILAPI");
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
const std::string OILAPI::keywordName = "OILAPI";
const std::string OILAPI::data::itemName = "data";


OILCOMPR::OILCOMPR() : ParserKeyword("OILCOMPR", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILCOMPR");
  {
     ParserRecord record;
     {
        ParserItem item("COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
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
const std::string OILCOMPR::keywordName = "OILCOMPR";
const std::string OILCOMPR::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";
const std::string OILCOMPR::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const std::string OILCOMPR::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";


OILDENT::OILDENT() : ParserKeyword("OILDENT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILDENT");
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
const std::string OILDENT::keywordName = "OILDENT";
const std::string OILDENT::REFERENCE_TEMPERATURE::itemName = "REFERENCE_TEMPERATURE";
const std::string OILDENT::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const std::string OILDENT::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";


OILJT::OILJT() : ParserKeyword("OILJT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILJT");
  {
     ParserRecord record;
     {
        ParserItem item("PREF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.013200) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("JOULE_THOMSON_COEFFICIENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("AbsoluteTemperature/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILJT::keywordName = "OILJT";
const std::string OILJT::PREF::itemName = "PREF";
const std::string OILJT::JOULE_THOMSON_COEFFICIENT::itemName = "JOULE_THOMSON_COEFFICIENT";


OILMW::OILMW() : ParserKeyword("OILMW", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILMW");
  {
     ParserRecord record;
     {
        ParserItem item("MOLAR_WEIGHT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILMW::keywordName = "OILMW";
const std::string OILMW::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


OILVISCT::OILVISCT() : ParserKeyword("OILVISCT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILVISCT");
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
const std::string OILVISCT::keywordName = "OILVISCT";
const std::string OILVISCT::DATA::itemName = "DATA";


OILVTIM::OILVTIM() : ParserKeyword("OILVTIM", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILVTIM");
  {
     ParserRecord record;
     {
        ParserItem item("INTERPOLATION_METHOD", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILVTIM::keywordName = "OILVTIM";
const std::string OILVTIM::INTERPOLATION_METHOD::itemName = "INTERPOLATION_METHOD";


OLDTRAN::OLDTRAN() : ParserKeyword("OLDTRAN", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("OLDTRAN");
}
const std::string OLDTRAN::keywordName = "OLDTRAN";


OLDTRANR::OLDTRANR() : ParserKeyword("OLDTRANR", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("OLDTRANR");
}
const std::string OLDTRANR::keywordName = "OLDTRANR";


OPERATE::OPERATE() : ParserKeyword("OPERATE", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("OPERATE");
  {
     ParserRecord record;
     {
        ParserItem item("TARGET_ARRAY", ParserItem::itype::STRING);
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
        ParserItem item("OPERATION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ARRAY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PARAM1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("PARAM2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPERATE::keywordName = "OPERATE";
const std::string OPERATE::TARGET_ARRAY::itemName = "TARGET_ARRAY";
const std::string OPERATE::I1::itemName = "I1";
const std::string OPERATE::I2::itemName = "I2";
const std::string OPERATE::J1::itemName = "J1";
const std::string OPERATE::J2::itemName = "J2";
const std::string OPERATE::K1::itemName = "K1";
const std::string OPERATE::K2::itemName = "K2";
const std::string OPERATE::OPERATION::itemName = "OPERATION";
const std::string OPERATE::ARRAY::itemName = "ARRAY";
const std::string OPERATE::PARAM1::itemName = "PARAM1";
const std::string OPERATE::PARAM2::itemName = "PARAM2";


OPERATER::OPERATER() : ParserKeyword("OPERATER", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("OPERATER");
  {
     ParserRecord record;
     {
        ParserItem item("TARGET_ARRAY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("OPERATION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ARRAY_PARAMETER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PARAM1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("PARAM2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NAME", ParserItem::itype::STRING);
        item.setDefault( std::string("OPERNUM") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPERATER::keywordName = "OPERATER";
const std::string OPERATER::TARGET_ARRAY::itemName = "TARGET_ARRAY";
const std::string OPERATER::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string OPERATER::OPERATION::itemName = "OPERATION";
const std::string OPERATER::ARRAY_PARAMETER::itemName = "ARRAY_PARAMETER";
const std::string OPERATER::PARAM1::itemName = "PARAM1";
const std::string OPERATER::PARAM2::itemName = "PARAM2";
const std::string OPERATER::REGION_NAME::itemName = "REGION_NAME";
const std::string OPERATER::REGION_NAME::defaultValue = "OPERNUM";


OPERNUM::OPERNUM() : ParserKeyword("OPERNUM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("OPERNUM");
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
const std::string OPERNUM::keywordName = "OPERNUM";
const std::string OPERNUM::data::itemName = "data";


OPTIONS::OPTIONS() : ParserKeyword("OPTIONS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("OPTIONS");
  {
     ParserRecord record;
     {
        ParserItem item("ITEM1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM3", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM4", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM5", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM6", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM7", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM8", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM9", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM10", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM11", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM12", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM13", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM14", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM15", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM16", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM17", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM18", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM19", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM20", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM21", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM22", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM23", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM24", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM25", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM26", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM27", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM28", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM29", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM30", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM31", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM32", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM33", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM34", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM35", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM36", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM37", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM38", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM39", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM40", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM41", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM42", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM43", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM44", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM45", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM46", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM47", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM48", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM49", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM50", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM51", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM52", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM53", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM54", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM55", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM56", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM57", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM58", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM59", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM60", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM61", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM62", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM63", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM64", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM65", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM66", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM67", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM68", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM69", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM70", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM71", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM72", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM73", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM74", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM75", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM76", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM77", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM78", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM79", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM80", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM81", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM82", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM83", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM84", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM85", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM86", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM87", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM88", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM89", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM90", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM91", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM92", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM93", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM94", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM95", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM96", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM97", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM98", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM99", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM100", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM101", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM102", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM103", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM104", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM105", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM106", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM107", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM108", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM109", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM110", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM111", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM112", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM113", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM114", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM115", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM116", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM117", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM118", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM119", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM120", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM121", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM122", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM123", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM124", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM125", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM126", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM127", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM128", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM129", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM130", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM131", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM132", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM133", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM134", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM135", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM136", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM137", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM138", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM139", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM140", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM141", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM142", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM143", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM144", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM145", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM146", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM147", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM148", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM149", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM150", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM151", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM152", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM153", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM154", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM155", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM156", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM157", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM158", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM159", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM160", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM161", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM162", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM163", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM164", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM165", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM166", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM167", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM168", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM169", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM170", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM171", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM172", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM173", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM174", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM175", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM176", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM177", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM178", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM179", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM180", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM181", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM182", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM183", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM184", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM185", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM186", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM187", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM188", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM189", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM190", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM191", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM192", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM193", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM194", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM195", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM196", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM197", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM198", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM199", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM200", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM201", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM202", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM203", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM204", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM205", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM206", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM207", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM208", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM209", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM210", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM211", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM212", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM213", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM214", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM215", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM216", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM217", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM218", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM219", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM220", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM221", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM222", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM223", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM224", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM225", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM226", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM227", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM228", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM229", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM230", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM231", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM232", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM233", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM234", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM235", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM236", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM237", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM238", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM239", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM240", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM241", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM242", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM243", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM244", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM245", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM246", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM247", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM248", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM249", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM250", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM251", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM252", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM253", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM254", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM255", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM256", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM257", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM258", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM259", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM260", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM261", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM262", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM263", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM264", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM265", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM266", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM267", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM268", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM269", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM270", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM271", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM272", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM273", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM274", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM275", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM276", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM277", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM278", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM279", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM280", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM281", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM282", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM283", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM284", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM285", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM286", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM287", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM288", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM289", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM290", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM291", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM292", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM293", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM294", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM295", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM296", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM297", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM298", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM299", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM300", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM301", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM302", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM303", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM304", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM305", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM306", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM307", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM308", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM309", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM310", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM311", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM312", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM313", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM314", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM315", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM316", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM317", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM318", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM319", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPTIONS::keywordName = "OPTIONS";
const std::string OPTIONS::ITEM1::itemName = "ITEM1";
const std::string OPTIONS::ITEM2::itemName = "ITEM2";
const std::string OPTIONS::ITEM3::itemName = "ITEM3";
const std::string OPTIONS::ITEM4::itemName = "ITEM4";
const std::string OPTIONS::ITEM5::itemName = "ITEM5";
const std::string OPTIONS::ITEM6::itemName = "ITEM6";
const std::string OPTIONS::ITEM7::itemName = "ITEM7";
const std::string OPTIONS::ITEM8::itemName = "ITEM8";
const std::string OPTIONS::ITEM9::itemName = "ITEM9";
const std::string OPTIONS::ITEM10::itemName = "ITEM10";
const std::string OPTIONS::ITEM11::itemName = "ITEM11";
const std::string OPTIONS::ITEM12::itemName = "ITEM12";
const std::string OPTIONS::ITEM13::itemName = "ITEM13";
const std::string OPTIONS::ITEM14::itemName = "ITEM14";
const std::string OPTIONS::ITEM15::itemName = "ITEM15";
const std::string OPTIONS::ITEM16::itemName = "ITEM16";
const std::string OPTIONS::ITEM17::itemName = "ITEM17";
const std::string OPTIONS::ITEM18::itemName = "ITEM18";
const std::string OPTIONS::ITEM19::itemName = "ITEM19";
const std::string OPTIONS::ITEM20::itemName = "ITEM20";
const std::string OPTIONS::ITEM21::itemName = "ITEM21";
const std::string OPTIONS::ITEM22::itemName = "ITEM22";
const std::string OPTIONS::ITEM23::itemName = "ITEM23";
const std::string OPTIONS::ITEM24::itemName = "ITEM24";
const std::string OPTIONS::ITEM25::itemName = "ITEM25";
const std::string OPTIONS::ITEM26::itemName = "ITEM26";
const std::string OPTIONS::ITEM27::itemName = "ITEM27";
const std::string OPTIONS::ITEM28::itemName = "ITEM28";
const std::string OPTIONS::ITEM29::itemName = "ITEM29";
const std::string OPTIONS::ITEM30::itemName = "ITEM30";
const std::string OPTIONS::ITEM31::itemName = "ITEM31";
const std::string OPTIONS::ITEM32::itemName = "ITEM32";
const std::string OPTIONS::ITEM33::itemName = "ITEM33";
const std::string OPTIONS::ITEM34::itemName = "ITEM34";
const std::string OPTIONS::ITEM35::itemName = "ITEM35";
const std::string OPTIONS::ITEM36::itemName = "ITEM36";
const std::string OPTIONS::ITEM37::itemName = "ITEM37";
const std::string OPTIONS::ITEM38::itemName = "ITEM38";
const std::string OPTIONS::ITEM39::itemName = "ITEM39";
const std::string OPTIONS::ITEM40::itemName = "ITEM40";
const std::string OPTIONS::ITEM41::itemName = "ITEM41";
const std::string OPTIONS::ITEM42::itemName = "ITEM42";
const std::string OPTIONS::ITEM43::itemName = "ITEM43";
const std::string OPTIONS::ITEM44::itemName = "ITEM44";
const std::string OPTIONS::ITEM45::itemName = "ITEM45";
const std::string OPTIONS::ITEM46::itemName = "ITEM46";
const std::string OPTIONS::ITEM47::itemName = "ITEM47";
const std::string OPTIONS::ITEM48::itemName = "ITEM48";
const std::string OPTIONS::ITEM49::itemName = "ITEM49";
const std::string OPTIONS::ITEM50::itemName = "ITEM50";
const std::string OPTIONS::ITEM51::itemName = "ITEM51";
const std::string OPTIONS::ITEM52::itemName = "ITEM52";
const std::string OPTIONS::ITEM53::itemName = "ITEM53";
const std::string OPTIONS::ITEM54::itemName = "ITEM54";
const std::string OPTIONS::ITEM55::itemName = "ITEM55";
const std::string OPTIONS::ITEM56::itemName = "ITEM56";
const std::string OPTIONS::ITEM57::itemName = "ITEM57";
const std::string OPTIONS::ITEM58::itemName = "ITEM58";
const std::string OPTIONS::ITEM59::itemName = "ITEM59";
const std::string OPTIONS::ITEM60::itemName = "ITEM60";
const std::string OPTIONS::ITEM61::itemName = "ITEM61";
const std::string OPTIONS::ITEM62::itemName = "ITEM62";
const std::string OPTIONS::ITEM63::itemName = "ITEM63";
const std::string OPTIONS::ITEM64::itemName = "ITEM64";
const std::string OPTIONS::ITEM65::itemName = "ITEM65";
const std::string OPTIONS::ITEM66::itemName = "ITEM66";
const std::string OPTIONS::ITEM67::itemName = "ITEM67";
const std::string OPTIONS::ITEM68::itemName = "ITEM68";
const std::string OPTIONS::ITEM69::itemName = "ITEM69";
const std::string OPTIONS::ITEM70::itemName = "ITEM70";
const std::string OPTIONS::ITEM71::itemName = "ITEM71";
const std::string OPTIONS::ITEM72::itemName = "ITEM72";
const std::string OPTIONS::ITEM73::itemName = "ITEM73";
const std::string OPTIONS::ITEM74::itemName = "ITEM74";
const std::string OPTIONS::ITEM75::itemName = "ITEM75";
const std::string OPTIONS::ITEM76::itemName = "ITEM76";
const std::string OPTIONS::ITEM77::itemName = "ITEM77";
const std::string OPTIONS::ITEM78::itemName = "ITEM78";
const std::string OPTIONS::ITEM79::itemName = "ITEM79";
const std::string OPTIONS::ITEM80::itemName = "ITEM80";
const std::string OPTIONS::ITEM81::itemName = "ITEM81";
const std::string OPTIONS::ITEM82::itemName = "ITEM82";
const std::string OPTIONS::ITEM83::itemName = "ITEM83";
const std::string OPTIONS::ITEM84::itemName = "ITEM84";
const std::string OPTIONS::ITEM85::itemName = "ITEM85";
const std::string OPTIONS::ITEM86::itemName = "ITEM86";
const std::string OPTIONS::ITEM87::itemName = "ITEM87";
const std::string OPTIONS::ITEM88::itemName = "ITEM88";
const std::string OPTIONS::ITEM89::itemName = "ITEM89";
const std::string OPTIONS::ITEM90::itemName = "ITEM90";
const std::string OPTIONS::ITEM91::itemName = "ITEM91";
const std::string OPTIONS::ITEM92::itemName = "ITEM92";
const std::string OPTIONS::ITEM93::itemName = "ITEM93";
const std::string OPTIONS::ITEM94::itemName = "ITEM94";
const std::string OPTIONS::ITEM95::itemName = "ITEM95";
const std::string OPTIONS::ITEM96::itemName = "ITEM96";
const std::string OPTIONS::ITEM97::itemName = "ITEM97";
const std::string OPTIONS::ITEM98::itemName = "ITEM98";
const std::string OPTIONS::ITEM99::itemName = "ITEM99";
const std::string OPTIONS::ITEM100::itemName = "ITEM100";
const std::string OPTIONS::ITEM101::itemName = "ITEM101";
const std::string OPTIONS::ITEM102::itemName = "ITEM102";
const std::string OPTIONS::ITEM103::itemName = "ITEM103";
const std::string OPTIONS::ITEM104::itemName = "ITEM104";
const std::string OPTIONS::ITEM105::itemName = "ITEM105";
const std::string OPTIONS::ITEM106::itemName = "ITEM106";
const std::string OPTIONS::ITEM107::itemName = "ITEM107";
const std::string OPTIONS::ITEM108::itemName = "ITEM108";
const std::string OPTIONS::ITEM109::itemName = "ITEM109";
const std::string OPTIONS::ITEM110::itemName = "ITEM110";
const std::string OPTIONS::ITEM111::itemName = "ITEM111";
const std::string OPTIONS::ITEM112::itemName = "ITEM112";
const std::string OPTIONS::ITEM113::itemName = "ITEM113";
const std::string OPTIONS::ITEM114::itemName = "ITEM114";
const std::string OPTIONS::ITEM115::itemName = "ITEM115";
const std::string OPTIONS::ITEM116::itemName = "ITEM116";
const std::string OPTIONS::ITEM117::itemName = "ITEM117";
const std::string OPTIONS::ITEM118::itemName = "ITEM118";
const std::string OPTIONS::ITEM119::itemName = "ITEM119";
const std::string OPTIONS::ITEM120::itemName = "ITEM120";
const std::string OPTIONS::ITEM121::itemName = "ITEM121";
const std::string OPTIONS::ITEM122::itemName = "ITEM122";
const std::string OPTIONS::ITEM123::itemName = "ITEM123";
const std::string OPTIONS::ITEM124::itemName = "ITEM124";
const std::string OPTIONS::ITEM125::itemName = "ITEM125";
const std::string OPTIONS::ITEM126::itemName = "ITEM126";
const std::string OPTIONS::ITEM127::itemName = "ITEM127";
const std::string OPTIONS::ITEM128::itemName = "ITEM128";
const std::string OPTIONS::ITEM129::itemName = "ITEM129";
const std::string OPTIONS::ITEM130::itemName = "ITEM130";
const std::string OPTIONS::ITEM131::itemName = "ITEM131";
const std::string OPTIONS::ITEM132::itemName = "ITEM132";
const std::string OPTIONS::ITEM133::itemName = "ITEM133";
const std::string OPTIONS::ITEM134::itemName = "ITEM134";
const std::string OPTIONS::ITEM135::itemName = "ITEM135";
const std::string OPTIONS::ITEM136::itemName = "ITEM136";
const std::string OPTIONS::ITEM137::itemName = "ITEM137";
const std::string OPTIONS::ITEM138::itemName = "ITEM138";
const std::string OPTIONS::ITEM139::itemName = "ITEM139";
const std::string OPTIONS::ITEM140::itemName = "ITEM140";
const std::string OPTIONS::ITEM141::itemName = "ITEM141";
const std::string OPTIONS::ITEM142::itemName = "ITEM142";
const std::string OPTIONS::ITEM143::itemName = "ITEM143";
const std::string OPTIONS::ITEM144::itemName = "ITEM144";
const std::string OPTIONS::ITEM145::itemName = "ITEM145";
const std::string OPTIONS::ITEM146::itemName = "ITEM146";
const std::string OPTIONS::ITEM147::itemName = "ITEM147";
const std::string OPTIONS::ITEM148::itemName = "ITEM148";
const std::string OPTIONS::ITEM149::itemName = "ITEM149";
const std::string OPTIONS::ITEM150::itemName = "ITEM150";
const std::string OPTIONS::ITEM151::itemName = "ITEM151";
const std::string OPTIONS::ITEM152::itemName = "ITEM152";
const std::string OPTIONS::ITEM153::itemName = "ITEM153";
const std::string OPTIONS::ITEM154::itemName = "ITEM154";
const std::string OPTIONS::ITEM155::itemName = "ITEM155";
const std::string OPTIONS::ITEM156::itemName = "ITEM156";
const std::string OPTIONS::ITEM157::itemName = "ITEM157";
const std::string OPTIONS::ITEM158::itemName = "ITEM158";
const std::string OPTIONS::ITEM159::itemName = "ITEM159";
const std::string OPTIONS::ITEM160::itemName = "ITEM160";
const std::string OPTIONS::ITEM161::itemName = "ITEM161";
const std::string OPTIONS::ITEM162::itemName = "ITEM162";
const std::string OPTIONS::ITEM163::itemName = "ITEM163";
const std::string OPTIONS::ITEM164::itemName = "ITEM164";
const std::string OPTIONS::ITEM165::itemName = "ITEM165";
const std::string OPTIONS::ITEM166::itemName = "ITEM166";
const std::string OPTIONS::ITEM167::itemName = "ITEM167";
const std::string OPTIONS::ITEM168::itemName = "ITEM168";
const std::string OPTIONS::ITEM169::itemName = "ITEM169";
const std::string OPTIONS::ITEM170::itemName = "ITEM170";
const std::string OPTIONS::ITEM171::itemName = "ITEM171";
const std::string OPTIONS::ITEM172::itemName = "ITEM172";
const std::string OPTIONS::ITEM173::itemName = "ITEM173";
const std::string OPTIONS::ITEM174::itemName = "ITEM174";
const std::string OPTIONS::ITEM175::itemName = "ITEM175";
const std::string OPTIONS::ITEM176::itemName = "ITEM176";
const std::string OPTIONS::ITEM177::itemName = "ITEM177";
const std::string OPTIONS::ITEM178::itemName = "ITEM178";
const std::string OPTIONS::ITEM179::itemName = "ITEM179";
const std::string OPTIONS::ITEM180::itemName = "ITEM180";
const std::string OPTIONS::ITEM181::itemName = "ITEM181";
const std::string OPTIONS::ITEM182::itemName = "ITEM182";
const std::string OPTIONS::ITEM183::itemName = "ITEM183";
const std::string OPTIONS::ITEM184::itemName = "ITEM184";
const std::string OPTIONS::ITEM185::itemName = "ITEM185";
const std::string OPTIONS::ITEM186::itemName = "ITEM186";
const std::string OPTIONS::ITEM187::itemName = "ITEM187";
const std::string OPTIONS::ITEM188::itemName = "ITEM188";
const std::string OPTIONS::ITEM189::itemName = "ITEM189";
const std::string OPTIONS::ITEM190::itemName = "ITEM190";
const std::string OPTIONS::ITEM191::itemName = "ITEM191";
const std::string OPTIONS::ITEM192::itemName = "ITEM192";
const std::string OPTIONS::ITEM193::itemName = "ITEM193";
const std::string OPTIONS::ITEM194::itemName = "ITEM194";
const std::string OPTIONS::ITEM195::itemName = "ITEM195";
const std::string OPTIONS::ITEM196::itemName = "ITEM196";
const std::string OPTIONS::ITEM197::itemName = "ITEM197";
const std::string OPTIONS::ITEM198::itemName = "ITEM198";
const std::string OPTIONS::ITEM199::itemName = "ITEM199";
const std::string OPTIONS::ITEM200::itemName = "ITEM200";
const std::string OPTIONS::ITEM201::itemName = "ITEM201";
const std::string OPTIONS::ITEM202::itemName = "ITEM202";
const std::string OPTIONS::ITEM203::itemName = "ITEM203";
const std::string OPTIONS::ITEM204::itemName = "ITEM204";
const std::string OPTIONS::ITEM205::itemName = "ITEM205";
const std::string OPTIONS::ITEM206::itemName = "ITEM206";
const std::string OPTIONS::ITEM207::itemName = "ITEM207";
const std::string OPTIONS::ITEM208::itemName = "ITEM208";
const std::string OPTIONS::ITEM209::itemName = "ITEM209";
const std::string OPTIONS::ITEM210::itemName = "ITEM210";
const std::string OPTIONS::ITEM211::itemName = "ITEM211";
const std::string OPTIONS::ITEM212::itemName = "ITEM212";
const std::string OPTIONS::ITEM213::itemName = "ITEM213";
const std::string OPTIONS::ITEM214::itemName = "ITEM214";
const std::string OPTIONS::ITEM215::itemName = "ITEM215";
const std::string OPTIONS::ITEM216::itemName = "ITEM216";
const std::string OPTIONS::ITEM217::itemName = "ITEM217";
const std::string OPTIONS::ITEM218::itemName = "ITEM218";
const std::string OPTIONS::ITEM219::itemName = "ITEM219";
const std::string OPTIONS::ITEM220::itemName = "ITEM220";
const std::string OPTIONS::ITEM221::itemName = "ITEM221";
const std::string OPTIONS::ITEM222::itemName = "ITEM222";
const std::string OPTIONS::ITEM223::itemName = "ITEM223";
const std::string OPTIONS::ITEM224::itemName = "ITEM224";
const std::string OPTIONS::ITEM225::itemName = "ITEM225";
const std::string OPTIONS::ITEM226::itemName = "ITEM226";
const std::string OPTIONS::ITEM227::itemName = "ITEM227";
const std::string OPTIONS::ITEM228::itemName = "ITEM228";
const std::string OPTIONS::ITEM229::itemName = "ITEM229";
const std::string OPTIONS::ITEM230::itemName = "ITEM230";
const std::string OPTIONS::ITEM231::itemName = "ITEM231";
const std::string OPTIONS::ITEM232::itemName = "ITEM232";
const std::string OPTIONS::ITEM233::itemName = "ITEM233";
const std::string OPTIONS::ITEM234::itemName = "ITEM234";
const std::string OPTIONS::ITEM235::itemName = "ITEM235";
const std::string OPTIONS::ITEM236::itemName = "ITEM236";
const std::string OPTIONS::ITEM237::itemName = "ITEM237";
const std::string OPTIONS::ITEM238::itemName = "ITEM238";
const std::string OPTIONS::ITEM239::itemName = "ITEM239";
const std::string OPTIONS::ITEM240::itemName = "ITEM240";
const std::string OPTIONS::ITEM241::itemName = "ITEM241";
const std::string OPTIONS::ITEM242::itemName = "ITEM242";
const std::string OPTIONS::ITEM243::itemName = "ITEM243";
const std::string OPTIONS::ITEM244::itemName = "ITEM244";
const std::string OPTIONS::ITEM245::itemName = "ITEM245";
const std::string OPTIONS::ITEM246::itemName = "ITEM246";
const std::string OPTIONS::ITEM247::itemName = "ITEM247";
const std::string OPTIONS::ITEM248::itemName = "ITEM248";
const std::string OPTIONS::ITEM249::itemName = "ITEM249";
const std::string OPTIONS::ITEM250::itemName = "ITEM250";
const std::string OPTIONS::ITEM251::itemName = "ITEM251";
const std::string OPTIONS::ITEM252::itemName = "ITEM252";
const std::string OPTIONS::ITEM253::itemName = "ITEM253";
const std::string OPTIONS::ITEM254::itemName = "ITEM254";
const std::string OPTIONS::ITEM255::itemName = "ITEM255";
const std::string OPTIONS::ITEM256::itemName = "ITEM256";
const std::string OPTIONS::ITEM257::itemName = "ITEM257";
const std::string OPTIONS::ITEM258::itemName = "ITEM258";
const std::string OPTIONS::ITEM259::itemName = "ITEM259";
const std::string OPTIONS::ITEM260::itemName = "ITEM260";
const std::string OPTIONS::ITEM261::itemName = "ITEM261";
const std::string OPTIONS::ITEM262::itemName = "ITEM262";
const std::string OPTIONS::ITEM263::itemName = "ITEM263";
const std::string OPTIONS::ITEM264::itemName = "ITEM264";
const std::string OPTIONS::ITEM265::itemName = "ITEM265";
const std::string OPTIONS::ITEM266::itemName = "ITEM266";
const std::string OPTIONS::ITEM267::itemName = "ITEM267";
const std::string OPTIONS::ITEM268::itemName = "ITEM268";
const std::string OPTIONS::ITEM269::itemName = "ITEM269";
const std::string OPTIONS::ITEM270::itemName = "ITEM270";
const std::string OPTIONS::ITEM271::itemName = "ITEM271";
const std::string OPTIONS::ITEM272::itemName = "ITEM272";
const std::string OPTIONS::ITEM273::itemName = "ITEM273";
const std::string OPTIONS::ITEM274::itemName = "ITEM274";
const std::string OPTIONS::ITEM275::itemName = "ITEM275";
const std::string OPTIONS::ITEM276::itemName = "ITEM276";
const std::string OPTIONS::ITEM277::itemName = "ITEM277";
const std::string OPTIONS::ITEM278::itemName = "ITEM278";
const std::string OPTIONS::ITEM279::itemName = "ITEM279";
const std::string OPTIONS::ITEM280::itemName = "ITEM280";
const std::string OPTIONS::ITEM281::itemName = "ITEM281";
const std::string OPTIONS::ITEM282::itemName = "ITEM282";
const std::string OPTIONS::ITEM283::itemName = "ITEM283";
const std::string OPTIONS::ITEM284::itemName = "ITEM284";
const std::string OPTIONS::ITEM285::itemName = "ITEM285";
const std::string OPTIONS::ITEM286::itemName = "ITEM286";
const std::string OPTIONS::ITEM287::itemName = "ITEM287";
const std::string OPTIONS::ITEM288::itemName = "ITEM288";
const std::string OPTIONS::ITEM289::itemName = "ITEM289";
const std::string OPTIONS::ITEM290::itemName = "ITEM290";
const std::string OPTIONS::ITEM291::itemName = "ITEM291";
const std::string OPTIONS::ITEM292::itemName = "ITEM292";
const std::string OPTIONS::ITEM293::itemName = "ITEM293";
const std::string OPTIONS::ITEM294::itemName = "ITEM294";
const std::string OPTIONS::ITEM295::itemName = "ITEM295";
const std::string OPTIONS::ITEM296::itemName = "ITEM296";
const std::string OPTIONS::ITEM297::itemName = "ITEM297";
const std::string OPTIONS::ITEM298::itemName = "ITEM298";
const std::string OPTIONS::ITEM299::itemName = "ITEM299";
const std::string OPTIONS::ITEM300::itemName = "ITEM300";
const std::string OPTIONS::ITEM301::itemName = "ITEM301";
const std::string OPTIONS::ITEM302::itemName = "ITEM302";
const std::string OPTIONS::ITEM303::itemName = "ITEM303";
const std::string OPTIONS::ITEM304::itemName = "ITEM304";
const std::string OPTIONS::ITEM305::itemName = "ITEM305";
const std::string OPTIONS::ITEM306::itemName = "ITEM306";
const std::string OPTIONS::ITEM307::itemName = "ITEM307";
const std::string OPTIONS::ITEM308::itemName = "ITEM308";
const std::string OPTIONS::ITEM309::itemName = "ITEM309";
const std::string OPTIONS::ITEM310::itemName = "ITEM310";
const std::string OPTIONS::ITEM311::itemName = "ITEM311";
const std::string OPTIONS::ITEM312::itemName = "ITEM312";
const std::string OPTIONS::ITEM313::itemName = "ITEM313";
const std::string OPTIONS::ITEM314::itemName = "ITEM314";
const std::string OPTIONS::ITEM315::itemName = "ITEM315";
const std::string OPTIONS::ITEM316::itemName = "ITEM316";
const std::string OPTIONS::ITEM317::itemName = "ITEM317";
const std::string OPTIONS::ITEM318::itemName = "ITEM318";
const std::string OPTIONS::ITEM319::itemName = "ITEM319";


OPTIONS3::OPTIONS3() : ParserKeyword("OPTIONS3", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("OPTIONS3");
  {
     ParserRecord record;
     {
        ParserItem item("ITEM1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM3", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM4", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM5", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM6", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM7", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM8", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM9", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM10", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM11", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM12", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM13", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM14", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM15", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM16", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM17", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM18", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM19", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM20", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM21", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM22", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM23", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM24", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM25", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM26", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM27", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM28", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM29", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM30", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM31", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM32", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM33", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM34", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM35", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM36", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM37", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM38", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM39", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM40", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM41", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM42", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM43", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM44", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM45", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM46", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM47", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM48", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM49", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM50", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM51", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM52", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM53", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM54", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM55", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM56", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM57", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM58", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM59", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM60", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM61", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM62", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM63", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM64", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM65", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM66", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM67", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM68", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM69", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM70", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM71", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM72", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM73", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM74", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM75", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM76", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM77", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM78", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM79", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM80", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM81", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM82", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM83", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM84", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM85", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM86", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM87", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM88", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM89", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM90", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM91", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM92", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM93", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM94", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM95", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM96", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM97", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM98", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM99", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM100", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM101", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM102", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM103", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM104", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM105", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM106", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM107", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM108", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM109", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM110", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM111", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM112", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM113", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM114", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM115", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM116", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM117", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM118", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM119", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM120", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM121", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM122", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM123", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM124", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM125", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM126", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM127", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM128", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM129", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM130", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM131", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM132", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM133", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM134", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM135", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM136", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM137", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM138", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM139", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM140", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM141", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM142", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM143", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM144", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM145", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM146", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM147", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM148", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM149", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM150", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM151", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM152", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM153", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM154", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM155", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM156", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM157", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM158", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM159", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM160", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM161", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM162", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM163", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM164", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM165", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM166", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM167", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM168", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM169", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM170", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM171", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM172", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM173", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM174", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM175", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM176", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM177", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM178", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM179", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM180", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM181", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM182", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM183", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM184", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM185", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM186", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM187", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM188", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM189", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM190", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM191", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM192", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM193", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM194", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM195", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM196", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM197", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM198", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM199", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM200", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM201", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM202", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM203", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM204", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM205", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM206", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM207", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM208", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM209", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM210", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM211", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM212", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM213", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM214", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM215", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM216", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM217", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM218", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM219", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM220", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM221", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM222", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM223", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM224", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM225", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM226", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM227", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM228", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM229", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM230", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM231", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM232", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM233", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM234", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM235", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM236", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM237", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM238", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM239", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM240", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM241", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM242", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM243", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM244", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM245", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM246", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM247", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM248", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM249", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM250", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM251", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM252", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM253", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM254", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM255", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM256", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM257", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM258", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM259", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM260", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM261", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM262", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM263", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM264", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM265", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM266", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM267", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM268", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM269", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM270", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM271", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM272", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM273", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM274", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM275", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM276", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM277", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM278", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM279", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM280", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM281", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM282", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM283", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM284", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM285", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM286", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM287", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("ITEM288", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPTIONS3::keywordName = "OPTIONS3";
const std::string OPTIONS3::ITEM1::itemName = "ITEM1";
const std::string OPTIONS3::ITEM2::itemName = "ITEM2";
const std::string OPTIONS3::ITEM3::itemName = "ITEM3";
const std::string OPTIONS3::ITEM4::itemName = "ITEM4";
const std::string OPTIONS3::ITEM5::itemName = "ITEM5";
const std::string OPTIONS3::ITEM6::itemName = "ITEM6";
const std::string OPTIONS3::ITEM7::itemName = "ITEM7";
const std::string OPTIONS3::ITEM8::itemName = "ITEM8";
const std::string OPTIONS3::ITEM9::itemName = "ITEM9";
const std::string OPTIONS3::ITEM10::itemName = "ITEM10";
const std::string OPTIONS3::ITEM11::itemName = "ITEM11";
const std::string OPTIONS3::ITEM12::itemName = "ITEM12";
const std::string OPTIONS3::ITEM13::itemName = "ITEM13";
const std::string OPTIONS3::ITEM14::itemName = "ITEM14";
const std::string OPTIONS3::ITEM15::itemName = "ITEM15";
const std::string OPTIONS3::ITEM16::itemName = "ITEM16";
const std::string OPTIONS3::ITEM17::itemName = "ITEM17";
const std::string OPTIONS3::ITEM18::itemName = "ITEM18";
const std::string OPTIONS3::ITEM19::itemName = "ITEM19";
const std::string OPTIONS3::ITEM20::itemName = "ITEM20";
const std::string OPTIONS3::ITEM21::itemName = "ITEM21";
const std::string OPTIONS3::ITEM22::itemName = "ITEM22";
const std::string OPTIONS3::ITEM23::itemName = "ITEM23";
const std::string OPTIONS3::ITEM24::itemName = "ITEM24";
const std::string OPTIONS3::ITEM25::itemName = "ITEM25";
const std::string OPTIONS3::ITEM26::itemName = "ITEM26";
const std::string OPTIONS3::ITEM27::itemName = "ITEM27";
const std::string OPTIONS3::ITEM28::itemName = "ITEM28";
const std::string OPTIONS3::ITEM29::itemName = "ITEM29";
const std::string OPTIONS3::ITEM30::itemName = "ITEM30";
const std::string OPTIONS3::ITEM31::itemName = "ITEM31";
const std::string OPTIONS3::ITEM32::itemName = "ITEM32";
const std::string OPTIONS3::ITEM33::itemName = "ITEM33";
const std::string OPTIONS3::ITEM34::itemName = "ITEM34";
const std::string OPTIONS3::ITEM35::itemName = "ITEM35";
const std::string OPTIONS3::ITEM36::itemName = "ITEM36";
const std::string OPTIONS3::ITEM37::itemName = "ITEM37";
const std::string OPTIONS3::ITEM38::itemName = "ITEM38";
const std::string OPTIONS3::ITEM39::itemName = "ITEM39";
const std::string OPTIONS3::ITEM40::itemName = "ITEM40";
const std::string OPTIONS3::ITEM41::itemName = "ITEM41";
const std::string OPTIONS3::ITEM42::itemName = "ITEM42";
const std::string OPTIONS3::ITEM43::itemName = "ITEM43";
const std::string OPTIONS3::ITEM44::itemName = "ITEM44";
const std::string OPTIONS3::ITEM45::itemName = "ITEM45";
const std::string OPTIONS3::ITEM46::itemName = "ITEM46";
const std::string OPTIONS3::ITEM47::itemName = "ITEM47";
const std::string OPTIONS3::ITEM48::itemName = "ITEM48";
const std::string OPTIONS3::ITEM49::itemName = "ITEM49";
const std::string OPTIONS3::ITEM50::itemName = "ITEM50";
const std::string OPTIONS3::ITEM51::itemName = "ITEM51";
const std::string OPTIONS3::ITEM52::itemName = "ITEM52";
const std::string OPTIONS3::ITEM53::itemName = "ITEM53";
const std::string OPTIONS3::ITEM54::itemName = "ITEM54";
const std::string OPTIONS3::ITEM55::itemName = "ITEM55";
const std::string OPTIONS3::ITEM56::itemName = "ITEM56";
const std::string OPTIONS3::ITEM57::itemName = "ITEM57";
const std::string OPTIONS3::ITEM58::itemName = "ITEM58";
const std::string OPTIONS3::ITEM59::itemName = "ITEM59";
const std::string OPTIONS3::ITEM60::itemName = "ITEM60";
const std::string OPTIONS3::ITEM61::itemName = "ITEM61";
const std::string OPTIONS3::ITEM62::itemName = "ITEM62";
const std::string OPTIONS3::ITEM63::itemName = "ITEM63";
const std::string OPTIONS3::ITEM64::itemName = "ITEM64";
const std::string OPTIONS3::ITEM65::itemName = "ITEM65";
const std::string OPTIONS3::ITEM66::itemName = "ITEM66";
const std::string OPTIONS3::ITEM67::itemName = "ITEM67";
const std::string OPTIONS3::ITEM68::itemName = "ITEM68";
const std::string OPTIONS3::ITEM69::itemName = "ITEM69";
const std::string OPTIONS3::ITEM70::itemName = "ITEM70";
const std::string OPTIONS3::ITEM71::itemName = "ITEM71";
const std::string OPTIONS3::ITEM72::itemName = "ITEM72";
const std::string OPTIONS3::ITEM73::itemName = "ITEM73";
const std::string OPTIONS3::ITEM74::itemName = "ITEM74";
const std::string OPTIONS3::ITEM75::itemName = "ITEM75";
const std::string OPTIONS3::ITEM76::itemName = "ITEM76";
const std::string OPTIONS3::ITEM77::itemName = "ITEM77";
const std::string OPTIONS3::ITEM78::itemName = "ITEM78";
const std::string OPTIONS3::ITEM79::itemName = "ITEM79";
const std::string OPTIONS3::ITEM80::itemName = "ITEM80";
const std::string OPTIONS3::ITEM81::itemName = "ITEM81";
const std::string OPTIONS3::ITEM82::itemName = "ITEM82";
const std::string OPTIONS3::ITEM83::itemName = "ITEM83";
const std::string OPTIONS3::ITEM84::itemName = "ITEM84";
const std::string OPTIONS3::ITEM85::itemName = "ITEM85";
const std::string OPTIONS3::ITEM86::itemName = "ITEM86";
const std::string OPTIONS3::ITEM87::itemName = "ITEM87";
const std::string OPTIONS3::ITEM88::itemName = "ITEM88";
const std::string OPTIONS3::ITEM89::itemName = "ITEM89";
const std::string OPTIONS3::ITEM90::itemName = "ITEM90";
const std::string OPTIONS3::ITEM91::itemName = "ITEM91";
const std::string OPTIONS3::ITEM92::itemName = "ITEM92";
const std::string OPTIONS3::ITEM93::itemName = "ITEM93";
const std::string OPTIONS3::ITEM94::itemName = "ITEM94";
const std::string OPTIONS3::ITEM95::itemName = "ITEM95";
const std::string OPTIONS3::ITEM96::itemName = "ITEM96";
const std::string OPTIONS3::ITEM97::itemName = "ITEM97";
const std::string OPTIONS3::ITEM98::itemName = "ITEM98";
const std::string OPTIONS3::ITEM99::itemName = "ITEM99";
const std::string OPTIONS3::ITEM100::itemName = "ITEM100";
const std::string OPTIONS3::ITEM101::itemName = "ITEM101";
const std::string OPTIONS3::ITEM102::itemName = "ITEM102";
const std::string OPTIONS3::ITEM103::itemName = "ITEM103";
const std::string OPTIONS3::ITEM104::itemName = "ITEM104";
const std::string OPTIONS3::ITEM105::itemName = "ITEM105";
const std::string OPTIONS3::ITEM106::itemName = "ITEM106";
const std::string OPTIONS3::ITEM107::itemName = "ITEM107";
const std::string OPTIONS3::ITEM108::itemName = "ITEM108";
const std::string OPTIONS3::ITEM109::itemName = "ITEM109";
const std::string OPTIONS3::ITEM110::itemName = "ITEM110";
const std::string OPTIONS3::ITEM111::itemName = "ITEM111";
const std::string OPTIONS3::ITEM112::itemName = "ITEM112";
const std::string OPTIONS3::ITEM113::itemName = "ITEM113";
const std::string OPTIONS3::ITEM114::itemName = "ITEM114";
const std::string OPTIONS3::ITEM115::itemName = "ITEM115";
const std::string OPTIONS3::ITEM116::itemName = "ITEM116";
const std::string OPTIONS3::ITEM117::itemName = "ITEM117";
const std::string OPTIONS3::ITEM118::itemName = "ITEM118";
const std::string OPTIONS3::ITEM119::itemName = "ITEM119";
const std::string OPTIONS3::ITEM120::itemName = "ITEM120";
const std::string OPTIONS3::ITEM121::itemName = "ITEM121";
const std::string OPTIONS3::ITEM122::itemName = "ITEM122";
const std::string OPTIONS3::ITEM123::itemName = "ITEM123";
const std::string OPTIONS3::ITEM124::itemName = "ITEM124";
const std::string OPTIONS3::ITEM125::itemName = "ITEM125";
const std::string OPTIONS3::ITEM126::itemName = "ITEM126";
const std::string OPTIONS3::ITEM127::itemName = "ITEM127";
const std::string OPTIONS3::ITEM128::itemName = "ITEM128";
const std::string OPTIONS3::ITEM129::itemName = "ITEM129";
const std::string OPTIONS3::ITEM130::itemName = "ITEM130";
const std::string OPTIONS3::ITEM131::itemName = "ITEM131";
const std::string OPTIONS3::ITEM132::itemName = "ITEM132";
const std::string OPTIONS3::ITEM133::itemName = "ITEM133";
const std::string OPTIONS3::ITEM134::itemName = "ITEM134";
const std::string OPTIONS3::ITEM135::itemName = "ITEM135";
const std::string OPTIONS3::ITEM136::itemName = "ITEM136";
const std::string OPTIONS3::ITEM137::itemName = "ITEM137";
const std::string OPTIONS3::ITEM138::itemName = "ITEM138";
const std::string OPTIONS3::ITEM139::itemName = "ITEM139";
const std::string OPTIONS3::ITEM140::itemName = "ITEM140";
const std::string OPTIONS3::ITEM141::itemName = "ITEM141";
const std::string OPTIONS3::ITEM142::itemName = "ITEM142";
const std::string OPTIONS3::ITEM143::itemName = "ITEM143";
const std::string OPTIONS3::ITEM144::itemName = "ITEM144";
const std::string OPTIONS3::ITEM145::itemName = "ITEM145";
const std::string OPTIONS3::ITEM146::itemName = "ITEM146";
const std::string OPTIONS3::ITEM147::itemName = "ITEM147";
const std::string OPTIONS3::ITEM148::itemName = "ITEM148";
const std::string OPTIONS3::ITEM149::itemName = "ITEM149";
const std::string OPTIONS3::ITEM150::itemName = "ITEM150";
const std::string OPTIONS3::ITEM151::itemName = "ITEM151";
const std::string OPTIONS3::ITEM152::itemName = "ITEM152";
const std::string OPTIONS3::ITEM153::itemName = "ITEM153";
const std::string OPTIONS3::ITEM154::itemName = "ITEM154";
const std::string OPTIONS3::ITEM155::itemName = "ITEM155";
const std::string OPTIONS3::ITEM156::itemName = "ITEM156";
const std::string OPTIONS3::ITEM157::itemName = "ITEM157";
const std::string OPTIONS3::ITEM158::itemName = "ITEM158";
const std::string OPTIONS3::ITEM159::itemName = "ITEM159";
const std::string OPTIONS3::ITEM160::itemName = "ITEM160";
const std::string OPTIONS3::ITEM161::itemName = "ITEM161";
const std::string OPTIONS3::ITEM162::itemName = "ITEM162";
const std::string OPTIONS3::ITEM163::itemName = "ITEM163";
const std::string OPTIONS3::ITEM164::itemName = "ITEM164";
const std::string OPTIONS3::ITEM165::itemName = "ITEM165";
const std::string OPTIONS3::ITEM166::itemName = "ITEM166";
const std::string OPTIONS3::ITEM167::itemName = "ITEM167";
const std::string OPTIONS3::ITEM168::itemName = "ITEM168";
const std::string OPTIONS3::ITEM169::itemName = "ITEM169";
const std::string OPTIONS3::ITEM170::itemName = "ITEM170";
const std::string OPTIONS3::ITEM171::itemName = "ITEM171";
const std::string OPTIONS3::ITEM172::itemName = "ITEM172";
const std::string OPTIONS3::ITEM173::itemName = "ITEM173";
const std::string OPTIONS3::ITEM174::itemName = "ITEM174";
const std::string OPTIONS3::ITEM175::itemName = "ITEM175";
const std::string OPTIONS3::ITEM176::itemName = "ITEM176";
const std::string OPTIONS3::ITEM177::itemName = "ITEM177";
const std::string OPTIONS3::ITEM178::itemName = "ITEM178";
const std::string OPTIONS3::ITEM179::itemName = "ITEM179";
const std::string OPTIONS3::ITEM180::itemName = "ITEM180";
const std::string OPTIONS3::ITEM181::itemName = "ITEM181";
const std::string OPTIONS3::ITEM182::itemName = "ITEM182";
const std::string OPTIONS3::ITEM183::itemName = "ITEM183";
const std::string OPTIONS3::ITEM184::itemName = "ITEM184";
const std::string OPTIONS3::ITEM185::itemName = "ITEM185";
const std::string OPTIONS3::ITEM186::itemName = "ITEM186";
const std::string OPTIONS3::ITEM187::itemName = "ITEM187";
const std::string OPTIONS3::ITEM188::itemName = "ITEM188";
const std::string OPTIONS3::ITEM189::itemName = "ITEM189";
const std::string OPTIONS3::ITEM190::itemName = "ITEM190";
const std::string OPTIONS3::ITEM191::itemName = "ITEM191";
const std::string OPTIONS3::ITEM192::itemName = "ITEM192";
const std::string OPTIONS3::ITEM193::itemName = "ITEM193";
const std::string OPTIONS3::ITEM194::itemName = "ITEM194";
const std::string OPTIONS3::ITEM195::itemName = "ITEM195";
const std::string OPTIONS3::ITEM196::itemName = "ITEM196";
const std::string OPTIONS3::ITEM197::itemName = "ITEM197";
const std::string OPTIONS3::ITEM198::itemName = "ITEM198";
const std::string OPTIONS3::ITEM199::itemName = "ITEM199";
const std::string OPTIONS3::ITEM200::itemName = "ITEM200";
const std::string OPTIONS3::ITEM201::itemName = "ITEM201";
const std::string OPTIONS3::ITEM202::itemName = "ITEM202";
const std::string OPTIONS3::ITEM203::itemName = "ITEM203";
const std::string OPTIONS3::ITEM204::itemName = "ITEM204";
const std::string OPTIONS3::ITEM205::itemName = "ITEM205";
const std::string OPTIONS3::ITEM206::itemName = "ITEM206";
const std::string OPTIONS3::ITEM207::itemName = "ITEM207";
const std::string OPTIONS3::ITEM208::itemName = "ITEM208";
const std::string OPTIONS3::ITEM209::itemName = "ITEM209";
const std::string OPTIONS3::ITEM210::itemName = "ITEM210";
const std::string OPTIONS3::ITEM211::itemName = "ITEM211";
const std::string OPTIONS3::ITEM212::itemName = "ITEM212";
const std::string OPTIONS3::ITEM213::itemName = "ITEM213";
const std::string OPTIONS3::ITEM214::itemName = "ITEM214";
const std::string OPTIONS3::ITEM215::itemName = "ITEM215";
const std::string OPTIONS3::ITEM216::itemName = "ITEM216";
const std::string OPTIONS3::ITEM217::itemName = "ITEM217";
const std::string OPTIONS3::ITEM218::itemName = "ITEM218";
const std::string OPTIONS3::ITEM219::itemName = "ITEM219";
const std::string OPTIONS3::ITEM220::itemName = "ITEM220";
const std::string OPTIONS3::ITEM221::itemName = "ITEM221";
const std::string OPTIONS3::ITEM222::itemName = "ITEM222";
const std::string OPTIONS3::ITEM223::itemName = "ITEM223";
const std::string OPTIONS3::ITEM224::itemName = "ITEM224";
const std::string OPTIONS3::ITEM225::itemName = "ITEM225";
const std::string OPTIONS3::ITEM226::itemName = "ITEM226";
const std::string OPTIONS3::ITEM227::itemName = "ITEM227";
const std::string OPTIONS3::ITEM228::itemName = "ITEM228";
const std::string OPTIONS3::ITEM229::itemName = "ITEM229";
const std::string OPTIONS3::ITEM230::itemName = "ITEM230";
const std::string OPTIONS3::ITEM231::itemName = "ITEM231";
const std::string OPTIONS3::ITEM232::itemName = "ITEM232";
const std::string OPTIONS3::ITEM233::itemName = "ITEM233";
const std::string OPTIONS3::ITEM234::itemName = "ITEM234";
const std::string OPTIONS3::ITEM235::itemName = "ITEM235";
const std::string OPTIONS3::ITEM236::itemName = "ITEM236";
const std::string OPTIONS3::ITEM237::itemName = "ITEM237";
const std::string OPTIONS3::ITEM238::itemName = "ITEM238";
const std::string OPTIONS3::ITEM239::itemName = "ITEM239";
const std::string OPTIONS3::ITEM240::itemName = "ITEM240";
const std::string OPTIONS3::ITEM241::itemName = "ITEM241";
const std::string OPTIONS3::ITEM242::itemName = "ITEM242";
const std::string OPTIONS3::ITEM243::itemName = "ITEM243";
const std::string OPTIONS3::ITEM244::itemName = "ITEM244";
const std::string OPTIONS3::ITEM245::itemName = "ITEM245";
const std::string OPTIONS3::ITEM246::itemName = "ITEM246";
const std::string OPTIONS3::ITEM247::itemName = "ITEM247";
const std::string OPTIONS3::ITEM248::itemName = "ITEM248";
const std::string OPTIONS3::ITEM249::itemName = "ITEM249";
const std::string OPTIONS3::ITEM250::itemName = "ITEM250";
const std::string OPTIONS3::ITEM251::itemName = "ITEM251";
const std::string OPTIONS3::ITEM252::itemName = "ITEM252";
const std::string OPTIONS3::ITEM253::itemName = "ITEM253";
const std::string OPTIONS3::ITEM254::itemName = "ITEM254";
const std::string OPTIONS3::ITEM255::itemName = "ITEM255";
const std::string OPTIONS3::ITEM256::itemName = "ITEM256";
const std::string OPTIONS3::ITEM257::itemName = "ITEM257";
const std::string OPTIONS3::ITEM258::itemName = "ITEM258";
const std::string OPTIONS3::ITEM259::itemName = "ITEM259";
const std::string OPTIONS3::ITEM260::itemName = "ITEM260";
const std::string OPTIONS3::ITEM261::itemName = "ITEM261";
const std::string OPTIONS3::ITEM262::itemName = "ITEM262";
const std::string OPTIONS3::ITEM263::itemName = "ITEM263";
const std::string OPTIONS3::ITEM264::itemName = "ITEM264";
const std::string OPTIONS3::ITEM265::itemName = "ITEM265";
const std::string OPTIONS3::ITEM266::itemName = "ITEM266";
const std::string OPTIONS3::ITEM267::itemName = "ITEM267";
const std::string OPTIONS3::ITEM268::itemName = "ITEM268";
const std::string OPTIONS3::ITEM269::itemName = "ITEM269";
const std::string OPTIONS3::ITEM270::itemName = "ITEM270";
const std::string OPTIONS3::ITEM271::itemName = "ITEM271";
const std::string OPTIONS3::ITEM272::itemName = "ITEM272";
const std::string OPTIONS3::ITEM273::itemName = "ITEM273";
const std::string OPTIONS3::ITEM274::itemName = "ITEM274";
const std::string OPTIONS3::ITEM275::itemName = "ITEM275";
const std::string OPTIONS3::ITEM276::itemName = "ITEM276";
const std::string OPTIONS3::ITEM277::itemName = "ITEM277";
const std::string OPTIONS3::ITEM278::itemName = "ITEM278";
const std::string OPTIONS3::ITEM279::itemName = "ITEM279";
const std::string OPTIONS3::ITEM280::itemName = "ITEM280";
const std::string OPTIONS3::ITEM281::itemName = "ITEM281";
const std::string OPTIONS3::ITEM282::itemName = "ITEM282";
const std::string OPTIONS3::ITEM283::itemName = "ITEM283";
const std::string OPTIONS3::ITEM284::itemName = "ITEM284";
const std::string OPTIONS3::ITEM285::itemName = "ITEM285";
const std::string OPTIONS3::ITEM286::itemName = "ITEM286";
const std::string OPTIONS3::ITEM287::itemName = "ITEM287";
const std::string OPTIONS3::ITEM288::itemName = "ITEM288";


OUTRAD::OUTRAD() : ParserKeyword("OUTRAD", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("OUTRAD");
  {
     ParserRecord record;
     {
        ParserItem item("RADIUS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OUTRAD::keywordName = "OUTRAD";
const std::string OUTRAD::RADIUS::itemName = "RADIUS";


OUTSOL::OUTSOL() : ParserKeyword("OUTSOL", KeywordSize(0, false)) {
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("OUTSOL");
}
const std::string OUTSOL::keywordName = "OUTSOL";


OVERBURD::OVERBURD() : ParserKeyword("OVERBURD", KeywordSize("ROCKCOMP", "NTROCC", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OVERBURD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string OVERBURD::keywordName = "OVERBURD";
const std::string OVERBURD::DATA::itemName = "DATA";


}
}
