#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/Q.hpp>
namespace Opm {
namespace ParserKeywords {
QDRILL::QDRILL( ) : ParserKeyword("QDRILL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("QDRILL");
  {
     ParserRecord record;
     {
        ParserItem item("WELL_NAME", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string QDRILL::keywordName = "QDRILL";
const std::string QDRILL::WELL_NAME::itemName = "WELL_NAME";


QHRATING::QHRATING( ) : ParserKeyword("QHRATING")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("RIVRDIMS","NRATTA",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("QHRATING");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length*Length*Length/Time");
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string QHRATING::keywordName = "QHRATING";
const std::string QHRATING::DATA::itemName = "DATA";


QMOBIL::QMOBIL( ) : ParserKeyword("QMOBIL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("QMOBIL");
  {
     ParserRecord record;
     {
        ParserItem item("MOBILE_END_POINT_CORRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string QMOBIL::keywordName = "QMOBIL";
const std::string QMOBIL::MOBILE_END_POINT_CORRECTION::itemName = "MOBILE_END_POINT_CORRECTION";


}
}
