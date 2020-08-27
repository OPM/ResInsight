#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/Z.hpp>
namespace Opm {
namespace ParserKeywords {
ZCORN::ZCORN( ) : ParserKeyword("ZCORN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
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


ZFACT1::ZFACT1( ) : ParserKeyword("ZFACT1")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


ZFACT1S::ZFACT1S( ) : ParserKeyword("ZFACT1S")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


ZFACTOR::ZFACTOR( ) : ParserKeyword("ZFACTOR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


ZFACTORS::ZFACTORS( ) : ParserKeyword("ZFACTORS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
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


ZIPP2OFF::ZIPP2OFF( ) : ParserKeyword("ZIPP2OFF")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ZIPP2OFF");
}
const std::string ZIPP2OFF::keywordName = "ZIPP2OFF";


ZIPPY2::ZIPPY2( ) : ParserKeyword("ZIPPY2")
{
  setFixedSize( (size_t) 1);
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


}
}
