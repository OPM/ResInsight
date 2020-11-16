#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/R.hpp>
namespace Opm {
namespace ParserKeywords {
RADFIN4::RADFIN4( ) : ParserKeyword("RADFIN4")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("RADFIN4");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
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
        ParserItem item("NR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NTHETA", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NWMAX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RADFIN4::keywordName = "RADFIN4";
const std::string RADFIN4::NAME::itemName = "NAME";
const std::string RADFIN4::I1::itemName = "I1";
const std::string RADFIN4::I2::itemName = "I2";
const std::string RADFIN4::J1::itemName = "J1";
const std::string RADFIN4::J2::itemName = "J2";
const std::string RADFIN4::K1::itemName = "K1";
const std::string RADFIN4::K2::itemName = "K2";
const std::string RADFIN4::NR::itemName = "NR";
const std::string RADFIN4::NTHETA::itemName = "NTHETA";
const std::string RADFIN4::NZ::itemName = "NZ";
const std::string RADFIN4::NWMAX::itemName = "NWMAX";
const int RADFIN4::NWMAX::defaultValue = 1;


RADIAL::RADIAL( ) : ParserKeyword("RADIAL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RADIAL");
}
const std::string RADIAL::keywordName = "RADIAL";


RAINFALL::RAINFALL( ) : ParserKeyword("RAINFALL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RAINFALL");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JAN_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("FEB_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("MAR_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("APR_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("MAI_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("JUN_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("JUL_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("AUG_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("SEP_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("OCT_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("NOV_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("DES_FLUX", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/Time*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RAINFALL::keywordName = "RAINFALL";
const std::string RAINFALL::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string RAINFALL::JAN_FLUX::itemName = "JAN_FLUX";
const std::string RAINFALL::FEB_FLUX::itemName = "FEB_FLUX";
const std::string RAINFALL::MAR_FLUX::itemName = "MAR_FLUX";
const std::string RAINFALL::APR_FLUX::itemName = "APR_FLUX";
const std::string RAINFALL::MAI_FLUX::itemName = "MAI_FLUX";
const std::string RAINFALL::JUN_FLUX::itemName = "JUN_FLUX";
const std::string RAINFALL::JUL_FLUX::itemName = "JUL_FLUX";
const std::string RAINFALL::AUG_FLUX::itemName = "AUG_FLUX";
const std::string RAINFALL::SEP_FLUX::itemName = "SEP_FLUX";
const std::string RAINFALL::OCT_FLUX::itemName = "OCT_FLUX";
const std::string RAINFALL::NOV_FLUX::itemName = "NOV_FLUX";
const std::string RAINFALL::DES_FLUX::itemName = "DES_FLUX";


RBEDCONT::RBEDCONT( ) : ParserKeyword("RBEDCONT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("RIVRDIMS","MXTBGR",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RBEDCONT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RBEDCONT::keywordName = "RBEDCONT";
const std::string RBEDCONT::DATA::itemName = "DATA";


RCMASTS::RCMASTS( ) : ParserKeyword("RCMASTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RCMASTS");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_TSTEP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RCMASTS::keywordName = "RCMASTS";
const std::string RCMASTS::MIN_TSTEP::itemName = "MIN_TSTEP";


REACHES::REACHES( ) : ParserKeyword("REACHES")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("REACHES");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("XPOS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("YPOS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("ZPOS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH1", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("INPUT_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("START_REACH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("END_REACH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("OUTLET_REACH", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("BRANCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("LENGTH2", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("XLENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("YLENGTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("REACH_LENGTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("NUM_REACHES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_SOMETHING", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string REACHES::keywordName = "REACHES";
const std::string REACHES::RIVER::itemName = "RIVER";
const std::string REACHES::XPOS::itemName = "XPOS";
const std::string REACHES::YPOS::itemName = "YPOS";
const std::string REACHES::ZPOS::itemName = "ZPOS";
const std::string REACHES::LENGTH1::itemName = "LENGTH1";
const std::string REACHES::INPUT_TYPE::itemName = "INPUT_TYPE";
const std::string REACHES::START_REACH::itemName = "START_REACH";
const std::string REACHES::END_REACH::itemName = "END_REACH";
const std::string REACHES::OUTLET_REACH::itemName = "OUTLET_REACH";
const std::string REACHES::BRANCH::itemName = "BRANCH";
const std::string REACHES::LENGTH2::itemName = "LENGTH2";
const std::string REACHES::DEPTH::itemName = "DEPTH";
const std::string REACHES::PROFILE::itemName = "PROFILE";
const int REACHES::PROFILE::defaultValue = 1;
const std::string REACHES::ROUGHNESS::itemName = "ROUGHNESS";
const std::string REACHES::XLENGTH::itemName = "XLENGTH";
const std::string REACHES::YLENGTH::itemName = "YLENGTH";
const std::string REACHES::REACH_LENGTH::itemName = "REACH_LENGTH";
const double REACHES::REACH_LENGTH::defaultValue = 0.000000;
const std::string REACHES::NUM_REACHES::itemName = "NUM_REACHES";
const int REACHES::NUM_REACHES::defaultValue = 1;
const std::string REACHES::DEPTH_SOMETHING::itemName = "DEPTH_SOMETHING";
const int REACHES::DEPTH_SOMETHING::defaultValue = 1;


READDATA::READDATA( ) : ParserKeyword("READDATA")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("READDATA");
  {
     ParserRecord record;
     {
        ParserItem item("INPUT_METHOD", ParserItem::itype::STRING);
        item.setDefault( std::string("FILE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string READDATA::keywordName = "READDATA";
const std::string READDATA::INPUT_METHOD::itemName = "INPUT_METHOD";
const std::string READDATA::INPUT_METHOD::defaultValue = "FILE";


REFINE::REFINE( ) : ParserKeyword("REFINE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("REFINE");
  {
     ParserRecord record;
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string REFINE::keywordName = "REFINE";
const std::string REFINE::LGR::itemName = "LGR";


REGDIMS::REGDIMS( ) : ParserKeyword("REGDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("REGDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTFIP", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NMFIPR", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NRFREG", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NTFREG", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ETRACK", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NTCREG", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OPERNUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OPERATE_DWORK", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_OPERATE_IWORK", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NPLMIX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string REGDIMS::keywordName = "REGDIMS";
const std::string REGDIMS::NTFIP::itemName = "NTFIP";
const int REGDIMS::NTFIP::defaultValue = 1;
const std::string REGDIMS::NMFIPR::itemName = "NMFIPR";
const int REGDIMS::NMFIPR::defaultValue = 1;
const std::string REGDIMS::NRFREG::itemName = "NRFREG";
const int REGDIMS::NRFREG::defaultValue = 0;
const std::string REGDIMS::NTFREG::itemName = "NTFREG";
const int REGDIMS::NTFREG::defaultValue = 0;
const std::string REGDIMS::MAX_ETRACK::itemName = "MAX_ETRACK";
const int REGDIMS::MAX_ETRACK::defaultValue = 0;
const std::string REGDIMS::NTCREG::itemName = "NTCREG";
const int REGDIMS::NTCREG::defaultValue = 1;
const std::string REGDIMS::MAX_OPERNUM::itemName = "MAX_OPERNUM";
const int REGDIMS::MAX_OPERNUM::defaultValue = 0;
const std::string REGDIMS::MAX_OPERATE_DWORK::itemName = "MAX_OPERATE_DWORK";
const int REGDIMS::MAX_OPERATE_DWORK::defaultValue = 0;
const std::string REGDIMS::MAX_OPERATE_IWORK::itemName = "MAX_OPERATE_IWORK";
const int REGDIMS::MAX_OPERATE_IWORK::defaultValue = 0;
const std::string REGDIMS::NPLMIX::itemName = "NPLMIX";
const int REGDIMS::NPLMIX::defaultValue = 1;


REGION2REGION_PROBE::REGION2REGION_PROBE( ) : ParserKeyword("REGION2REGION_PROBE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RGFT");
  addDeckName("ROFT");
  addDeckName("RWFT");
  {
     ParserRecord record;
     {
        ParserItem item("REGION1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REGION2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string REGION2REGION_PROBE::keywordName = "REGION2REGION_PROBE";
const std::string REGION2REGION_PROBE::REGION1::itemName = "REGION1";
const std::string REGION2REGION_PROBE::REGION2::itemName = "REGION2";


REGIONS::REGIONS( ) : ParserKeyword("REGIONS")
{
  setFixedSize( (size_t) 0);
  clearDeckNames();
  addDeckName("REGIONS");
}
const std::string REGIONS::keywordName = "REGIONS";


REGION_PROBE::REGION_PROBE( ) : ParserKeyword("REGION_PROBE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RAPI");
  addDeckName("RCAD");
  addDeckName("RCFT");
  addDeckName("RCGC");
  addDeckName("RCIP");
  addDeckName("RCSC");
  addDeckName("RGDEN");
  addDeckName("RGFTG");
  addDeckName("RGFTL");
  addDeckName("RGIP");
  addDeckName("RGIPG");
  addDeckName("RGIPL");
  addDeckName("RGIR");
  addDeckName("RGIT");
  addDeckName("RGP");
  addDeckName("RGPR");
  addDeckName("RGPRF");
  addDeckName("RGPRS");
  addDeckName("RGPT");
  addDeckName("RGPTF");
  addDeckName("RGPTS");
  addDeckName("RGPV");
  addDeckName("RGSAT");
  addDeckName("RGVIS");
  addDeckName("RHPV");
  addDeckName("RNFT");
  addDeckName("RNIP");
  addDeckName("RODEN");
  addDeckName("ROE");
  addDeckName("ROEIG");
  addDeckName("ROEIW");
  addDeckName("ROEW");
  addDeckName("ROEWG");
  addDeckName("ROEWW");
  addDeckName("ROIP");
  addDeckName("ROIPG");
  addDeckName("ROIPL");
  addDeckName("ROIR");
  addDeckName("ROIT");
  addDeckName("ROP");
  addDeckName("ROPR");
  addDeckName("ROPT");
  addDeckName("ROPV");
  addDeckName("RORFE");
  addDeckName("RORFF");
  addDeckName("RORFG");
  addDeckName("RORFR");
  addDeckName("RORFS");
  addDeckName("RORFW");
  addDeckName("RORFX");
  addDeckName("RORFY");
  addDeckName("RORME");
  addDeckName("RORMF");
  addDeckName("RORMG");
  addDeckName("RORMR");
  addDeckName("RORMS");
  addDeckName("RORMW");
  addDeckName("RORMX");
  addDeckName("RORMY");
  addDeckName("ROSAT");
  addDeckName("ROVIS");
  addDeckName("RPPC");
  addDeckName("RPPG");
  addDeckName("RPPO");
  addDeckName("RPPW");
  addDeckName("RPR");
  addDeckName("RPRGZ");
  addDeckName("RPRH");
  addDeckName("RPRP");
  addDeckName("RRPV");
  addDeckName("RRS");
  addDeckName("RRTM");
  addDeckName("RRV");
  addDeckName("RSFT");
  addDeckName("RSIP");
  addDeckName("RTADSFOA");
  addDeckName("RTADSUR");
  addDeckName("RTDCYFOA");
  addDeckName("RTFTTFOA");
  addDeckName("RTFTTSUR");
  addDeckName("RTIPT");
  addDeckName("RTIPTFOA");
  addDeckName("RTIPTHEA");
  addDeckName("RTIPTSUR");
  addDeckName("RTMOBFOA");
  addDeckName("RWDEN");
  addDeckName("RWIP");
  addDeckName("RWIR");
  addDeckName("RWIT");
  addDeckName("RWP");
  addDeckName("RWPR");
  addDeckName("RWPT");
  addDeckName("RWPV");
  addDeckName("RWSAT");
  addDeckName("RWVIS");
  setMatchRegex("R[OGW]?[OIP][EPRT]_.+|RU.+|RTIPF.+|RTIPS.+|RTFTF.+|RTFTS.+|RTFTT.+|RTIPT.+|RTIPF.+|RTIPS.+|RTIP[1-9][0-9]*.+|RTFTT.+|RTFTF.+|RTFTS.+|RTFT[1-9][0-9]*.+|RTADS.+|RTDCY.+");
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
const std::string REGION_PROBE::keywordName = "REGION_PROBE";
const std::string REGION_PROBE::data::itemName = "data";


RESIDNUM::RESIDNUM( ) : ParserKeyword("RESIDNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("RESIDNUM");
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
const std::string RESIDNUM::keywordName = "RESIDNUM";
const std::string RESIDNUM::data::itemName = "data";


RESTART::RESTART( ) : ParserKeyword("RESTART")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RESTART");
  {
     ParserRecord record;
     {
        ParserItem item("ROOTNAME", ParserItem::itype::STRING);
        item.setDefault( std::string("BASE") );
        record.addItem(item);
     }
     {
        ParserItem item("REPORTNUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SAVEFILE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SAVEFILE_FORMAT", ParserItem::itype::STRING);
        item.setDefault( std::string("UNFORMATTED") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RESTART::keywordName = "RESTART";
const std::string RESTART::ROOTNAME::itemName = "ROOTNAME";
const std::string RESTART::ROOTNAME::defaultValue = "BASE";
const std::string RESTART::REPORTNUMBER::itemName = "REPORTNUMBER";
const std::string RESTART::SAVEFILE::itemName = "SAVEFILE";
const std::string RESTART::SAVEFILE_FORMAT::itemName = "SAVEFILE_FORMAT";
const std::string RESTART::SAVEFILE_FORMAT::defaultValue = "UNFORMATTED";


RESVNUM::RESVNUM( ) : ParserKeyword("RESVNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RESVNUM");
  {
     ParserRecord record;
     {
        ParserItem item("NEXT_RES", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RESVNUM::keywordName = "RESVNUM";
const std::string RESVNUM::NEXT_RES::itemName = "NEXT_RES";


RHO::RHO( ) : ParserKeyword("RHO")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RHO");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("Density");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string RHO::keywordName = "RHO";
const std::string RHO::data::itemName = "data";
const double RHO::data::defaultValue = 0.000000;


RIVDEBUG::RIVDEBUG( ) : ParserKeyword("RIVDEBUG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RIVDEBUG");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DEBUG_CONTROL", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVDEBUG::keywordName = "RIVDEBUG";
const std::string RIVDEBUG::RIVER::itemName = "RIVER";
const std::string RIVDEBUG::DEBUG_CONTROL::itemName = "DEBUG_CONTROL";


RIVERSYS::RIVERSYS( ) : ParserKeyword("RIVERSYS")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RIVERSYS");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("EQUATION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("BRANCH_NR", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("BRANCH_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DOWNSTREAM_BC", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVERSYS::keywordName = "RIVERSYS";
const std::string RIVERSYS::RIVER::itemName = "RIVER";
const std::string RIVERSYS::EQUATION::itemName = "EQUATION";
const std::string RIVERSYS::BRANCH_NR::itemName = "BRANCH_NR";
const std::string RIVERSYS::BRANCH_NAME::itemName = "BRANCH_NAME";
const std::string RIVERSYS::DOWNSTREAM_BC::itemName = "DOWNSTREAM_BC";


RIVRDIMS::RIVRDIMS( ) : ParserKeyword("RIVRDIMS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RIVRDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_RIVERS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_REACHES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_BRANCHES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_BLOCKS", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXTBPR", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MXDPTB", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("MXTBGR", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NMDEPT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXDEPT", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("NMMAST", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXMAST", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     {
        ParserItem item("NRATTA", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXRATE", ParserItem::itype::INT);
        item.setDefault( 2 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVRDIMS::keywordName = "RIVRDIMS";
const std::string RIVRDIMS::MAX_RIVERS::itemName = "MAX_RIVERS";
const int RIVRDIMS::MAX_RIVERS::defaultValue = 0;
const std::string RIVRDIMS::MAX_REACHES::itemName = "MAX_REACHES";
const int RIVRDIMS::MAX_REACHES::defaultValue = 1;
const std::string RIVRDIMS::MAX_BRANCHES::itemName = "MAX_BRANCHES";
const int RIVRDIMS::MAX_BRANCHES::defaultValue = 1;
const std::string RIVRDIMS::MAX_BLOCKS::itemName = "MAX_BLOCKS";
const int RIVRDIMS::MAX_BLOCKS::defaultValue = 1;
const std::string RIVRDIMS::MXTBPR::itemName = "MXTBPR";
const int RIVRDIMS::MXTBPR::defaultValue = 1;
const std::string RIVRDIMS::MXDPTB::itemName = "MXDPTB";
const int RIVRDIMS::MXDPTB::defaultValue = 2;
const std::string RIVRDIMS::MXTBGR::itemName = "MXTBGR";
const int RIVRDIMS::MXTBGR::defaultValue = 1;
const std::string RIVRDIMS::NMDEPT::itemName = "NMDEPT";
const int RIVRDIMS::NMDEPT::defaultValue = 0;
const std::string RIVRDIMS::MXDEPT::itemName = "MXDEPT";
const int RIVRDIMS::MXDEPT::defaultValue = 2;
const std::string RIVRDIMS::NMMAST::itemName = "NMMAST";
const int RIVRDIMS::NMMAST::defaultValue = 0;
const std::string RIVRDIMS::MXMAST::itemName = "MXMAST";
const int RIVRDIMS::MXMAST::defaultValue = 2;
const std::string RIVRDIMS::NRATTA::itemName = "NRATTA";
const int RIVRDIMS::NRATTA::defaultValue = 0;
const std::string RIVRDIMS::MXRATE::itemName = "MXRATE";
const int RIVRDIMS::MXRATE::defaultValue = 2;


RIVRPROP::RIVRPROP( ) : ParserKeyword("RIVRPROP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RIVRPROP");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REACH1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REACH2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ROUGHNESS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVRPROP::keywordName = "RIVRPROP";
const std::string RIVRPROP::RIVER::itemName = "RIVER";
const std::string RIVRPROP::REACH1::itemName = "REACH1";
const std::string RIVRPROP::REACH2::itemName = "REACH2";
const std::string RIVRPROP::ROUGHNESS::itemName = "ROUGHNESS";


RIVRXSEC::RIVRXSEC( ) : ParserKeyword("RIVRXSEC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("RIVRDIMS","MXDPTB",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RIVRXSEC");
  {
     ParserRecord record;
     {
        ParserItem item("DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("WET_PERIMTER", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("AREA", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVRXSEC::keywordName = "RIVRXSEC";
const std::string RIVRXSEC::DEPTH::itemName = "DEPTH";
const std::string RIVRXSEC::WET_PERIMTER::itemName = "WET_PERIMTER";
const std::string RIVRXSEC::AREA::itemName = "AREA";


RIVSALT::RIVSALT( ) : ParserKeyword("RIVSALT")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RIVSALT");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SALINITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("BRANCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REACH", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVSALT::keywordName = "RIVSALT";
const std::string RIVSALT::RIVER::itemName = "RIVER";
const std::string RIVSALT::SALINITY::itemName = "SALINITY";
const std::string RIVSALT::BRANCH::itemName = "BRANCH";
const std::string RIVSALT::REACH::itemName = "REACH";


RIVTRACE::RIVTRACE( ) : ParserKeyword("RIVTRACE")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RIVTRACE");
  {
     ParserRecord record;
     {
        ParserItem item("RIVER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TC", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("TCUM", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/LiquidSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("BRANCH", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REACH", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RIVTRACE::keywordName = "RIVTRACE";
const std::string RIVTRACE::RIVER::itemName = "RIVER";
const std::string RIVTRACE::TRACER::itemName = "TRACER";
const std::string RIVTRACE::TC::itemName = "TC";
const std::string RIVTRACE::TCUM::itemName = "TCUM";
const std::string RIVTRACE::BRANCH::itemName = "BRANCH";
const std::string RIVTRACE::REACH::itemName = "REACH";


RKTRMDIR::RKTRMDIR( ) : ParserKeyword("RKTRMDIR")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RKTRMDIR");
}
const std::string RKTRMDIR::keywordName = "RKTRMDIR";


ROCK::ROCK( ) : ParserKeyword("ROCK")
{
  setSizeType(UNKNOWN);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCK");
  {
     ParserRecord record;
     {
        ParserItem item("PREF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.013200) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("COMPRESSIBILITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1/Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCK::keywordName = "ROCK";
const std::string ROCK::PREF::itemName = "PREF";
const double ROCK::PREF::defaultValue = 1.013200;
const std::string ROCK::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";
const double ROCK::COMPRESSIBILITY::defaultValue = 0.000000;


ROCK2D::ROCK2D( ) : ParserKeyword("ROCK2D")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCK2D");
  {
     ParserRecord record;
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PVMULT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCK2D::keywordName = "ROCK2D";
const std::string ROCK2D::PRESSURE::itemName = "PRESSURE";
const std::string ROCK2D::PVMULT::itemName = "PVMULT";


ROCK2DTR::ROCK2DTR( ) : ParserKeyword("ROCK2DTR")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCK2DTR");
  {
     ParserRecord record;
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TRANSMULT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCK2DTR::keywordName = "ROCK2DTR";
const std::string ROCK2DTR::PRESSURE::itemName = "PRESSURE";
const std::string ROCK2DTR::TRANSMULT::itemName = "TRANSMULT";


ROCKCOMP::ROCKCOMP( ) : ParserKeyword("ROCKCOMP")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ROCKCOMP");
  {
     ParserRecord record;
     {
        ParserItem item("HYSTERESIS", ParserItem::itype::STRING);
        item.setDefault( std::string("REVERS") );
        record.addItem(item);
     }
     {
        ParserItem item("NTROCC", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("WATER_COMPACTION", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKCOMP::keywordName = "ROCKCOMP";
const std::string ROCKCOMP::HYSTERESIS::itemName = "HYSTERESIS";
const std::string ROCKCOMP::HYSTERESIS::defaultValue = "REVERS";
const std::string ROCKCOMP::NTROCC::itemName = "NTROCC";
const int ROCKCOMP::NTROCC::defaultValue = 1;
const std::string ROCKCOMP::WATER_COMPACTION::itemName = "WATER_COMPACTION";
const std::string ROCKCOMP::WATER_COMPACTION::defaultValue = "NO";


ROCKFRAC::ROCKFRAC( ) : ParserKeyword("ROCKFRAC")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ROCKFRAC");
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
const std::string ROCKFRAC::keywordName = "ROCKFRAC";
const std::string ROCKFRAC::data::itemName = "data";


ROCKNUM::ROCKNUM( ) : ParserKeyword("ROCKNUM")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("ROCKNUM");
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
const std::string ROCKNUM::keywordName = "ROCKNUM";
const std::string ROCKNUM::data::itemName = "data";


ROCKOPTS::ROCKOPTS( ) : ParserKeyword("ROCKOPTS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("METHOD", ParserItem::itype::STRING);
        item.setDefault( std::string("PRESSURE") );
        record.addItem(item);
     }
     {
        ParserItem item("REF_PRESSURE", ParserItem::itype::STRING);
        item.setDefault( std::string("NOSTORE") );
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("PVTNUM") );
        record.addItem(item);
     }
     {
        ParserItem item("HYST_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("DEFLATION") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKOPTS::keywordName = "ROCKOPTS";
const std::string ROCKOPTS::METHOD::itemName = "METHOD";
const std::string ROCKOPTS::METHOD::defaultValue = "PRESSURE";
const std::string ROCKOPTS::REF_PRESSURE::itemName = "REF_PRESSURE";
const std::string ROCKOPTS::REF_PRESSURE::defaultValue = "NOSTORE";
const std::string ROCKOPTS::TABLE_TYPE::itemName = "TABLE_TYPE";
const std::string ROCKOPTS::TABLE_TYPE::defaultValue = "PVTNUM";
const std::string ROCKOPTS::HYST_TYPE::itemName = "HYST_TYPE";
const std::string ROCKOPTS::HYST_TYPE::defaultValue = "DEFLATION";


ROCKPAMA::ROCKPAMA( ) : ParserKeyword("ROCKPAMA")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKPAMA");
  {
     ParserRecord record;
     {
        ParserItem item("K", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("M", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("G", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("B", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("E1", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("f", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.500000) );
        record.addItem(item);
     }
     {
        ParserItem item("n", ParserItem::itype::DOUBLE);
        item.setDefault( double(3.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("g", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("Bs", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("Es", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKPAMA::keywordName = "ROCKPAMA";
const std::string ROCKPAMA::K::itemName = "K";
const std::string ROCKPAMA::M::itemName = "M";
const std::string ROCKPAMA::G::itemName = "G";
const double ROCKPAMA::G::defaultValue = 0.000000;
const std::string ROCKPAMA::B::itemName = "B";
const double ROCKPAMA::B::defaultValue = 0.000000;
const std::string ROCKPAMA::E1::itemName = "E1";
const double ROCKPAMA::E1::defaultValue = 0.000000;
const std::string ROCKPAMA::f::itemName = "f";
const double ROCKPAMA::f::defaultValue = 0.500000;
const std::string ROCKPAMA::n::itemName = "n";
const double ROCKPAMA::n::defaultValue = 3.000000;
const std::string ROCKPAMA::g::itemName = "g";
const double ROCKPAMA::g::defaultValue = 1.000000;
const std::string ROCKPAMA::Bs::itemName = "Bs";
const double ROCKPAMA::Bs::defaultValue = 0.000000;
const std::string ROCKPAMA::Es::itemName = "Es";
const double ROCKPAMA::Es::defaultValue = 0.000000;


ROCKTAB::ROCKTAB( ) : ParserKeyword("ROCKTAB")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTAB");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKTAB::keywordName = "ROCKTAB";
const std::string ROCKTAB::DATA::itemName = "DATA";


ROCKTABH::ROCKTABH( ) : ParserKeyword("ROCKTABH")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTABH");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKTABH::keywordName = "ROCKTABH";
const std::string ROCKTABH::DATA::itemName = "DATA";


ROCKTABW::ROCKTABW( ) : ParserKeyword("ROCKTABW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTABW");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKTABW::keywordName = "ROCKTABW";
const std::string ROCKTABW::DATA::itemName = "DATA";


ROCKTHSG::ROCKTHSG( ) : ParserKeyword("ROCKTHSG")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTHSG");
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
const std::string ROCKTHSG::keywordName = "ROCKTHSG";
const std::string ROCKTHSG::DATA::itemName = "DATA";


ROCKTSIG::ROCKTSIG( ) : ParserKeyword("ROCKTSIG")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTSIG");
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
const std::string ROCKTSIG::keywordName = "ROCKTSIG";
const std::string ROCKTSIG::DATA::itemName = "DATA";


ROCKV::ROCKV( ) : ParserKeyword("ROCKV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("ROCKV");
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
const std::string ROCKV::keywordName = "ROCKV";
const std::string ROCKV::data::itemName = "data";


ROCKWNOD::ROCKWNOD( ) : ParserKeyword("ROCKWNOD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKWNOD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKWNOD::keywordName = "ROCKWNOD";
const std::string ROCKWNOD::DATA::itemName = "DATA";


RPTCPL::RPTCPL( ) : ParserKeyword("RPTCPL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RPTCPL");
}
const std::string RPTCPL::keywordName = "RPTCPL";


RPTGRID::RPTGRID( ) : ParserKeyword("RPTGRID")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RPTGRID");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTGRID::keywordName = "RPTGRID";
const std::string RPTGRID::DATA::itemName = "DATA";


RPTGRIDL::RPTGRIDL( ) : ParserKeyword("RPTGRIDL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RPTGRIDL");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTGRIDL::keywordName = "RPTGRIDL";
const std::string RPTGRIDL::DATA::itemName = "DATA";


RPTHMD::RPTHMD( ) : ParserKeyword("RPTHMD")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RPTHMD");
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
     addRecord( record );
  }
}
const std::string RPTHMD::keywordName = "RPTHMD";
const std::string RPTHMD::ITEM1::itemName = "ITEM1";
const int RPTHMD::ITEM1::defaultValue = 0;
const std::string RPTHMD::ITEM2::itemName = "ITEM2";
const int RPTHMD::ITEM2::defaultValue = 0;
const std::string RPTHMD::ITEM3::itemName = "ITEM3";
const int RPTHMD::ITEM3::defaultValue = 0;
const std::string RPTHMD::ITEM4::itemName = "ITEM4";
const int RPTHMD::ITEM4::defaultValue = 0;
const std::string RPTHMD::ITEM5::itemName = "ITEM5";
const int RPTHMD::ITEM5::defaultValue = 0;
const std::string RPTHMD::ITEM6::itemName = "ITEM6";
const int RPTHMD::ITEM6::defaultValue = 0;


RPTHMG::RPTHMG( ) : ParserKeyword("RPTHMG")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RPTHMG");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INCLUDE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTHMG::keywordName = "RPTHMG";
const std::string RPTHMG::GROUP::itemName = "GROUP";
const std::string RPTHMG::INCLUDE::itemName = "INCLUDE";


RPTHMW::RPTHMW( ) : ParserKeyword("RPTHMW")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RPTHMW");
  {
     ParserRecord record;
     {
        ParserItem item("GROUP", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("INCLUDE", ParserItem::itype::STRING);
        item.setDefault( std::string("ON") );
        record.addItem(item);
     }
     {
        ParserItem item("INCLUDE_RFT", ParserItem::itype::STRING);
        item.setDefault( std::string("OFF") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTHMW::keywordName = "RPTHMW";
const std::string RPTHMW::GROUP::itemName = "GROUP";
const std::string RPTHMW::INCLUDE::itemName = "INCLUDE";
const std::string RPTHMW::INCLUDE::defaultValue = "ON";
const std::string RPTHMW::INCLUDE_RFT::itemName = "INCLUDE_RFT";
const std::string RPTHMW::INCLUDE_RFT::defaultValue = "OFF";


RPTINIT::RPTINIT( ) : ParserKeyword("RPTINIT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RPTINIT");
  {
     ParserRecord record;
     {
        ParserItem item("MNEMONICS_LIST", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTINIT::keywordName = "RPTINIT";
const std::string RPTINIT::MNEMONICS_LIST::itemName = "MNEMONICS_LIST";


RPTISOL::RPTISOL( ) : ParserKeyword("RPTISOL")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RPTISOL");
}
const std::string RPTISOL::keywordName = "RPTISOL";


RPTONLY::RPTONLY( ) : ParserKeyword("RPTONLY")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RPTONLY");
}
const std::string RPTONLY::keywordName = "RPTONLY";


RPTONLYO::RPTONLYO( ) : ParserKeyword("RPTONLYO")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RPTONLYO");
}
const std::string RPTONLYO::keywordName = "RPTONLYO";


RPTPROPS::RPTPROPS( ) : ParserKeyword("RPTPROPS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RPTPROPS");
  {
     ParserRecord record;
     {
        ParserItem item("mnemonics", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTPROPS::keywordName = "RPTPROPS";
const std::string RPTPROPS::mnemonics::itemName = "mnemonics";


RPTREGS::RPTREGS( ) : ParserKeyword("RPTREGS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("RPTREGS");
  {
     ParserRecord record;
     {
        ParserItem item("MNEMONIC_LIST", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTREGS::keywordName = "RPTREGS";
const std::string RPTREGS::MNEMONIC_LIST::itemName = "MNEMONIC_LIST";


RPTRST::RPTRST( ) : ParserKeyword("RPTRST")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RPTRST");
  {
     ParserRecord record;
     {
        ParserItem item("MNEMONIC_LIST", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTRST::keywordName = "RPTRST";
const std::string RPTRST::MNEMONIC_LIST::itemName = "MNEMONIC_LIST";


RPTRUNSP::RPTRUNSP( ) : ParserKeyword("RPTRUNSP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RPTRUNSP");
}
const std::string RPTRUNSP::keywordName = "RPTRUNSP";


RPTSCHED::RPTSCHED( ) : ParserKeyword("RPTSCHED")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RPTSCHED");
  {
     ParserRecord record;
     {
        ParserItem item("MNEMONIC_LIST", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTSCHED::keywordName = "RPTSCHED";
const std::string RPTSCHED::MNEMONIC_LIST::itemName = "MNEMONIC_LIST";


RPTSMRY::RPTSMRY( ) : ParserKeyword("RPTSMRY")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RPTSMRY");
  {
     ParserRecord record;
     {
        ParserItem item("WRITE", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTSMRY::keywordName = "RPTSMRY";
const std::string RPTSMRY::WRITE::itemName = "WRITE";


RPTSOL::RPTSOL( ) : ParserKeyword("RPTSOL")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RPTSOL");
  {
     ParserRecord record;
     {
        ParserItem item("mnemonics", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTSOL::keywordName = "RPTSOL";
const std::string RPTSOL::mnemonics::itemName = "mnemonics";


RS::RS( ) : ParserKeyword("RS")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RS");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("GasDissolutionFactor");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string RS::keywordName = "RS";
const std::string RS::data::itemName = "data";


RSCONST::RSCONST( ) : ParserKeyword("RSCONST")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RSCONST");
  {
     ParserRecord record;
     {
        ParserItem item("RS", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor");
        record.addItem(item);
     }
     {
        ParserItem item("PB", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RSCONST::keywordName = "RSCONST";
const std::string RSCONST::RS::itemName = "RS";
const std::string RSCONST::PB::itemName = "PB";


RSCONSTT::RSCONSTT( ) : ParserKeyword("RSCONSTT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RSCONSTT");
  {
     ParserRecord record;
     {
        ParserItem item("RS_CONSTT", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasDissolutionFactor");
        record.addItem(item);
     }
     {
        ParserItem item("PB_CONSTT", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RSCONSTT::keywordName = "RSCONSTT";
const std::string RSCONSTT::RS_CONSTT::itemName = "RS_CONSTT";
const std::string RSCONSTT::PB_CONSTT::itemName = "PB_CONSTT";


RSGI::RSGI( ) : ParserKeyword("RSGI")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RSGI");
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
const std::string RSGI::keywordName = "RSGI";
const std::string RSGI::DATA::itemName = "DATA";


RSSPEC::RSSPEC( ) : ParserKeyword("RSSPEC")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RSSPEC");
}
const std::string RSSPEC::keywordName = "RSSPEC";


RSVD::RSVD( ) : ParserKeyword("RSVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RSVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("GasDissolutionFactor");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RSVD::keywordName = "RSVD";
const std::string RSVD::DATA::itemName = "DATA";


RTEMP::RTEMP( ) : ParserKeyword("RTEMP")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RTEMP");
  {
     ParserRecord record;
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.setDefault( double(15.555000) );
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RTEMP::keywordName = "RTEMP";
const std::string RTEMP::TEMP::itemName = "TEMP";
const double RTEMP::TEMP::defaultValue = 15.555000;


RTEMPA::RTEMPA( ) : ParserKeyword("RTEMPA")
{
  setSizeType(SLASH_TERMINATED);
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RTEMPA");
  {
     ParserRecord record;
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.setDefault( double(15.555000) );
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RTEMPA::keywordName = "RTEMPA";
const std::string RTEMPA::TEMP::itemName = "TEMP";
const double RTEMPA::TEMP::defaultValue = 15.555000;


RTEMPVD::RTEMPVD( ) : ParserKeyword("RTEMPVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RTEMPVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RTEMPVD::keywordName = "RTEMPVD";
const std::string RTEMPVD::DATA::itemName = "DATA";


RUNSPEC::RUNSPEC( ) : ParserKeyword("RUNSPEC")
{
  setFixedSize( (size_t) 0);
  clearDeckNames();
  addDeckName("RUNSPEC");
}
const std::string RUNSPEC::keywordName = "RUNSPEC";


RUNSUM::RUNSUM( ) : ParserKeyword("RUNSUM")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RUNSUM");
}
const std::string RUNSUM::keywordName = "RUNSUM";


RV::RV( ) : ParserKeyword("RV")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RV");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string RV::keywordName = "RV";
const std::string RV::data::itemName = "data";


RVCONST::RVCONST( ) : ParserKeyword("RVCONST")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RVCONST");
  {
     ParserRecord record;
     {
        ParserItem item("RV", ParserItem::itype::DOUBLE);
        item.push_backDimension("OilDissolutionFactor");
        record.addItem(item);
     }
     {
        ParserItem item("DEWP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RVCONST::keywordName = "RVCONST";
const std::string RVCONST::RV::itemName = "RV";
const std::string RVCONST::DEWP::itemName = "DEWP";


RVCONSTT::RVCONSTT( ) : ParserKeyword("RVCONSTT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RVCONSTT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RVCONSTT::keywordName = "RVCONSTT";
const std::string RVCONSTT::DATA::itemName = "DATA";


RVGI::RVGI( ) : ParserKeyword("RVGI")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RVGI");
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
const std::string RVGI::keywordName = "RVGI";
const std::string RVGI::DATA::itemName = "DATA";


RVVD::RVVD( ) : ParserKeyword("RVVD")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL",0);
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RVVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("OilDissolutionFactor");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RVVD::keywordName = "RVVD";
const std::string RVVD::DATA::itemName = "DATA";


RWGSALT::RWGSALT( ) : ParserKeyword("RWGSALT")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
  setTableCollection( true );
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RWGSALT");
  {
     ParserRecord record;
     {
        ParserItem item("RESERVOIR_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("SALT_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("OilDissolutionFactor");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string RWGSALT::keywordName = "RWGSALT";
const std::string RWGSALT::RESERVOIR_PRESSURE::itemName = "RESERVOIR_PRESSURE";
const std::string RWGSALT::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";
const std::string RWGSALT::DATA::itemName = "DATA";


}
}
