
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/Y.hpp>
namespace Opm {
namespace ParserKeywords {
YMF::YMF() : ParserKeyword("YMF", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("YMF");
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
const std::string YMF::keywordName = "YMF";
const std::string YMF::data::itemName = "data";


YMODULE::YMODULE() : ParserKeyword("YMODULE", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("YMODULE");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Ymodule");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string YMODULE::keywordName = "YMODULE";
const std::string YMODULE::data::itemName = "data";


}
}
