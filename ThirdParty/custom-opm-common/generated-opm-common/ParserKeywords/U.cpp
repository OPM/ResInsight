
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/U.hpp>
namespace Opm {
namespace ParserKeywords {
UDADIMS::UDADIMS() : ParserKeyword("UDADIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDADIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_UDQ_REPLACE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("IGNORED", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("TOTAL_UDQ_UNIQUE", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDADIMS::keywordName = "UDADIMS";
const std::string UDADIMS::NUM_UDQ_REPLACE::itemName = "NUM_UDQ_REPLACE";
const std::string UDADIMS::IGNORED::itemName = "IGNORED";
const std::string UDADIMS::TOTAL_UDQ_UNIQUE::itemName = "TOTAL_UDQ_UNIQUE";


UDQ::UDQ() : ParserKeyword("UDQ", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("UDQ");
  {
     ParserRecord record;
     {
        ParserItem item("ACTION", ParserItem::itype::RAW_STRING);
        record.addItem(item);
     }
     {
        ParserItem item("QUANTITY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::RAW_STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDQ::keywordName = "UDQ";
const std::string UDQ::ACTION::itemName = "ACTION";
const std::string UDQ::QUANTITY::itemName = "QUANTITY";
const std::string UDQ::DATA::itemName = "DATA";


UDQDIMS::UDQDIMS() : ParserKeyword("UDQDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDQDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_FUNCTIONS", ParserItem::itype::INT);
        item.setDefault( 16 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ITEMS", ParserItem::itype::INT);
        item.setDefault( 16 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_CONNECTIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FIELDS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GROUP", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_REGION", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SEGMENT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WELL", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_AQUIFER", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_BLOCK", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("RESTART_NEW_SEED", ParserItem::itype::STRING);
        item.setDefault( std::string("N") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDQDIMS::keywordName = "UDQDIMS";
const std::string UDQDIMS::MAX_FUNCTIONS::itemName = "MAX_FUNCTIONS";
const std::string UDQDIMS::MAX_ITEMS::itemName = "MAX_ITEMS";
const std::string UDQDIMS::MAX_CONNECTIONS::itemName = "MAX_CONNECTIONS";
const std::string UDQDIMS::MAX_FIELDS::itemName = "MAX_FIELDS";
const std::string UDQDIMS::MAX_GROUP::itemName = "MAX_GROUP";
const std::string UDQDIMS::MAX_REGION::itemName = "MAX_REGION";
const std::string UDQDIMS::MAX_SEGMENT::itemName = "MAX_SEGMENT";
const std::string UDQDIMS::MAX_WELL::itemName = "MAX_WELL";
const std::string UDQDIMS::MAX_AQUIFER::itemName = "MAX_AQUIFER";
const std::string UDQDIMS::MAX_BLOCK::itemName = "MAX_BLOCK";
const std::string UDQDIMS::RESTART_NEW_SEED::itemName = "RESTART_NEW_SEED";
const std::string UDQDIMS::RESTART_NEW_SEED::defaultValue = "N";


UDQPARAM::UDQPARAM() : ParserKeyword("UDQPARAM", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDQPARAM");
  {
     ParserRecord record;
     {
        ParserItem item("RANDOM_SEED", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("RANGE", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("UNDEFINED_VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("CMP_EPSILON", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.000100) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDQPARAM::keywordName = "UDQPARAM";
const std::string UDQPARAM::RANDOM_SEED::itemName = "RANDOM_SEED";
const std::string UDQPARAM::RANGE::itemName = "RANGE";
const std::string UDQPARAM::UNDEFINED_VALUE::itemName = "UNDEFINED_VALUE";
const std::string UDQPARAM::CMP_EPSILON::itemName = "CMP_EPSILON";


UDT::UDT() : ParserKeyword("UDT", KeywordSize(DOUBLE_SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("UDT");
  setDoubleRecordsKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIMENSIONS", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("INTERPOLATION_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INTERPOLATION_POINTS", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_VALUES", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDT::keywordName = "UDT";
const std::string UDT::TABLE_NAME::itemName = "TABLE_NAME";
const std::string UDT::DIMENSIONS::itemName = "DIMENSIONS";
const std::string UDT::INTERPOLATION_TYPE::itemName = "INTERPOLATION_TYPE";
const std::string UDT::INTERPOLATION_POINTS::itemName = "INTERPOLATION_POINTS";
const std::string UDT::TABLE_VALUES::itemName = "TABLE_VALUES";


UDTDIMS::UDTDIMS() : ParserKeyword("UDTDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDTDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_TABLES", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ROWS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_INTERPOLATION_POINTS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_DIMENSIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string UDTDIMS::keywordName = "UDTDIMS";
const std::string UDTDIMS::MAX_TABLES::itemName = "MAX_TABLES";
const std::string UDTDIMS::MAX_ROWS::itemName = "MAX_ROWS";
const std::string UDTDIMS::MAX_INTERPOLATION_POINTS::itemName = "MAX_INTERPOLATION_POINTS";
const std::string UDTDIMS::MAX_DIMENSIONS::itemName = "MAX_DIMENSIONS";


UNCODHMD::UNCODHMD() : ParserKeyword("UNCODHMD", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNCODHMD");
}
const std::string UNCODHMD::keywordName = "UNCODHMD";


UNIFIN::UNIFIN() : ParserKeyword("UNIFIN", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFIN");
}
const std::string UNIFIN::keywordName = "UNIFIN";


UNIFOUT::UNIFOUT() : ParserKeyword("UNIFOUT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFOUT");
}
const std::string UNIFOUT::keywordName = "UNIFOUT";


UNIFOUTS::UNIFOUTS() : ParserKeyword("UNIFOUTS", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFOUTS");
}
const std::string UNIFOUTS::keywordName = "UNIFOUTS";


UNIFSAVE::UNIFSAVE() : ParserKeyword("UNIFSAVE", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFSAVE");
}
const std::string UNIFSAVE::keywordName = "UNIFSAVE";


USECUPL::USECUPL() : ParserKeyword("USECUPL", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("USECUPL");
  {
     ParserRecord record;
     {
        ParserItem item("BASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FMT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string USECUPL::keywordName = "USECUPL";
const std::string USECUPL::BASE::itemName = "BASE";
const std::string USECUPL::FMT::itemName = "FMT";


USEFLUX::USEFLUX() : ParserKeyword("USEFLUX", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("USEFLUX");
}
const std::string USEFLUX::keywordName = "USEFLUX";


USENOFLO::USENOFLO() : ParserKeyword("USENOFLO", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("USENOFLO");
}
const std::string USENOFLO::keywordName = "USENOFLO";


}
}
