#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/U.hpp>
namespace Opm {
namespace ParserKeywords {
UDADIMS::UDADIMS( ) : ParserKeyword("UDADIMS")
{
  setFixedSize( (size_t) 1);
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
const int UDADIMS::NUM_UDQ_REPLACE::defaultValue = 0;
const std::string UDADIMS::IGNORED::itemName = "IGNORED";
const int UDADIMS::IGNORED::defaultValue = 0;
const std::string UDADIMS::TOTAL_UDQ_UNIQUE::itemName = "TOTAL_UDQ_UNIQUE";
const int UDADIMS::TOTAL_UDQ_UNIQUE::defaultValue = 100;


UDQ::UDQ( ) : ParserKeyword("UDQ")
{
  setSizeType(SLASH_TERMINATED);
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


UDQDIMS::UDQDIMS( ) : ParserKeyword("UDQDIMS")
{
  setFixedSize( (size_t) 1);
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
const int UDQDIMS::MAX_FUNCTIONS::defaultValue = 16;
const std::string UDQDIMS::MAX_ITEMS::itemName = "MAX_ITEMS";
const int UDQDIMS::MAX_ITEMS::defaultValue = 16;
const std::string UDQDIMS::MAX_CONNECTIONS::itemName = "MAX_CONNECTIONS";
const int UDQDIMS::MAX_CONNECTIONS::defaultValue = 0;
const std::string UDQDIMS::MAX_FIELDS::itemName = "MAX_FIELDS";
const int UDQDIMS::MAX_FIELDS::defaultValue = 0;
const std::string UDQDIMS::MAX_GROUP::itemName = "MAX_GROUP";
const int UDQDIMS::MAX_GROUP::defaultValue = 0;
const std::string UDQDIMS::MAX_REGION::itemName = "MAX_REGION";
const int UDQDIMS::MAX_REGION::defaultValue = 0;
const std::string UDQDIMS::MAX_SEGMENT::itemName = "MAX_SEGMENT";
const int UDQDIMS::MAX_SEGMENT::defaultValue = 0;
const std::string UDQDIMS::MAX_WELL::itemName = "MAX_WELL";
const int UDQDIMS::MAX_WELL::defaultValue = 0;
const std::string UDQDIMS::MAX_AQUIFER::itemName = "MAX_AQUIFER";
const int UDQDIMS::MAX_AQUIFER::defaultValue = 0;
const std::string UDQDIMS::MAX_BLOCK::itemName = "MAX_BLOCK";
const int UDQDIMS::MAX_BLOCK::defaultValue = 0;
const std::string UDQDIMS::RESTART_NEW_SEED::itemName = "RESTART_NEW_SEED";
const std::string UDQDIMS::RESTART_NEW_SEED::defaultValue = "N";


UDQPARAM::UDQPARAM( ) : ParserKeyword("UDQPARAM")
{
  setFixedSize( (size_t) 1);
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
const int UDQPARAM::RANDOM_SEED::defaultValue = 1;
const std::string UDQPARAM::RANGE::itemName = "RANGE";
const double UDQPARAM::RANGE::defaultValue = 100000000000000000000.000000;
const std::string UDQPARAM::UNDEFINED_VALUE::itemName = "UNDEFINED_VALUE";
const double UDQPARAM::UNDEFINED_VALUE::defaultValue = 0.000000;
const std::string UDQPARAM::CMP_EPSILON::itemName = "CMP_EPSILON";
const double UDQPARAM::CMP_EPSILON::defaultValue = 0.000100;


UDT::UDT( ) : ParserKeyword("UDT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("UDT");
}
const std::string UDT::keywordName = "UDT";


UDTDIMS::UDTDIMS( ) : ParserKeyword("UDTDIMS")
{
  setFixedSize( (size_t) 1);
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
const int UDTDIMS::MAX_TABLES::defaultValue = 0;
const std::string UDTDIMS::MAX_ROWS::itemName = "MAX_ROWS";
const int UDTDIMS::MAX_ROWS::defaultValue = 0;
const std::string UDTDIMS::MAX_INTERPOLATION_POINTS::itemName = "MAX_INTERPOLATION_POINTS";
const int UDTDIMS::MAX_INTERPOLATION_POINTS::defaultValue = 0;
const std::string UDTDIMS::MAX_DIMENSIONS::itemName = "MAX_DIMENSIONS";
const int UDTDIMS::MAX_DIMENSIONS::defaultValue = 0;


UNCODHMD::UNCODHMD( ) : ParserKeyword("UNCODHMD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNCODHMD");
}
const std::string UNCODHMD::keywordName = "UNCODHMD";


UNIFIN::UNIFIN( ) : ParserKeyword("UNIFIN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFIN");
}
const std::string UNIFIN::keywordName = "UNIFIN";


UNIFOUT::UNIFOUT( ) : ParserKeyword("UNIFOUT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFOUT");
}
const std::string UNIFOUT::keywordName = "UNIFOUT";


UNIFOUTS::UNIFOUTS( ) : ParserKeyword("UNIFOUTS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFOUTS");
}
const std::string UNIFOUTS::keywordName = "UNIFOUTS";


UNIFSAVE::UNIFSAVE( ) : ParserKeyword("UNIFSAVE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFSAVE");
}
const std::string UNIFSAVE::keywordName = "UNIFSAVE";


USECUPL::USECUPL( ) : ParserKeyword("USECUPL")
{
  setSizeType(SLASH_TERMINATED);
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


USEFLUX::USEFLUX( ) : ParserKeyword("USEFLUX")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("USEFLUX");
}
const std::string USEFLUX::keywordName = "USEFLUX";


USENOFLO::USENOFLO( ) : ParserKeyword("USENOFLO")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("USENOFLO");
}
const std::string USENOFLO::keywordName = "USENOFLO";


}
}
