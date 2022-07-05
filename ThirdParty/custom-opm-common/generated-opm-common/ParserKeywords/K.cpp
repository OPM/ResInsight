
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/K.hpp>
namespace Opm {
namespace ParserKeywords {
KRNUM::KRNUM() : ParserKeyword("KRNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("KRNUMX");
  addDeckName("KRNUMX-");
  addDeckName("KRNUMY");
  addDeckName("KRNUMY-");
  addDeckName("KRNUMR");
  addDeckName("KRNUMZ");
  addDeckName("KRNUMR-");
  addDeckName("KRNUMZ-");
  addDeckName("KRNUMT");
  addDeckName("KRNUMT-");
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
const std::string KRNUM::keywordName = "KRNUM";
const std::string KRNUM::data::itemName = "data";


KRNUMMF::KRNUMMF() : ParserKeyword("KRNUMMF", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("KRNUMMF");
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
const std::string KRNUMMF::keywordName = "KRNUMMF";
const std::string KRNUMMF::data::itemName = "data";


}
}
