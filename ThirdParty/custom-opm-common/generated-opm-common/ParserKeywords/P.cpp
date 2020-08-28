#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
namespace Opm {
namespace ParserKeywords {
PARALLEL::PARALLEL( ) : ParserKeyword("PARALLEL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PARALLEL");
  {
     ParserRecord record;
     {
        ParserItem item("NDMAIN", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MACHINE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("DISTRIBUTED") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PARALLEL::keywordName = "PARALLEL";
const std::string PARALLEL::NDMAIN::itemName = "NDMAIN";
const int PARALLEL::NDMAIN::defaultValue = 1;
const std::string PARALLEL::MACHINE_TYPE::itemName = "MACHINE_TYPE";
const std::string PARALLEL::MACHINE_TYPE::defaultValue = "DISTRIBUTED";


PARAOPTS::PARAOPTS( ) : ParserKeyword("PARAOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PARAOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("METHOD", ParserItem::itype::STRING);
        item.setDefault( std::string("TREE") );
        record.addItem(item);
     }
     {
        ParserItem item("SET_PRINT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SIZE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NUM_BUFFERS", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("VALUE_MEM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VALUE_COARSE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VALUE_NNC", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("VALUE_PRT_FILE", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("RESERVED", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PARAOPTS::keywordName = "PARAOPTS";
const std::string PARAOPTS::METHOD::itemName = "METHOD";
const std::string PARAOPTS::METHOD::defaultValue = "TREE";
const std::string PARAOPTS::SET_PRINT::itemName = "SET_PRINT";
const int PARAOPTS::SET_PRINT::defaultValue = 0;
const std::string PARAOPTS::SIZE::itemName = "SIZE";
const int PARAOPTS::SIZE::defaultValue = 0;
const std::string PARAOPTS::NUM_BUFFERS::itemName = "NUM_BUFFERS";
const int PARAOPTS::NUM_BUFFERS::defaultValue = 2;
const std::string PARAOPTS::VALUE_MEM::itemName = "VALUE_MEM";
const int PARAOPTS::VALUE_MEM::defaultValue = 0;
const std::string PARAOPTS::VALUE_COARSE::itemName = "VALUE_COARSE";
const int PARAOPTS::VALUE_COARSE::defaultValue = 0;
const std::string PARAOPTS::VALUE_NNC::itemName = "VALUE_NNC";
const int PARAOPTS::VALUE_NNC::defaultValue = 0;
const std::string PARAOPTS::VALUE_PRT_FILE::itemName = "VALUE_PRT_FILE";
const int PARAOPTS::VALUE_PRT_FILE::defaultValue = 1;
const std::string PARAOPTS::RESERVED::itemName = "RESERVED";


PARTTRAC::PARTTRAC( ) : ParserKeyword("PARTTRAC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PARTTRAC");
  {
     ParserRecord record;
     {
        ParserItem item("NPARTT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NKPTMX", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NPKPMX", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PARTTRAC::keywordName = "PARTTRAC";
const std::string PARTTRAC::NPARTT::itemName = "NPARTT";
const int PARTTRAC::NPARTT::defaultValue = 0;
const std::string PARTTRAC::NKPTMX::itemName = "NKPTMX";
const int PARTTRAC::NKPTMX::defaultValue = 0;
const std::string PARTTRAC::NPKPMX::itemName = "NPKPMX";
const int PARTTRAC::NPKPMX::defaultValue = 0;


PATHS::PATHS( ) : ParserKeyword("PATHS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PATHS");
  {
     ParserRecord record;
     {
        ParserItem item("PathName", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PathValue", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PATHS::keywordName = "PATHS";
const std::string PATHS::PathName::itemName = "PathName";
const std::string PATHS::PathValue::itemName = "PathValue";


PBUB::PBUB( ) : ParserKeyword("PBUB")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PBUB");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PBUB::keywordName = "PBUB";
const std::string PBUB::data::itemName = "data";


PBVD::PBVD( ) : ParserKeyword("PBVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PBVD");
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
const std::string PBVD::keywordName = "PBVD";
const std::string PBVD::DATA::itemName = "DATA";


PCG::PCG( ) : ParserKeyword("PCG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PCG");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PCG::keywordName = "PCG";
const std::string PCG::data::itemName = "data";


PCG32D::PCG32D( ) : ParserKeyword("PCG32D")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PCG32D");
  {
     ParserRecord record;
     {
        ParserItem item("SOME_DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PCG32D::keywordName = "PCG32D";
const std::string PCG32D::SOME_DATA::itemName = "SOME_DATA";


PCW::PCW( ) : ParserKeyword("PCW")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PCW");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PCW::keywordName = "PCW";
const std::string PCW::data::itemName = "data";


PCW32D::PCW32D( ) : ParserKeyword("PCW32D")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PCW32D");
  {
     ParserRecord record;
     {
        ParserItem item("SOME_DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PCW32D::keywordName = "PCW32D";
const std::string PCW32D::SOME_DATA::itemName = "SOME_DATA";


PDEW::PDEW( ) : ParserKeyword("PDEW")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PDEW");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PDEW::keywordName = "PDEW";
const std::string PDEW::data::itemName = "data";


PDVD::PDVD( ) : ParserKeyword("PDVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PDVD");
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
const std::string PDVD::keywordName = "PDVD";
const std::string PDVD::DATA::itemName = "DATA";


PEBI::PEBI( ) : ParserKeyword("PEBI")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PEBI");
  {
     ParserRecord record;
     {
        ParserItem item("NEG_TRANSMISSIBILITIES", ParserItem::itype::STRING);
        item.setDefault( std::string("No") );
        record.addItem(item);
     }
     {
        ParserItem item("AVOID_GRID_CALC", ParserItem::itype::STRING);
        item.setDefault( std::string("No") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PEBI::keywordName = "PEBI";
const std::string PEBI::NEG_TRANSMISSIBILITIES::itemName = "NEG_TRANSMISSIBILITIES";
const std::string PEBI::NEG_TRANSMISSIBILITIES::defaultValue = "No";
const std::string PEBI::AVOID_GRID_CALC::itemName = "AVOID_GRID_CALC";
const std::string PEBI::AVOID_GRID_CALC::defaultValue = "No";


PECOEFS::PECOEFS( ) : ParserKeyword("PECOEFS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PECOEFS");
  {
     ParserRecord record;
     {
        ParserItem item("WAT_SALINITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("MINERAL_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("PHI_EFF_0", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PHI_EFF_1", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("C_0", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("C_K", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SHEAR_MOD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("ALPHA", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("METHOD", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PECOEFS::keywordName = "PECOEFS";
const std::string PECOEFS::WAT_SALINITY::itemName = "WAT_SALINITY";
const std::string PECOEFS::TEMP::itemName = "TEMP";
const std::string PECOEFS::MINERAL_DENSITY::itemName = "MINERAL_DENSITY";
const std::string PECOEFS::PHI_EFF_0::itemName = "PHI_EFF_0";
const std::string PECOEFS::PHI_EFF_1::itemName = "PHI_EFF_1";
const std::string PECOEFS::C_0::itemName = "C_0";
const std::string PECOEFS::C_K::itemName = "C_K";
const std::string PECOEFS::SHEAR_MOD::itemName = "SHEAR_MOD";
const std::string PECOEFS::ALPHA::itemName = "ALPHA";
const double PECOEFS::ALPHA::defaultValue = 1.000000;
const std::string PECOEFS::E::itemName = "E";
const double PECOEFS::E::defaultValue = 1.000000;
const std::string PECOEFS::METHOD::itemName = "METHOD";
const int PECOEFS::METHOD::defaultValue = 0;


PEDIMS::PEDIMS( ) : ParserKeyword("PEDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PEDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NUM_REGIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PRESSURE_POINTS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PEDIMS::keywordName = "PEDIMS";
const std::string PEDIMS::NUM_REGIONS::itemName = "NUM_REGIONS";
const int PEDIMS::NUM_REGIONS::defaultValue = 0;
const std::string PEDIMS::MAX_PRESSURE_POINTS::itemName = "MAX_PRESSURE_POINTS";
const int PEDIMS::MAX_PRESSURE_POINTS::defaultValue = 0;


PEGTABX::PEGTABX( ) : ParserKeyword("PEGTABX")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("PEDIMS","NUM_REGIONS",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PEGTAB0");
  addDeckName("PEGTAB1");
  addDeckName("PEGTAB2");
  addDeckName("PEGTAB3");
  addDeckName("PEGTAB4");
  addDeckName("PEGTAB5");
  addDeckName("PEGTAB6");
  addDeckName("PEGTAB7");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PEGTABX::keywordName = "PEGTABX";
const std::string PEGTABX::DATA::itemName = "DATA";


PEKTABX::PEKTABX( ) : ParserKeyword("PEKTABX")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("PEDIMS","NUM_REGIONS",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PEKTAB0");
  addDeckName("PEKTAB1");
  addDeckName("PEKTAB2");
  addDeckName("PEKTAB3");
  addDeckName("PEKTAB4");
  addDeckName("PEKTAB5");
  addDeckName("PEKTAB6");
  addDeckName("PEKTAB7");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PEKTABX::keywordName = "PEKTABX";
const std::string PEKTABX::DATA::itemName = "DATA";


PENUM::PENUM( ) : ParserKeyword("PENUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("PENUM");
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
const std::string PENUM::keywordName = "PENUM";
const std::string PENUM::data::itemName = "data";


PERFORMANCE_PROBE::PERFORMANCE_PROBE( ) : ParserKeyword("PERFORMANCE_PROBE")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ELAPSED");
  addDeckName("HLINEARS");
  addDeckName("HSUMLINS");
  addDeckName("MAXDPR");
  addDeckName("MAXDSG");
  addDeckName("MAXDSO");
  addDeckName("MAXDSW");
  addDeckName("MEMORYTS");
  addDeckName("MLINEARS");
  addDeckName("MSUMLINS");
  addDeckName("MSUMNEWT");
  addDeckName("NBYTOT");
  addDeckName("NEWTON");
  addDeckName("NLINEARS");
  addDeckName("NLINSMAX");
  addDeckName("NLINSMIN");
  addDeckName("PERFORMA");
  addDeckName("STEPTYPE");
  addDeckName("TCPU");
  addDeckName("TCPUDAY");
  addDeckName("TCPUH");
  addDeckName("TCPUHT");
  addDeckName("TCPUSCH");
  addDeckName("TCPUTS");
  addDeckName("TCPUTSH");
  addDeckName("TCPUTSHT");
  addDeckName("TELAPDAY");
  addDeckName("TELAPLIN");
  addDeckName("TELAPTS");
  addDeckName("TIMESTEP");
  addDeckName("WNEWTON");
  addDeckName("ZIPEFF");
  addDeckName("ZIPEFFC");
}
const std::string PERFORMANCE_PROBE::keywordName = "PERFORMANCE_PROBE";


PERMAVE::PERMAVE( ) : ParserKeyword("PERMAVE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMAVE");
  {
     ParserRecord record;
     {
        ParserItem item("EXPO_0", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("EXPO_1", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     {
        ParserItem item("EXPO_2", ParserItem::itype::INT);
        item.setDefault( -1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PERMAVE::keywordName = "PERMAVE";
const std::string PERMAVE::EXPO_0::itemName = "EXPO_0";
const int PERMAVE::EXPO_0::defaultValue = -1;
const std::string PERMAVE::EXPO_1::itemName = "EXPO_1";
const int PERMAVE::EXPO_1::defaultValue = -1;
const std::string PERMAVE::EXPO_2::itemName = "EXPO_2";
const int PERMAVE::EXPO_2::defaultValue = -1;


PERMFACT::PERMFACT( ) : ParserKeyword("PERMFACT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PERMFACT");
  {
     ParserRecord record;
     {
        ParserItem item("POROSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PERMFACTMULT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PERMFACT::keywordName = "PERMFACT";
const std::string PERMFACT::POROSITY::itemName = "POROSITY";
const std::string PERMFACT::PERMFACTMULT::itemName = "PERMFACTMULT";


PERMJFUN::PERMJFUN( ) : ParserKeyword("PERMJFUN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMJFUN");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMJFUN::keywordName = "PERMJFUN";
const std::string PERMJFUN::data::itemName = "data";


PERMR::PERMR( ) : ParserKeyword("PERMR")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMR::keywordName = "PERMR";
const std::string PERMR::data::itemName = "data";


PERMTHT::PERMTHT( ) : ParserKeyword("PERMTHT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMTHT");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMTHT::keywordName = "PERMTHT";
const std::string PERMTHT::data::itemName = "data";


PERMX::PERMX( ) : ParserKeyword("PERMX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMX::keywordName = "PERMX";
const std::string PERMX::data::itemName = "data";


PERMXY::PERMXY( ) : ParserKeyword("PERMXY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMXY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMXY::keywordName = "PERMXY";
const std::string PERMXY::data::itemName = "data";


PERMY::PERMY( ) : ParserKeyword("PERMY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMY::keywordName = "PERMY";
const std::string PERMY::data::itemName = "data";
const double PERMY::data::defaultValue = 0.000000;


PERMYZ::PERMYZ( ) : ParserKeyword("PERMYZ")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMYZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMYZ::keywordName = "PERMYZ";
const std::string PERMYZ::data::itemName = "data";


PERMZ::PERMZ( ) : ParserKeyword("PERMZ")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMZ");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMZ::keywordName = "PERMZ";
const std::string PERMZ::data::itemName = "data";
const double PERMZ::data::defaultValue = 0.000000;


PERMZX::PERMZX( ) : ParserKeyword("PERMZX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMZX");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Permeability");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMZX::keywordName = "PERMZX";
const std::string PERMZX::data::itemName = "data";


PETGRID::PETGRID( ) : ParserKeyword("PETGRID")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PETGRID");
  {
     ParserRecord record;
     {
        ParserItem item("FILE_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PETGRID::keywordName = "PETGRID";
const std::string PETGRID::FILE_NAME::itemName = "FILE_NAME";


PETOPTS::PETOPTS( ) : ParserKeyword("PETOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PETOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("OPTIONS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PETOPTS::keywordName = "PETOPTS";
const std::string PETOPTS::OPTIONS::itemName = "OPTIONS";


PICOND::PICOND( ) : ParserKeyword("PICOND")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PICOND");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_INTERVAL_BELOW_DEWPOINT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_INTERVAL_ABOVE_DEWPOINT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("D_F", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("INCLUDE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("F_L", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("F_U", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.100000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DELTA_WAT_SAT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DELTA_PRESSURE", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("DELTA_FRAC_COMP", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.010000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_DELTA_TIME", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("ADAPTIVE_ORD_CONTROL", ParserItem::itype::DOUBLE);
        item.setDefault( double(-1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("ADAPTIVE_ORD_MIN_SPACING", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PICOND::keywordName = "PICOND";
const std::string PICOND::MAX_INTERVAL_BELOW_DEWPOINT::itemName = "MAX_INTERVAL_BELOW_DEWPOINT";
const std::string PICOND::MAX_INTERVAL_ABOVE_DEWPOINT::itemName = "MAX_INTERVAL_ABOVE_DEWPOINT";
const std::string PICOND::D_F::itemName = "D_F";
const double PICOND::D_F::defaultValue = 1.000000;
const std::string PICOND::INCLUDE::itemName = "INCLUDE";
const std::string PICOND::INCLUDE::defaultValue = "NO";
const std::string PICOND::F_L::itemName = "F_L";
const double PICOND::F_L::defaultValue = 0.000000;
const std::string PICOND::F_U::itemName = "F_U";
const double PICOND::F_U::defaultValue = 1.100000;
const std::string PICOND::DELTA_WAT_SAT::itemName = "DELTA_WAT_SAT";
const std::string PICOND::DELTA_PRESSURE::itemName = "DELTA_PRESSURE";
const std::string PICOND::DELTA_FRAC_COMP::itemName = "DELTA_FRAC_COMP";
const double PICOND::DELTA_FRAC_COMP::defaultValue = 0.010000;
const std::string PICOND::MAX_DELTA_TIME::itemName = "MAX_DELTA_TIME";
const double PICOND::MAX_DELTA_TIME::defaultValue = -1.000000;
const std::string PICOND::ADAPTIVE_ORD_CONTROL::itemName = "ADAPTIVE_ORD_CONTROL";
const double PICOND::ADAPTIVE_ORD_CONTROL::defaultValue = -1.000000;
const std::string PICOND::ADAPTIVE_ORD_MIN_SPACING::itemName = "ADAPTIVE_ORD_MIN_SPACING";


PIMTDIMS::PIMTDIMS( ) : ParserKeyword("PIMTDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PIMTDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTPIMT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NPPIMT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PIMTDIMS::keywordName = "PIMTDIMS";
const std::string PIMTDIMS::NTPIMT::itemName = "NTPIMT";
const int PIMTDIMS::NTPIMT::defaultValue = 0;
const std::string PIMTDIMS::NPPIMT::itemName = "NPPIMT";
const int PIMTDIMS::NPPIMT::defaultValue = 0;


PIMULTAB::PIMULTAB( ) : ParserKeyword("PIMULTAB")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("PIMTDIMS","NTPIMT",0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PIMULTAB");
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
const std::string PIMULTAB::keywordName = "PIMULTAB";
const std::string PIMULTAB::TABLE::itemName = "TABLE";


PINCH::PINCH( ) : ParserKeyword("PINCH")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCH");
  {
     ParserRecord record;
     {
        ParserItem item("THRESHOLD_THICKNESS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("CONTROL_OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("GAP") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_EMPTY_GAP", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PINCHOUT_OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("TOPBOT") );
        record.addItem(item);
     }
     {
        ParserItem item("MULTZ_OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("TOP") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PINCH::keywordName = "PINCH";
const std::string PINCH::THRESHOLD_THICKNESS::itemName = "THRESHOLD_THICKNESS";
const double PINCH::THRESHOLD_THICKNESS::defaultValue = 0.001000;
const std::string PINCH::CONTROL_OPTION::itemName = "CONTROL_OPTION";
const std::string PINCH::CONTROL_OPTION::defaultValue = "GAP";
const std::string PINCH::MAX_EMPTY_GAP::itemName = "MAX_EMPTY_GAP";
const double PINCH::MAX_EMPTY_GAP::defaultValue = 100000000000000000000.000000;
const std::string PINCH::PINCHOUT_OPTION::itemName = "PINCHOUT_OPTION";
const std::string PINCH::PINCHOUT_OPTION::defaultValue = "TOPBOT";
const std::string PINCH::MULTZ_OPTION::itemName = "MULTZ_OPTION";
const std::string PINCH::MULTZ_OPTION::defaultValue = "TOP";


PINCHNUM::PINCHNUM( ) : ParserKeyword("PINCHNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCHNUM");
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
const std::string PINCHNUM::keywordName = "PINCHNUM";
const std::string PINCHNUM::data::itemName = "data";


PINCHOUT::PINCHOUT( ) : ParserKeyword("PINCHOUT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCHOUT");
}
const std::string PINCHOUT::keywordName = "PINCHOUT";


PINCHREG::PINCHREG( ) : ParserKeyword("PINCHREG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCHREG");
  {
     ParserRecord record;
     {
        ParserItem item("THRESHOLD_THICKNESS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION1", ParserItem::itype::STRING);
        item.setDefault( std::string("GAP") );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GAP", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION2", ParserItem::itype::STRING);
        item.setDefault( std::string("TOPBOT") );
        record.addItem(item);
     }
     {
        ParserItem item("OPTION3", ParserItem::itype::STRING);
        item.setDefault( std::string("TOP") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PINCHREG::keywordName = "PINCHREG";
const std::string PINCHREG::THRESHOLD_THICKNESS::itemName = "THRESHOLD_THICKNESS";
const double PINCHREG::THRESHOLD_THICKNESS::defaultValue = 0.001000;
const std::string PINCHREG::OPTION1::itemName = "OPTION1";
const std::string PINCHREG::OPTION1::defaultValue = "GAP";
const std::string PINCHREG::MAX_GAP::itemName = "MAX_GAP";
const double PINCHREG::MAX_GAP::defaultValue = 100000000000000000000.000000;
const std::string PINCHREG::OPTION2::itemName = "OPTION2";
const std::string PINCHREG::OPTION2::defaultValue = "TOPBOT";
const std::string PINCHREG::OPTION3::itemName = "OPTION3";
const std::string PINCHREG::OPTION3::defaultValue = "TOP";


PINCHXY::PINCHXY( ) : ParserKeyword("PINCHXY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCHXY");
  {
     ParserRecord record;
     {
        ParserItem item("THRESHOLD_XR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("THRESHOLD_YTHETA", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.001000) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PINCHXY::keywordName = "PINCHXY";
const std::string PINCHXY::THRESHOLD_XR::itemName = "THRESHOLD_XR";
const double PINCHXY::THRESHOLD_XR::defaultValue = 0.001000;
const std::string PINCHXY::THRESHOLD_YTHETA::itemName = "THRESHOLD_YTHETA";
const double PINCHXY::THRESHOLD_YTHETA::defaultValue = 0.001000;


PINTDIMS::PINTDIMS( ) : ParserKeyword("PINTDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PINTDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTSKWAT", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NTSKPOLY", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NTPMWINJ", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PINTDIMS::keywordName = "PINTDIMS";
const std::string PINTDIMS::NTSKWAT::itemName = "NTSKWAT";
const int PINTDIMS::NTSKWAT::defaultValue = 1;
const std::string PINTDIMS::NTSKPOLY::itemName = "NTSKPOLY";
const int PINTDIMS::NTSKPOLY::defaultValue = 1;
const std::string PINTDIMS::NTPMWINJ::itemName = "NTPMWINJ";
const int PINTDIMS::NTPMWINJ::defaultValue = 1;


PLMIXNUM::PLMIXNUM( ) : ParserKeyword("PLMIXNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("PLMIXNUM");
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
const std::string PLMIXNUM::keywordName = "PLMIXNUM";
const std::string PLMIXNUM::data::itemName = "data";


PLMIXPAR::PLMIXPAR( ) : ParserKeyword("PLMIXPAR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NPLMIX",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLMIXPAR");
  {
     ParserRecord record;
     {
        ParserItem item("TODD_LONGSTAFF", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLMIXPAR::keywordName = "PLMIXPAR";
const std::string PLMIXPAR::TODD_LONGSTAFF::itemName = "TODD_LONGSTAFF";


PLYADS::PLYADS( ) : ParserKeyword("PLYADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYADS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("PolymerDensity");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYADS::keywordName = "PLYADS";
const std::string PLYADS::DATA::itemName = "DATA";


PLYADSS::PLYADSS( ) : ParserKeyword("PLYADSS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYADSS");
  {
     ParserRecord record;
     {
        ParserItem item("POLYMER_C", ParserItem::itype::DOUBLE);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     {
        ParserItem item("POLYMER_ADS_C", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYADSS::keywordName = "PLYADSS";
const std::string PLYADSS::POLYMER_C::itemName = "POLYMER_C";
const std::string PLYADSS::POLYMER_ADS_C::itemName = "POLYMER_ADS_C";
const std::string PLYADSS::DATA::itemName = "DATA";


PLYATEMP::PLYATEMP( ) : ParserKeyword("PLYATEMP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYATEMP");
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
const std::string PLYATEMP::keywordName = "PLYATEMP";
const std::string PLYATEMP::data::itemName = "data";


PLYCAMAX::PLYCAMAX( ) : ParserKeyword("PLYCAMAX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYCAMAX");
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
const std::string PLYCAMAX::keywordName = "PLYCAMAX";
const std::string PLYCAMAX::data::itemName = "data";


PLYDHFLF::PLYDHFLF( ) : ParserKeyword("PLYDHFLF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYDHFLF");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYDHFLF::keywordName = "PLYDHFLF";
const std::string PLYDHFLF::DATA::itemName = "DATA";


PLYESAL::PLYESAL( ) : ParserKeyword("PLYESAL")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYESAL");
  {
     ParserRecord record;
     {
        ParserItem item("ALPHAP", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYESAL::keywordName = "PLYESAL";
const std::string PLYESAL::ALPHAP::itemName = "ALPHAP";


PLYKRRF::PLYKRRF( ) : ParserKeyword("PLYKRRF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYKRRF");
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
const std::string PLYKRRF::keywordName = "PLYKRRF";
const std::string PLYKRRF::data::itemName = "data";


PLYMAX::PLYMAX( ) : ParserKeyword("PLYMAX")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NPLMIX",0);
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYMAX");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_POLYMER_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SALT_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYMAX::keywordName = "PLYMAX";
const std::string PLYMAX::MAX_POLYMER_CONCENTRATION::itemName = "MAX_POLYMER_CONCENTRATION";
const std::string PLYMAX::MAX_SALT_CONCENTRATION::itemName = "MAX_SALT_CONCENTRATION";


PLYMWINJ::PLYMWINJ( ) : ParserKeyword("PLYMWINJ")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYMWINJ");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("THROUGHPUT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("VELOCITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length/Time");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("MOLECULARWEIGHT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYMWINJ::keywordName = "PLYMWINJ";
const std::string PLYMWINJ::TABLE_NUMBER::itemName = "TABLE_NUMBER";
const std::string PLYMWINJ::THROUGHPUT::itemName = "THROUGHPUT";
const std::string PLYMWINJ::VELOCITY::itemName = "VELOCITY";
const std::string PLYMWINJ::MOLECULARWEIGHT::itemName = "MOLECULARWEIGHT";


PLYOPTS::PLYOPTS( ) : ParserKeyword("PLYOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_SWAT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYOPTS::keywordName = "PLYOPTS";
const std::string PLYOPTS::MIN_SWAT::itemName = "MIN_SWAT";
const double PLYOPTS::MIN_SWAT::defaultValue = 0.000001;


PLYRMDEN::PLYRMDEN( ) : ParserKeyword("PLYRMDEN")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PLYRMDEN");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/ReservoirVolume");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PLYRMDEN::keywordName = "PLYRMDEN";
const std::string PLYRMDEN::data::itemName = "data";


PLYROCK::PLYROCK( ) : ParserKeyword("PLYROCK")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYROCK");
  {
     ParserRecord record;
     {
        ParserItem item("IPV", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("RRF", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("ROCK_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("AI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ADSORPTION", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYROCK::keywordName = "PLYROCK";
const std::string PLYROCK::IPV::itemName = "IPV";
const std::string PLYROCK::RRF::itemName = "RRF";
const std::string PLYROCK::ROCK_DENSITY::itemName = "ROCK_DENSITY";
const std::string PLYROCK::AI::itemName = "AI";
const double PLYROCK::AI::defaultValue = 1.000000;
const std::string PLYROCK::MAX_ADSORPTION::itemName = "MAX_ADSORPTION";


PLYROCKM::PLYROCKM( ) : ParserKeyword("PLYROCKM")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PLYROCKM");
  {
     ParserRecord record;
     {
        ParserItem item("IPV", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("RRF", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("ROCK_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("AI", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ADSORPTION", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYROCKM::keywordName = "PLYROCKM";
const std::string PLYROCKM::IPV::itemName = "IPV";
const std::string PLYROCKM::RRF::itemName = "RRF";
const std::string PLYROCKM::ROCK_DENSITY::itemName = "ROCK_DENSITY";
const std::string PLYROCKM::AI::itemName = "AI";
const double PLYROCKM::AI::defaultValue = 1.000000;
const std::string PLYROCKM::MAX_ADSORPTION::itemName = "MAX_ADSORPTION";


PLYSHEAR::PLYSHEAR( ) : ParserKeyword("PLYSHEAR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYSHEAR");
  {
     ParserRecord record;
     {
        ParserItem item("WATER_VELOCITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length/Time");
        record.addItem(item);
     }
     {
        ParserItem item("VRF", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYSHEAR::keywordName = "PLYSHEAR";
const std::string PLYSHEAR::WATER_VELOCITY::itemName = "WATER_VELOCITY";
const std::string PLYSHEAR::VRF::itemName = "VRF";


PLYSHLOG::PLYSHLOG( ) : ParserKeyword("PLYSHLOG")
{
  setFixedSize( (size_t) 2);
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYSHLOG");
  {
     ParserRecord record;
     {
        ParserItem item("REF_POLYMER_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.push_backDimension("PolymerDensity");
        record.addItem(item);
     }
     {
        ParserItem item("REF_SALINITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     {
        ParserItem item("REF_TEMPERATURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYSHLOG::keywordName = "PLYSHLOG";
const std::string PLYSHLOG::REF_POLYMER_CONCENTRATION::itemName = "REF_POLYMER_CONCENTRATION";
const std::string PLYSHLOG::REF_SALINITY::itemName = "REF_SALINITY";
const std::string PLYSHLOG::REF_TEMPERATURE::itemName = "REF_TEMPERATURE";
const std::string PLYSHLOG::DATA::itemName = "DATA";


PLYTRRF::PLYTRRF( ) : ParserKeyword("PLYTRRF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYTRRF");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYTRRF::keywordName = "PLYTRRF";
const std::string PLYTRRF::DATA::itemName = "DATA";


PLYTRRFA::PLYTRRFA( ) : ParserKeyword("PLYTRRFA")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYTRRFA");
  {
     ParserRecord record;
     {
        ParserItem item("NBTRRF", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYTRRFA::keywordName = "PLYTRRFA";
const std::string PLYTRRFA::NBTRRF::itemName = "NBTRRF";


PLYVISC::PLYVISC( ) : ParserKeyword("PLYVISC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYVISC");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("PolymerDensity");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVISC::keywordName = "PLYVISC";
const std::string PLYVISC::DATA::itemName = "DATA";


PLYVISCS::PLYVISCS( ) : ParserKeyword("PLYVISCS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PLYVISCS");
  {
     ParserRecord record;
     {
        ParserItem item("PC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVISCS::keywordName = "PLYVISCS";
const std::string PLYVISCS::PC::itemName = "PC";
const std::string PLYVISCS::DATA::itemName = "DATA";


PLYVISCT::PLYVISCT( ) : ParserKeyword("PLYVISCT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PLYVISCT");
  {
     ParserRecord record;
     {
        ParserItem item("PC", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVISCT::keywordName = "PLYVISCT";
const std::string PLYVISCT::PC::itemName = "PC";
const std::string PLYVISCT::DATA::itemName = "DATA";


PLYVMH::PLYVMH( ) : ParserKeyword("PLYVMH")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NPLMIX",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYVMH");
  {
     ParserRecord record;
     {
        ParserItem item("K_MH", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("A_MH", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("GAMMA", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KAPPA", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVMH::keywordName = "PLYVMH";
const std::string PLYVMH::K_MH::itemName = "K_MH";
const std::string PLYVMH::A_MH::itemName = "A_MH";
const std::string PLYVMH::GAMMA::itemName = "GAMMA";
const std::string PLYVMH::KAPPA::itemName = "KAPPA";


PLYVSCST::PLYVSCST( ) : ParserKeyword("PLYVSCST")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PLYVSCST");
  {
     ParserRecord record;
     {
        ParserItem item("PC1", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("MULT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVSCST::keywordName = "PLYVSCST";
const std::string PLYVSCST::PC1::itemName = "PC1";
const std::string PLYVSCST::MULT::itemName = "MULT";


PMAX::PMAX( ) : ParserKeyword("PMAX")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PMAX");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("MAX_PRESSURE_CHECK", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_PRESSURE_CHECK", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("NUM_NODES", ParserItem::itype::INT);
        item.setDefault( 30 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PMAX::keywordName = "PMAX";
const std::string PMAX::MAX_PRESSURE::itemName = "MAX_PRESSURE";
const std::string PMAX::MAX_PRESSURE_CHECK::itemName = "MAX_PRESSURE_CHECK";
const double PMAX::MAX_PRESSURE_CHECK::defaultValue = 0.000000;
const std::string PMAX::MIN_PRESSURE_CHECK::itemName = "MIN_PRESSURE_CHECK";
const double PMAX::MIN_PRESSURE_CHECK::defaultValue = 100000000000000000000.000000;
const std::string PMAX::NUM_NODES::itemName = "NUM_NODES";
const int PMAX::NUM_NODES::defaultValue = 30;


PMISC::PMISC( ) : ParserKeyword("PMISC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PMISC");
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
const std::string PMISC::keywordName = "PMISC";
const std::string PMISC::DATA::itemName = "DATA";


POLYMER::POLYMER( ) : ParserKeyword("POLYMER")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("POLYMER");
}
const std::string POLYMER::keywordName = "POLYMER";


POLYMW::POLYMW( ) : ParserKeyword("POLYMW")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("POLYMW");
}
const std::string POLYMW::keywordName = "POLYMW";


PORO::PORO( ) : ParserKeyword("PORO")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PORO");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PORO::keywordName = "PORO";
const std::string PORO::data::itemName = "data";
const double PORO::data::defaultValue = 0.000000;


PORV::PORV( ) : ParserKeyword("PORV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("PORV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("ReservoirVolume");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PORV::keywordName = "PORV";
const std::string PORV::data::itemName = "data";


PPCWMAX::PPCWMAX( ) : ParserKeyword("PPCWMAX")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PPCWMAX");
  {
     ParserRecord record;
     {
        ParserItem item("MAXIMUM_CAPILLARY_PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(100000000000000000000.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("MODIFY_CONNATE_SATURATION", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PPCWMAX::keywordName = "PPCWMAX";
const std::string PPCWMAX::MAXIMUM_CAPILLARY_PRESSURE::itemName = "MAXIMUM_CAPILLARY_PRESSURE";
const double PPCWMAX::MAXIMUM_CAPILLARY_PRESSURE::defaultValue = 100000000000000000000.000000;
const std::string PPCWMAX::MODIFY_CONNATE_SATURATION::itemName = "MODIFY_CONNATE_SATURATION";
const std::string PPCWMAX::MODIFY_CONNATE_SATURATION::defaultValue = "NO";


PRECSALT::PRECSALT( ) : ParserKeyword("PRECSALT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PRECSALT");
}
const std::string PRECSALT::keywordName = "PRECSALT";


PREF::PREF( ) : ParserKeyword("PREF")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PREF");
  {
     ParserRecord record;
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PREF::keywordName = "PREF";
const std::string PREF::PRESSURE::itemName = "PRESSURE";


PREFS::PREFS( ) : ParserKeyword("PREFS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PREFS");
  {
     ParserRecord record;
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PREFS::keywordName = "PREFS";
const std::string PREFS::PRESSURE::itemName = "PRESSURE";


PRESSURE::PRESSURE( ) : ParserKeyword("PRESSURE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PRESSURE");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PRESSURE::keywordName = "PRESSURE";
const std::string PRESSURE::data::itemName = "data";


PRIORITY::PRIORITY( ) : ParserKeyword("PRIORITY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PRIORITY");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_CALC_TIME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("A1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("B1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("C1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("D1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("E1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("F1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("G1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("H1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("A2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("B2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("C2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("D2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("E2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("F2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("G2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("H2", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PRIORITY::keywordName = "PRIORITY";
const std::string PRIORITY::MIN_CALC_TIME::itemName = "MIN_CALC_TIME";
const std::string PRIORITY::A1::itemName = "A1";
const double PRIORITY::A1::defaultValue = 0.000000;
const std::string PRIORITY::B1::itemName = "B1";
const double PRIORITY::B1::defaultValue = 0.000000;
const std::string PRIORITY::C1::itemName = "C1";
const double PRIORITY::C1::defaultValue = 0.000000;
const std::string PRIORITY::D1::itemName = "D1";
const double PRIORITY::D1::defaultValue = 0.000000;
const std::string PRIORITY::E1::itemName = "E1";
const double PRIORITY::E1::defaultValue = 0.000000;
const std::string PRIORITY::F1::itemName = "F1";
const double PRIORITY::F1::defaultValue = 0.000000;
const std::string PRIORITY::G1::itemName = "G1";
const double PRIORITY::G1::defaultValue = 0.000000;
const std::string PRIORITY::H1::itemName = "H1";
const double PRIORITY::H1::defaultValue = 0.000000;
const std::string PRIORITY::A2::itemName = "A2";
const double PRIORITY::A2::defaultValue = 0.000000;
const std::string PRIORITY::B2::itemName = "B2";
const double PRIORITY::B2::defaultValue = 0.000000;
const std::string PRIORITY::C2::itemName = "C2";
const double PRIORITY::C2::defaultValue = 0.000000;
const std::string PRIORITY::D2::itemName = "D2";
const double PRIORITY::D2::defaultValue = 0.000000;
const std::string PRIORITY::E2::itemName = "E2";
const double PRIORITY::E2::defaultValue = 0.000000;
const std::string PRIORITY::F2::itemName = "F2";
const double PRIORITY::F2::defaultValue = 0.000000;
const std::string PRIORITY::G2::itemName = "G2";
const double PRIORITY::G2::defaultValue = 0.000000;
const std::string PRIORITY::H2::itemName = "H2";
const double PRIORITY::H2::defaultValue = 0.000000;


PROPS::PROPS( ) : ParserKeyword("PROPS")
{
  setFixedSize( (size_t) 0);
  clearDeckNames();
  addDeckName("PROPS");
}
const std::string PROPS::keywordName = "PROPS";


PRORDER::PRORDER( ) : ParserKeyword("PRORDER")
{
  setFixedSize( (size_t) 2);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PRORDER");
  {
     ParserRecord record;
     {
        ParserItem item("NO1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NO2", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NO3", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NO4", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NO5", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("OPT1", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("OPT2", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("OPT3", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("OPT4", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     {
        ParserItem item("OPT5", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PRORDER::keywordName = "PRORDER";
const std::string PRORDER::NO1::itemName = "NO1";
const std::string PRORDER::NO2::itemName = "NO2";
const std::string PRORDER::NO3::itemName = "NO3";
const std::string PRORDER::NO4::itemName = "NO4";
const std::string PRORDER::NO5::itemName = "NO5";
const std::string PRORDER::OPT1::itemName = "OPT1";
const std::string PRORDER::OPT1::defaultValue = "YES";
const std::string PRORDER::OPT2::itemName = "OPT2";
const std::string PRORDER::OPT2::defaultValue = "YES";
const std::string PRORDER::OPT3::itemName = "OPT3";
const std::string PRORDER::OPT3::defaultValue = "YES";
const std::string PRORDER::OPT4::itemName = "OPT4";
const std::string PRORDER::OPT4::defaultValue = "YES";
const std::string PRORDER::OPT5::itemName = "OPT5";
const std::string PRORDER::OPT5::defaultValue = "YES";


PRVD::PRVD( ) : ParserKeyword("PRVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PRVD");
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
const std::string PRVD::keywordName = "PRVD";
const std::string PRVD::DATA::itemName = "DATA";


PSTEADY::PSTEADY( ) : ParserKeyword("PSTEADY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PSTEADY");
  {
     ParserRecord record;
     {
        ParserItem item("DAY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MONTH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("YEAR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DIFF", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE_TOL", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("OIL_TOL", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("WATER_TOL", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("GAS_TOL", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("BRINE_TOL", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("MAX_TIME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_TIME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("PIM_AQUIFERS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PSTEADY::keywordName = "PSTEADY";
const std::string PSTEADY::DAY::itemName = "DAY";
const std::string PSTEADY::MONTH::itemName = "MONTH";
const std::string PSTEADY::YEAR::itemName = "YEAR";
const std::string PSTEADY::DIFF::itemName = "DIFF";
const std::string PSTEADY::PRESSURE_TOL::itemName = "PRESSURE_TOL";
const std::string PSTEADY::OIL_TOL::itemName = "OIL_TOL";
const std::string PSTEADY::WATER_TOL::itemName = "WATER_TOL";
const std::string PSTEADY::GAS_TOL::itemName = "GAS_TOL";
const std::string PSTEADY::BRINE_TOL::itemName = "BRINE_TOL";
const std::string PSTEADY::MAX_TIME::itemName = "MAX_TIME";
const std::string PSTEADY::MIN_TIME::itemName = "MIN_TIME";
const std::string PSTEADY::PIM_AQUIFERS::itemName = "PIM_AQUIFERS";
const std::string PSTEADY::PIM_AQUIFERS::defaultValue = "NO";


PSWRG::PSWRG( ) : ParserKeyword("PSWRG")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PSWRG");
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
const std::string PSWRG::keywordName = "PSWRG";
const std::string PSWRG::data::itemName = "data";


PSWRO::PSWRO( ) : ParserKeyword("PSWRO")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PSWRO");
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
const std::string PSWRO::keywordName = "PSWRO";
const std::string PSWRO::data::itemName = "data";


PVCDO::PVCDO( ) : ParserKeyword("PVCDO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVCDO");
  {
     ParserRecord record;
     {
        ParserItem item("P_REF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_VOL_FACTOR", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_VISCOSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     {
        ParserItem item("OIL_VISCOSIBILITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVCDO::keywordName = "PVCDO";
const std::string PVCDO::P_REF::itemName = "P_REF";
const std::string PVCDO::OIL_VOL_FACTOR::itemName = "OIL_VOL_FACTOR";
const std::string PVCDO::OIL_COMPRESSIBILITY::itemName = "OIL_COMPRESSIBILITY";
const std::string PVCDO::OIL_VISCOSITY::itemName = "OIL_VISCOSITY";
const std::string PVCDO::OIL_VISCOSIBILITY::itemName = "OIL_VISCOSIBILITY";


PVCO::PVCO( ) : ParserKeyword("PVCO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVCO");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("GasSurfaceVolume/LiquidSurfaceVolume");
        item.push_backDimension("ReservoirVolume/LiquidSurfaceVolume");
        item.push_backDimension("Viscosity");
        item.push_backDimension("1/Pressure");
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVCO::keywordName = "PVCO";
const std::string PVCO::DATA::itemName = "DATA";


PVDG::PVDG( ) : ParserKeyword("PVDG")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDG");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDG::keywordName = "PVDG";
const std::string PVDG::DATA::itemName = "DATA";


PVDO::PVDO( ) : ParserKeyword("PVDO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDO");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDO::keywordName = "PVDO";
const std::string PVDO::DATA::itemName = "DATA";


PVDS::PVDS( ) : ParserKeyword("PVDS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDS::keywordName = "PVDS";
const std::string PVDS::DATA::itemName = "DATA";


PVTG::PVTG( ) : ParserKeyword("PVTG")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTG");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTG::keywordName = "PVTG";
const std::string PVTG::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string PVTG::DATA::itemName = "DATA";


PVTGW::PVTGW( ) : ParserKeyword("PVTGW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTGW");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTGW::keywordName = "PVTGW";
const std::string PVTGW::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string PVTGW::DATA::itemName = "DATA";


PVTGWO::PVTGWO( ) : ParserKeyword("PVTGWO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTGWO");
  {
     ParserRecord record;
     {
        ParserItem item("GAS_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTGWO::keywordName = "PVTGWO";
const std::string PVTGWO::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string PVTGWO::DATA::itemName = "DATA";


PVTNUM::PVTNUM( ) : ParserKeyword("PVTNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("PVTNUM");
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
const std::string PVTNUM::keywordName = "PVTNUM";
const std::string PVTNUM::data::itemName = "data";


PVTO::PVTO( ) : ParserKeyword("PVTO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTO");
  {
     ParserRecord record;
     {
        ParserItem item("RS", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTO::keywordName = "PVTO";
const std::string PVTO::RS::itemName = "RS";
const std::string PVTO::DATA::itemName = "DATA";


PVTW::PVTW( ) : ParserKeyword("PVTW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTW");
  {
     ParserRecord record;
     {
        ParserItem item("P_REF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_VOL_FACTOR", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(4e-05) );
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_VISCOSITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     {
        ParserItem item("WATER_VISCOSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTW::keywordName = "PVTW";
const std::string PVTW::P_REF::itemName = "P_REF";
const std::string PVTW::WATER_VOL_FACTOR::itemName = "WATER_VOL_FACTOR";
const double PVTW::WATER_VOL_FACTOR::defaultValue = 1.000000;
const std::string PVTW::WATER_COMPRESSIBILITY::itemName = "WATER_COMPRESSIBILITY";
const double PVTW::WATER_COMPRESSIBILITY::defaultValue = 0.000040;
const std::string PVTW::WATER_VISCOSITY::itemName = "WATER_VISCOSITY";
const double PVTW::WATER_VISCOSITY::defaultValue = 0.500000;
const std::string PVTW::WATER_VISCOSIBILITY::itemName = "WATER_VISCOSIBILITY";
const double PVTW::WATER_VISCOSIBILITY::defaultValue = 0.000000;


PVTWSALT::PVTWSALT( ) : ParserKeyword("PVTWSALT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTWSALT");
  setAlternatingKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("P_REF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("SALT_CONCENTRATION_REF", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        item.push_backDimension("1/Pressure");
        item.push_backDimension("Viscosity");
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTWSALT::keywordName = "PVTWSALT";
const std::string PVTWSALT::P_REF::itemName = "P_REF";
const std::string PVTWSALT::SALT_CONCENTRATION_REF::itemName = "SALT_CONCENTRATION_REF";
const double PVTWSALT::SALT_CONCENTRATION_REF::defaultValue = 0.000000;
const std::string PVTWSALT::DATA::itemName = "DATA";


PVT_M::PVT_M( ) : ParserKeyword("PVT_M")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PVT-M");
}
const std::string PVT_M::keywordName = "PVT_M";


PVZG::PVZG( ) : ParserKeyword("PVZG")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVZG");
  setAlternatingKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("REF_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("table", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVZG::keywordName = "PVZG";
const std::string PVZG::REF_TEMP::itemName = "REF_TEMP";
const std::string PVZG::table::itemName = "table";


PYACTION::PYACTION( ) : ParserKeyword("PYACTION")
{
  setFixedSize( (size_t) 2);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PYACTION");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("RUN_COUNT", ParserItem::itype::STRING);
        item.setDefault( std::string("SINGLE") );
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("FILENAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PYACTION::keywordName = "PYACTION";
const std::string PYACTION::NAME::itemName = "NAME";
const std::string PYACTION::RUN_COUNT::itemName = "RUN_COUNT";
const std::string PYACTION::RUN_COUNT::defaultValue = "SINGLE";
const std::string PYACTION::FILENAME::itemName = "FILENAME";


PYINPUT::PYINPUT( ) : ParserKeyword("PYINPUT")
{
  setSizeType(FIXED_CODE);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("PYINPUT");
  setCodeEnd("PYEND");
  {
     ParserRecord record;
     {
        ParserItem item("code", ParserItem::itype::RAW_STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string PYINPUT::keywordName = "PYINPUT";
const std::string PYINPUT::code::itemName = "code";


}
}
