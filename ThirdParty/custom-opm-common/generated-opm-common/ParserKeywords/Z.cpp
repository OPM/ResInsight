
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/Z.hpp>
namespace Opm {
namespace ParserKeywords {
ZCORN::ZCORN() : ParserKeyword("ZCORN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  setProhibitedKeywords({
    "GDFILE",
  });
  clearDeckNames();
  addDeckName("ZCORN");
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
const std::string ZCORN::keywordName = "ZCORN";
const std::string ZCORN::data::itemName = "data";


ZFACT1::ZFACT1() : ParserKeyword("ZFACT1", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACT1");
  {
     ParserRecord record;
     {
        ParserItem item("Z0", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACT1::keywordName = "ZFACT1";
const std::string ZFACT1::Z0::itemName = "Z0";


ZFACT1S::ZFACT1S() : ParserKeyword("ZFACT1S", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACT1S");
  {
     ParserRecord record;
     {
        ParserItem item("Z0", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACT1S::keywordName = "ZFACT1S";
const std::string ZFACT1S::Z0::itemName = "Z0";


ZFACTOR::ZFACTOR() : ParserKeyword("ZFACTOR", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACTOR");
  {
     ParserRecord record;
     {
        ParserItem item("Z0", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACTOR::keywordName = "ZFACTOR";
const std::string ZFACTOR::Z0::itemName = "Z0";


ZFACTORS::ZFACTORS() : ParserKeyword("ZFACTORS", KeywordSize("TABDIMS", "NUM_EOS_SURFACE", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACTORS");
  {
     ParserRecord record;
     {
        ParserItem item("Z0", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACTORS::keywordName = "ZFACTORS";
const std::string ZFACTORS::Z0::itemName = "Z0";


ZIPP2OFF::ZIPP2OFF() : ParserKeyword("ZIPP2OFF", KeywordSize(0, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ZIPP2OFF");
}
const std::string ZIPP2OFF::keywordName = "ZIPP2OFF";


ZIPPY2::ZIPPY2() : ParserKeyword("ZIPPY2", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ZIPPY2");
  {
     ParserRecord record;
     {
        ParserItem item("SETTINGS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZIPPY2::keywordName = "ZIPPY2";
const std::string ZIPPY2::SETTINGS::itemName = "SETTINGS";


ZMF::ZMF() : ParserKeyword("ZMF", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ZMF");
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
const std::string ZMF::keywordName = "ZMF";
const std::string ZMF::data::itemName = "data";


ZMFVD::ZMFVD() : ParserKeyword("ZMFVD", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZMFVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZMFVD::keywordName = "ZMFVD";
const std::string ZMFVD::DATA::itemName = "DATA";


}
}
