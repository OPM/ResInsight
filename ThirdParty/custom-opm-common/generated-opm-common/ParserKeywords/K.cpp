#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/K.hpp>
namespace Opm {
namespace ParserKeywords {
KRNUM::KRNUM( ) : ParserKeyword("KRNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("KRNUM");
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


KRNUMMF::KRNUMMF( ) : ParserKeyword("KRNUMMF")
{
  setFixedSize( (size_t) 1);
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
