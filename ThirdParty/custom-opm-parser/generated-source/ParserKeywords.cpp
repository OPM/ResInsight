#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>


namespace Opm {
namespace ParserKeywords {

ACTDIMS::ACTDIMS( ) : ParserKeyword("ACTDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ACTDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAX_ACTION",Opm::SINGLE,2));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ACTION_LINES",Opm::SINGLE,50));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ACTION_LINE_CHARACTERS",Opm::SINGLE,80));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ACTION_COND",Opm::SINGLE,3));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ACTDIMS::keywordName = "ACTDIMS";
const std::string ACTDIMS::MAX_ACTION::itemName = "MAX_ACTION";
const int ACTDIMS::MAX_ACTION::defaultValue = 2;
const std::string ACTDIMS::MAX_ACTION_LINES::itemName = "MAX_ACTION_LINES";
const int ACTDIMS::MAX_ACTION_LINES::defaultValue = 50;
const std::string ACTDIMS::MAX_ACTION_LINE_CHARACTERS::itemName = "MAX_ACTION_LINE_CHARACTERS";
const int ACTDIMS::MAX_ACTION_LINE_CHARACTERS::defaultValue = 80;
const std::string ACTDIMS::MAX_ACTION_COND::itemName = "MAX_ACTION_COND";
const int ACTDIMS::MAX_ACTION_COND::defaultValue = 3;


ACTNUM::ACTNUM( ) : ParserKeyword("ACTNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ACTNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ACTNUM::keywordName = "ACTNUM";
const std::string ACTNUM::data::itemName = "data";


ADD::ADD( ) : ParserKeyword("ADD") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ADD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("field",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("shift",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADD::keywordName = "ADD";
const std::string ADD::field::itemName = "field";
const std::string ADD::shift::itemName = "shift";
const std::string ADD::I1::itemName = "I1";
const std::string ADD::I2::itemName = "I2";
const std::string ADD::J1::itemName = "J1";
const std::string ADD::J2::itemName = "J2";
const std::string ADD::K1::itemName = "K1";
const std::string ADD::K2::itemName = "K2";


ADDREG::ADDREG( ) : ParserKeyword("ADDREG") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ADDREG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("ARRAY",Opm::SINGLE));
        item->setDescription("The 3D array we will update");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SHIFT",Opm::SINGLE,0));
        item->setDescription("The value we will multiply with");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REGION_NUMBER",Opm::SINGLE));
        item->setDescription("The region number we are interested in");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REGION_NAME",Opm::SINGLE,"M"));
        item->setDescription("The name of the region we are interested in");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADDREG::keywordName = "ADDREG";
const std::string ADDREG::ARRAY::itemName = "ARRAY";
const std::string ADDREG::SHIFT::itemName = "SHIFT";
const double ADDREG::SHIFT::defaultValue = 0;
const std::string ADDREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string ADDREG::REGION_NAME::itemName = "REGION_NAME";
const std::string ADDREG::REGION_NAME::defaultValue = "M";


ADSALNOD::ADSALNOD( ) : ParserKeyword("ADSALNOD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ADSALNOD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ADSALNOD::keywordName = "ADSALNOD";
const std::string ADSALNOD::DATA::itemName = "DATA";


ALL::ALL( ) : ParserKeyword("ALL") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ALL");
}
const std::string ALL::keywordName = "ALL";


API::API( ) : ParserKeyword("API") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("API");
}
const std::string API::keywordName = "API";


AQUCON::AQUCON( ) : ParserKeyword("AQUCON") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AQUCON");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("ID",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CONNECT_FACE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRANS_MULT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("TRANS_OPTION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ALLOW_INTERNAL_CELLS",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VEFRAC",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VEFRACP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUCON::keywordName = "AQUCON";
const std::string AQUCON::ID::itemName = "ID";
const std::string AQUCON::I1::itemName = "I1";
const std::string AQUCON::I2::itemName = "I2";
const std::string AQUCON::J1::itemName = "J1";
const std::string AQUCON::J2::itemName = "J2";
const std::string AQUCON::K1::itemName = "K1";
const std::string AQUCON::K2::itemName = "K2";
const std::string AQUCON::CONNECT_FACE::itemName = "CONNECT_FACE";
const std::string AQUCON::TRANS_MULT::itemName = "TRANS_MULT";
const std::string AQUCON::TRANS_OPTION::itemName = "TRANS_OPTION";
const std::string AQUCON::ALLOW_INTERNAL_CELLS::itemName = "ALLOW_INTERNAL_CELLS";
const std::string AQUCON::VEFRAC::itemName = "VEFRAC";
const std::string AQUCON::VEFRACP::itemName = "VEFRACP";


AQUDIMS::AQUDIMS( ) : ParserKeyword("AQUDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("AQUDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MXNAQN",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MXNAQC",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NIFTBL",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NRIFTB",Opm::SINGLE,36));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NANAQU",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NCAMAX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MXNALI",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MXAAQL",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUDIMS::keywordName = "AQUDIMS";
const std::string AQUDIMS::MXNAQN::itemName = "MXNAQN";
const int AQUDIMS::MXNAQN::defaultValue = 1;
const std::string AQUDIMS::MXNAQC::itemName = "MXNAQC";
const int AQUDIMS::MXNAQC::defaultValue = 1;
const std::string AQUDIMS::NIFTBL::itemName = "NIFTBL";
const int AQUDIMS::NIFTBL::defaultValue = 1;
const std::string AQUDIMS::NRIFTB::itemName = "NRIFTB";
const int AQUDIMS::NRIFTB::defaultValue = 36;
const std::string AQUDIMS::NANAQU::itemName = "NANAQU";
const int AQUDIMS::NANAQU::defaultValue = 1;
const std::string AQUDIMS::NCAMAX::itemName = "NCAMAX";
const int AQUDIMS::NCAMAX::defaultValue = 1;
const std::string AQUDIMS::MXNALI::itemName = "MXNALI";
const int AQUDIMS::MXNALI::defaultValue = 0;
const std::string AQUDIMS::MXAAQL::itemName = "MXAAQL";
const int AQUDIMS::MXAAQL::defaultValue = 0;


AQUNUM::AQUNUM( ) : ParserKeyword("AQUNUM") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("AQUNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("AQUIFER_ID",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("CROSS_SECTION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LENGTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PORO",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PERM",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DEPTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("INITIAL_PRESSURE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PVT_TABLE_NUM",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SAT_TABLE_NUM",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string AQUNUM::keywordName = "AQUNUM";
const std::string AQUNUM::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string AQUNUM::I::itemName = "I";
const std::string AQUNUM::J::itemName = "J";
const std::string AQUNUM::K::itemName = "K";
const std::string AQUNUM::CROSS_SECTION::itemName = "CROSS_SECTION";
const std::string AQUNUM::LENGTH::itemName = "LENGTH";
const std::string AQUNUM::PORO::itemName = "PORO";
const std::string AQUNUM::PERM::itemName = "PERM";
const std::string AQUNUM::DEPTH::itemName = "DEPTH";
const std::string AQUNUM::INITIAL_PRESSURE::itemName = "INITIAL_PRESSURE";
const std::string AQUNUM::PVT_TABLE_NUM::itemName = "PVT_TABLE_NUM";
const std::string AQUNUM::SAT_TABLE_NUM::itemName = "SAT_TABLE_NUM";


BLOCK_PROBE::BLOCK_PROBE( ) : ParserKeyword("BLOCK_PROBE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("BAPI");
  addDeckName("BCGC");
  addDeckName("BCSC");
  addDeckName("BCTRA_X");
  addDeckName("BDENG");
  addDeckName("BDENO");
  addDeckName("BDENW");
  addDeckName("BESALPLY");
  addDeckName("BESALSUR");
  addDeckName("BEWV_SAL");
  addDeckName("BEWV_SUR");
  addDeckName("BFLOGI");
  addDeckName("BFLOGJ");
  addDeckName("BFLOGK");
  addDeckName("BFLOOI");
  addDeckName("BFLOOJ");
  addDeckName("BFLOOK");
  addDeckName("BFLOWI");
  addDeckName("BGDEN");
  addDeckName("BGI");
  addDeckName("BGIP");
  addDeckName("BGIPG");
  addDeckName("BGIPL");
  addDeckName("BGKR");
  addDeckName("BGPC");
  addDeckName("BGPR");
  addDeckName("BGPV");
  addDeckName("BGSAT");
  addDeckName("BGSHY");
  addDeckName("BGSTRP");
  addDeckName("BGTPD");
  addDeckName("BGTRP");
  addDeckName("BGVIS");
  addDeckName("BHD");
  addDeckName("BHDF");
  addDeckName("BHDF_X");
  addDeckName("BHD_X");
  addDeckName("BHPV");
  addDeckName("BKRG");
  addDeckName("BKRO");
  addDeckName("BKRW");
  addDeckName("BNIP");
  addDeckName("BNKR");
  addDeckName("BNSAT");
  addDeckName("BODEN");
  addDeckName("BOIP");
  addDeckName("BOIPG");
  addDeckName("BOIPL");
  addDeckName("BOKR");
  addDeckName("BOPV");
  addDeckName("BOSAT");
  addDeckName("BOVIS");
  addDeckName("BPBUB");
  addDeckName("BPDEW");
  addDeckName("BPERMMOD");
  addDeckName("BPORVMOD");
  addDeckName("BPPC");
  addDeckName("BPPG");
  addDeckName("BPPO");
  addDeckName("BPPW");
  addDeckName("BPR");
  addDeckName("BPR_X");
  addDeckName("BRPV");
  addDeckName("BRS");
  addDeckName("BRSSAT");
  addDeckName("BRTM");
  addDeckName("BRV");
  addDeckName("BRVSAT");
  addDeckName("BSCN");
  addDeckName("BSCN_X");
  addDeckName("BSGAS");
  addDeckName("BSIP");
  addDeckName("BSOIL");
  addDeckName("BSTATE");
  addDeckName("BSWAT");
  addDeckName("BTADSALK");
  addDeckName("BTADSFOA");
  addDeckName("BTADSUR");
  addDeckName("BTCASUR");
  addDeckName("BTCNFALK");
  addDeckName("BTCNFANI");
  addDeckName("BTCNFCAT");
  addDeckName("BTCNFFOA");
  addDeckName("BTCNFHEA");
  addDeckName("BTCNFSUR");
  addDeckName("BTCNMFOA");
  addDeckName("BTDCYFOA");
  addDeckName("BTHLFFOA");
  addDeckName("BTIPTFOA");
  addDeckName("BTIPTHEA");
  addDeckName("BTIPTSUR");
  addDeckName("BTMOBFOA");
  addDeckName("BTPADALK");
  addDeckName("BTRADCAT");
  addDeckName("BTSADALK");
  addDeckName("BTSADCAT");
  addDeckName("BTSTMALK");
  addDeckName("BTSTSUR");
  addDeckName("BVELGI");
  addDeckName("BVELGJ");
  addDeckName("BVELGK");
  addDeckName("BVELOI");
  addDeckName("BVELOJ");
  addDeckName("BVELOK");
  addDeckName("BVELWI");
  addDeckName("BVELWJ");
  addDeckName("BVELWK");
  addDeckName("BVGAS");
  addDeckName("BVOIL");
  addDeckName("BVWAT");
  addDeckName("BWDEN");
  addDeckName("BWIP");
  addDeckName("BWKR");
  addDeckName("BWPC");
  addDeckName("BWPR");
  addDeckName("BWPV");
  addDeckName("BWSAT");
  addDeckName("BWSHY");
  addDeckName("BWSMA");
  addDeckName("BWVIS");
  addDeckName("LBCTRA_X");
  addDeckName("LBHDF_X");
  addDeckName("LBHD_X");
  addDeckName("LBPR_X");
  addDeckName("LBSCN_X");
  setMatchRegex("BU.+|BTIPF.+|BTIPS.+|BTCNF.+|BTCNS.+|BTCN[1-9][0-9]*.+|BTIPT.+|BTIPF.+|BTIPS.+|BTIP[1-9][0-9]*.+|BTADS.+|BTDCY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string BLOCK_PROBE::keywordName = "BLOCK_PROBE";
const std::string BLOCK_PROBE::I::itemName = "I";
const std::string BLOCK_PROBE::J::itemName = "J";
const std::string BLOCK_PROBE::K::itemName = "K";


BLOCK_PROBE300::BLOCK_PROBE300( ) : ParserKeyword("BLOCK_PROBE300") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("BTEMP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string BLOCK_PROBE300::keywordName = "BLOCK_PROBE300";
const std::string BLOCK_PROBE300::I::itemName = "I";
const std::string BLOCK_PROBE300::J::itemName = "J";
const std::string BLOCK_PROBE300::K::itemName = "K";


BOX::BOX( ) : ParserKeyword("BOX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("BOX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string BOX::keywordName = "BOX";
const std::string BOX::I1::itemName = "I1";
const std::string BOX::I2::itemName = "I2";
const std::string BOX::J1::itemName = "J1";
const std::string BOX::J2::itemName = "J2";
const std::string BOX::K1::itemName = "K1";
const std::string BOX::K2::itemName = "K2";


COMPDAT::COMPDAT( ) : ParserKeyword("COMPDAT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPDAT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATE",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SAT_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("CONNECTION_TRANSMISSIBILITY_FACTOR",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Viscosity*ReservoirVolume/Time*Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DIAMETER",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("Kh",Opm::SINGLE,-1));
        item->setDescription("");
        item->push_backDimension("Permeability*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SKIN",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("D_FACTOR",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("DIR",Opm::SINGLE,"Z"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PR",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPDAT::keywordName = "COMPDAT";
const std::string COMPDAT::WELL::itemName = "WELL";
const std::string COMPDAT::I::itemName = "I";
const int COMPDAT::I::defaultValue = 0;
const std::string COMPDAT::J::itemName = "J";
const int COMPDAT::J::defaultValue = 0;
const std::string COMPDAT::K1::itemName = "K1";
const std::string COMPDAT::K2::itemName = "K2";
const std::string COMPDAT::STATE::itemName = "STATE";
const std::string COMPDAT::STATE::defaultValue = "OPEN";
const std::string COMPDAT::SAT_TABLE::itemName = "SAT_TABLE";
const int COMPDAT::SAT_TABLE::defaultValue = 0;
const std::string COMPDAT::CONNECTION_TRANSMISSIBILITY_FACTOR::itemName = "CONNECTION_TRANSMISSIBILITY_FACTOR";
const std::string COMPDAT::DIAMETER::itemName = "DIAMETER";
const std::string COMPDAT::Kh::itemName = "Kh";
const double COMPDAT::Kh::defaultValue = -1;
const std::string COMPDAT::SKIN::itemName = "SKIN";
const double COMPDAT::SKIN::defaultValue = 0;
const std::string COMPDAT::D_FACTOR::itemName = "D_FACTOR";
const std::string COMPDAT::DIR::itemName = "DIR";
const std::string COMPDAT::DIR::defaultValue = "Z";
const std::string COMPDAT::PR::itemName = "PR";


COMPLUMP::COMPLUMP( ) : ParserKeyword("COMPLUMP") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPLUMP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("N",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPLUMP::keywordName = "COMPLUMP";
const std::string COMPLUMP::WELL::itemName = "WELL";
const std::string COMPLUMP::I::itemName = "I";
const std::string COMPLUMP::J::itemName = "J";
const std::string COMPLUMP::K1::itemName = "K1";
const std::string COMPLUMP::K2::itemName = "K2";
const std::string COMPLUMP::N::itemName = "N";


COMPORD::COMPORD( ) : ParserKeyword("COMPORD") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPORD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ORDER_TYPE",Opm::SINGLE,"TRACK"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPORD::keywordName = "COMPORD";
const std::string COMPORD::WELL::itemName = "WELL";
const std::string COMPORD::ORDER_TYPE::itemName = "ORDER_TYPE";
const std::string COMPORD::ORDER_TYPE::defaultValue = "TRACK";


COMPS::COMPS( ) : ParserKeyword("COMPS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("COMPS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NUM_COMPS",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPS::keywordName = "COMPS";
const std::string COMPS::NUM_COMPS::itemName = "NUM_COMPS";


COMPSEGS::COMPSEGS( ) : ParserKeyword("COMPSEGS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("COMPSEGS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BRANCH",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DISTANCE_START",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DISTANCE_END",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("DIRECTION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("END_IJK",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("CENTER_DEPTH",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THERMAL_LENGTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SEGMENT_NUMBER",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COMPSEGS::keywordName = "COMPSEGS";
const std::string COMPSEGS::WELL::itemName = "WELL";
const std::string COMPSEGS::I::itemName = "I";
const std::string COMPSEGS::J::itemName = "J";
const std::string COMPSEGS::K::itemName = "K";
const std::string COMPSEGS::BRANCH::itemName = "BRANCH";
const std::string COMPSEGS::DISTANCE_START::itemName = "DISTANCE_START";
const std::string COMPSEGS::DISTANCE_END::itemName = "DISTANCE_END";
const std::string COMPSEGS::DIRECTION::itemName = "DIRECTION";
const std::string COMPSEGS::END_IJK::itemName = "END_IJK";
const std::string COMPSEGS::CENTER_DEPTH::itemName = "CENTER_DEPTH";
const double COMPSEGS::CENTER_DEPTH::defaultValue = 0;
const std::string COMPSEGS::THERMAL_LENGTH::itemName = "THERMAL_LENGTH";
const std::string COMPSEGS::SEGMENT_NUMBER::itemName = "SEGMENT_NUMBER";


CONNECTION_PROBE::CONNECTION_PROBE( ) : ParserKeyword("CONNECTION_PROBE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("CAPI");
  addDeckName("CCFR");
  addDeckName("CCIC");
  addDeckName("CCIT");
  addDeckName("CCPC");
  addDeckName("CCPT");
  addDeckName("CDBF");
  addDeckName("CDFAC");
  addDeckName("CDSF");
  addDeckName("CDSM");
  addDeckName("CDSML");
  addDeckName("CGFR");
  addDeckName("CGFRF");
  addDeckName("CGFRL");
  addDeckName("CGFRS");
  addDeckName("CGFRU");
  addDeckName("CGIR");
  addDeckName("CGIRL");
  addDeckName("CGIT");
  addDeckName("CGITL");
  addDeckName("CGLR");
  addDeckName("CGLRL");
  addDeckName("CGOR");
  addDeckName("CGORL");
  addDeckName("CGPI");
  addDeckName("CGPP");
  addDeckName("CGPR");
  addDeckName("CGPRL");
  addDeckName("CGPT");
  addDeckName("CGPTF");
  addDeckName("CGPTL");
  addDeckName("CGPTS");
  addDeckName("CGQ");
  addDeckName("CLFR");
  addDeckName("CLFRL");
  addDeckName("CLPT");
  addDeckName("CLPTL");
  addDeckName("CNFR");
  addDeckName("CNIT");
  addDeckName("CNPT");
  addDeckName("COFR");
  addDeckName("COFRF");
  addDeckName("COFRL");
  addDeckName("COFRS");
  addDeckName("COFRU");
  addDeckName("COGR");
  addDeckName("COGRL");
  addDeckName("COIT");
  addDeckName("COITL");
  addDeckName("COPI");
  addDeckName("COPP");
  addDeckName("COPR");
  addDeckName("COPRL");
  addDeckName("COPT");
  addDeckName("COPTF");
  addDeckName("COPTL");
  addDeckName("COPTS");
  addDeckName("CPI");
  addDeckName("CSFR");
  addDeckName("CSIC");
  addDeckName("CSIR");
  addDeckName("CSIT");
  addDeckName("CSPC");
  addDeckName("CSPR");
  addDeckName("CSPT");
  addDeckName("CTFAC");
  addDeckName("CTFRALK");
  addDeckName("CTFRANI");
  addDeckName("CTFRCAT");
  addDeckName("CTFRFOA");
  addDeckName("CTFRSUR");
  addDeckName("CTITALK");
  addDeckName("CTITANI");
  addDeckName("CTITCAT");
  addDeckName("CTITFOA");
  addDeckName("CTITSUR");
  addDeckName("CTPTALK");
  addDeckName("CTPTANI");
  addDeckName("CTPTCAT");
  addDeckName("CTPTFOA");
  addDeckName("CTPTSUR");
  addDeckName("CVFR");
  addDeckName("CVFRL");
  addDeckName("CVIR");
  addDeckName("CVIRL");
  addDeckName("CVIT");
  addDeckName("CVITL");
  addDeckName("CVPT");
  addDeckName("CVPTL");
  addDeckName("CWCT");
  addDeckName("CWCTL");
  addDeckName("CWFR");
  addDeckName("CWFRL");
  addDeckName("CWFRU");
  addDeckName("CWGR");
  addDeckName("CWGRL");
  addDeckName("CWIR");
  addDeckName("CWIRL");
  addDeckName("CWIT");
  addDeckName("CWITL");
  addDeckName("CWPI");
  addDeckName("CWPP");
  addDeckName("CWPR");
  addDeckName("CWPRL");
  addDeckName("CWPT");
  addDeckName("CWPTL");
  addDeckName("LCGFRU");
  addDeckName("LCOFRU");
  addDeckName("LCWFRU");
  setMatchRegex("CU.+|CTFR.+|CTPR.+|CTPT.+|CTPC.+|CTIR.+|CTIT.+|CTIC.+|CTFR.+|CTPR.+|CTPT.+|CTPC.+|CTIR.+|CTIT.+|CTIC.+|CTIRF.+|CTIRS.+|CTPRF.+|CTPRS.+|CTITF.+|CTITS.+|CTPTF.+|CTPTS.+|CTICF.+|CTICS.+|CTPCF.+|CTPCS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string CONNECTION_PROBE::keywordName = "CONNECTION_PROBE";
const std::string CONNECTION_PROBE::WELL::itemName = "WELL";
const std::string CONNECTION_PROBE::I::itemName = "I";
const std::string CONNECTION_PROBE::J::itemName = "J";
const std::string CONNECTION_PROBE::K::itemName = "K";


COORD::COORD( ) : ParserKeyword("COORD") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("COORD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string COORD::keywordName = "COORD";
const std::string COORD::data::itemName = "data";


COPY::COPY( ) : ParserKeyword("COPY") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("COPY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("src",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("target",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COPY::keywordName = "COPY";
const std::string COPY::src::itemName = "src";
const std::string COPY::target::itemName = "target";
const std::string COPY::I1::itemName = "I1";
const std::string COPY::I2::itemName = "I2";
const std::string COPY::J1::itemName = "J1";
const std::string COPY::J2::itemName = "J2";
const std::string COPY::K1::itemName = "K1";
const std::string COPY::K2::itemName = "K2";


COPYREG::COPYREG( ) : ParserKeyword("COPYREG") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("COPYREG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("ARRAY",Opm::SINGLE));
        item->setDescription("The 3D array we will update");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TARGET_ARRAY",Opm::SINGLE));
        item->setDescription("The name of the array which will be set");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REGION_NUMBER",Opm::SINGLE));
        item->setDescription("The region number we are interested in");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REGION_NAME",Opm::SINGLE,"M"));
        item->setDescription("The name of the region we are interested in");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string COPYREG::keywordName = "COPYREG";
const std::string COPYREG::ARRAY::itemName = "ARRAY";
const std::string COPYREG::TARGET_ARRAY::itemName = "TARGET_ARRAY";
const std::string COPYREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string COPYREG::REGION_NAME::itemName = "REGION_NAME";
const std::string COPYREG::REGION_NAME::defaultValue = "M";


CPR::CPR( ) : ParserKeyword("CPR") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("CPR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string CPR::keywordName = "CPR";
const std::string CPR::WELL::itemName = "WELL";
const std::string CPR::I::itemName = "I";
const std::string CPR::J::itemName = "J";
const std::string CPR::K::itemName = "K";


CREF::CREF( ) : ParserKeyword("CREF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("CREF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("COMPRESSIBILITY",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string CREF::keywordName = "CREF";
const std::string CREF::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";


CREFS::CREFS( ) : ParserKeyword("CREFS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("CREFS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("COMPRESSIBILITY",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string CREFS::keywordName = "CREFS";
const std::string CREFS::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";


DATE::DATE( ) : ParserKeyword("DATE") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("DATE");
}
const std::string DATE::keywordName = "DATE";


DATES::DATES( ) : ParserKeyword("DATES") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DATES");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("DAY",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("MONTH",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("YEAR",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TIME",Opm::SINGLE,"00:00:00.000"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DATES::keywordName = "DATES";
const std::string DATES::DAY::itemName = "DAY";
const std::string DATES::MONTH::itemName = "MONTH";
const std::string DATES::YEAR::itemName = "YEAR";
const std::string DATES::TIME::itemName = "TIME";
const std::string DATES::TIME::defaultValue = "00:00:00.000";


DENSITY::DENSITY( ) : ParserKeyword("DENSITY") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DENSITY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("OIL",Opm::SINGLE,600));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER",Opm::SINGLE,999.01400000000001));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GAS",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DENSITY::keywordName = "DENSITY";
const std::string DENSITY::OIL::itemName = "OIL";
const double DENSITY::OIL::defaultValue = 600;
const std::string DENSITY::WATER::itemName = "WATER";
const double DENSITY::WATER::defaultValue = 999.014;
const std::string DENSITY::GAS::itemName = "GAS";
const double DENSITY::GAS::defaultValue = 1;


DEPTH::DEPTH( ) : ParserKeyword("DEPTH") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("DEPTH");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DEPTH::keywordName = "DEPTH";
const std::string DEPTH::data::itemName = "data";


DEPTHZ::DEPTHZ( ) : ParserKeyword("DEPTHZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DEPTHZ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DEPTHZ::keywordName = "DEPTHZ";
const std::string DEPTHZ::data::itemName = "data";


DIMENS::DIMENS( ) : ParserKeyword("DIMENS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DIMENS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NY",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NZ",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DIMENS::keywordName = "DIMENS";
const std::string DIMENS::NX::itemName = "NX";
const std::string DIMENS::NY::itemName = "NY";
const std::string DIMENS::NZ::itemName = "NZ";


DISGAS::DISGAS( ) : ParserKeyword("DISGAS") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("DISGAS");
}
const std::string DISGAS::keywordName = "DISGAS";


DREF::DREF( ) : ParserKeyword("DREF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DREF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DENSITY",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DREF::keywordName = "DREF";
const std::string DREF::DENSITY::itemName = "DENSITY";


DREFS::DREFS( ) : ParserKeyword("DREFS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("DREFS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DENSITY",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DREFS::keywordName = "DREFS";
const std::string DREFS::DENSITY::itemName = "DENSITY";


DRSDT::DRSDT( ) : ParserKeyword("DRSDT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRSDT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DRSDT_MAX",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("GasDissolutionFactor/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("Option",Opm::SINGLE,"ALL"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRSDT::keywordName = "DRSDT";
const std::string DRSDT::DRSDT_MAX::itemName = "DRSDT_MAX";
const std::string DRSDT::Option::itemName = "Option";
const std::string DRSDT::Option::defaultValue = "ALL";


DRVDT::DRVDT( ) : ParserKeyword("DRVDT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("DRVDT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DRVDT_MAX",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("OilDissolutionFactor/Time");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string DRVDT::keywordName = "DRVDT";
const std::string DRVDT::DRVDT_MAX::itemName = "DRVDT_MAX";


DX::DX( ) : ParserKeyword("DX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DX::keywordName = "DX";
const std::string DX::data::itemName = "data";


DXV::DXV( ) : ParserKeyword("DXV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DXV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DXV::keywordName = "DXV";
const std::string DXV::data::itemName = "data";


DY::DY( ) : ParserKeyword("DY") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DY::keywordName = "DY";
const std::string DY::data::itemName = "data";


DYV::DYV( ) : ParserKeyword("DYV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DYV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DYV::keywordName = "DYV";
const std::string DYV::data::itemName = "data";


DZ::DZ( ) : ParserKeyword("DZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZ::keywordName = "DZ";
const std::string DZ::data::itemName = "data";


DZV::DZV( ) : ParserKeyword("DZV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("DZV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string DZV::keywordName = "DZV";
const std::string DZV::data::itemName = "data";


ECHO::ECHO( ) : ParserKeyword("ECHO") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ECHO");
}
const std::string ECHO::keywordName = "ECHO";


EDIT::EDIT( ) : ParserKeyword("EDIT") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("EDIT");
}
const std::string EDIT::keywordName = "EDIT";


EDITNNC::EDITNNC( ) : ParserKeyword("EDITNNC") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("EDITNNC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRAN_MULT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SAT_TABLE12",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SAT_TABLE21",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PRESS_TABLE12",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PRESS_TABLE21",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FACE_FLOW12",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FACE_FLOW21",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DIFFM",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EDITNNC::keywordName = "EDITNNC";
const std::string EDITNNC::I1::itemName = "I1";
const std::string EDITNNC::J1::itemName = "J1";
const std::string EDITNNC::K1::itemName = "K1";
const std::string EDITNNC::I2::itemName = "I2";
const std::string EDITNNC::J2::itemName = "J2";
const std::string EDITNNC::K2::itemName = "K2";
const std::string EDITNNC::TRAN_MULT::itemName = "TRAN_MULT";
const std::string EDITNNC::SAT_TABLE12::itemName = "SAT_TABLE12";
const std::string EDITNNC::SAT_TABLE21::itemName = "SAT_TABLE21";
const std::string EDITNNC::PRESS_TABLE12::itemName = "PRESS_TABLE12";
const std::string EDITNNC::PRESS_TABLE21::itemName = "PRESS_TABLE21";
const std::string EDITNNC::FACE_FLOW12::itemName = "FACE_FLOW12";
const std::string EDITNNC::FACE_FLOW21::itemName = "FACE_FLOW21";
const std::string EDITNNC::DIFFM::itemName = "DIFFM";


EHYSTR::EHYSTR( ) : ParserKeyword("EHYSTR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("EHYSTR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("curvature_caplillary_pressure_hyst",Opm::SINGLE,0.10000000000000001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("relative_perm_hyst",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("curvature_param_killough_wetting",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("mod_param_trapped",Opm::SINGLE,0.10000000000000001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("limiting_hyst_flag",Opm::SINGLE,"BOTH"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("shape_cap_press_flag",Opm::SINGLE,"RETR"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("init_fluid_mob_flag",Opm::SINGLE,"DRAIN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("wetting_phase_flag",Opm::SINGLE,"OIL"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("baker_flag_oil",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("baker_flag_gas",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("baker_flag_water",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("threshold_saturation",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EHYSTR::keywordName = "EHYSTR";
const std::string EHYSTR::curvature_caplillary_pressure_hyst::itemName = "curvature_caplillary_pressure_hyst";
const double EHYSTR::curvature_caplillary_pressure_hyst::defaultValue = 0.1;
const std::string EHYSTR::relative_perm_hyst::itemName = "relative_perm_hyst";
const int EHYSTR::relative_perm_hyst::defaultValue = 0;
const std::string EHYSTR::curvature_param_killough_wetting::itemName = "curvature_param_killough_wetting";
const double EHYSTR::curvature_param_killough_wetting::defaultValue = 1;
const std::string EHYSTR::mod_param_trapped::itemName = "mod_param_trapped";
const double EHYSTR::mod_param_trapped::defaultValue = 0.1;
const std::string EHYSTR::limiting_hyst_flag::itemName = "limiting_hyst_flag";
const std::string EHYSTR::limiting_hyst_flag::defaultValue = "BOTH";
const std::string EHYSTR::shape_cap_press_flag::itemName = "shape_cap_press_flag";
const std::string EHYSTR::shape_cap_press_flag::defaultValue = "RETR";
const std::string EHYSTR::init_fluid_mob_flag::itemName = "init_fluid_mob_flag";
const std::string EHYSTR::init_fluid_mob_flag::defaultValue = "DRAIN";
const std::string EHYSTR::wetting_phase_flag::itemName = "wetting_phase_flag";
const std::string EHYSTR::wetting_phase_flag::defaultValue = "OIL";
const std::string EHYSTR::baker_flag_oil::itemName = "baker_flag_oil";
const std::string EHYSTR::baker_flag_oil::defaultValue = "NO";
const std::string EHYSTR::baker_flag_gas::itemName = "baker_flag_gas";
const std::string EHYSTR::baker_flag_gas::defaultValue = "NO";
const std::string EHYSTR::baker_flag_water::itemName = "baker_flag_water";
const std::string EHYSTR::baker_flag_water::defaultValue = "NO";
const std::string EHYSTR::threshold_saturation::itemName = "threshold_saturation";
const double EHYSTR::threshold_saturation::defaultValue = 0;


END::END( ) : ParserKeyword("END") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("END");
}
const std::string END::keywordName = "END";


ENDBOX::ENDBOX( ) : ParserKeyword("ENDBOX") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("ENDBOX");
}
const std::string ENDBOX::keywordName = "ENDBOX";


ENDINC::ENDINC( ) : ParserKeyword("ENDINC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ENDINC");
}
const std::string ENDINC::keywordName = "ENDINC";


ENDNUM::ENDNUM( ) : ParserKeyword("ENDNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("ENDNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ENDNUM::keywordName = "ENDNUM";
const std::string ENDNUM::data::itemName = "data";


ENDPOINT_SPECIFIERS::ENDPOINT_SPECIFIERS( ) : ParserKeyword("ENDPOINT_SPECIFIERS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IKRG");
  addDeckName("IKRGR");
  addDeckName("IKRGRX");
  addDeckName("IKRGRX-");
  addDeckName("IKRGRY");
  addDeckName("IKRGRY-");
  addDeckName("IKRGRZ");
  addDeckName("IKRGRZ-");
  addDeckName("IKRGX");
  addDeckName("IKRGX-");
  addDeckName("IKRGY");
  addDeckName("IKRGY-");
  addDeckName("IKRGZ");
  addDeckName("IKRGZ-");
  addDeckName("IKRO");
  addDeckName("IKRORG");
  addDeckName("IKRORGX");
  addDeckName("IKRORGX-");
  addDeckName("IKRORGY");
  addDeckName("IKRORGY-");
  addDeckName("IKRORGZ");
  addDeckName("IKRORGZ-");
  addDeckName("IKRORW");
  addDeckName("IKRORWX");
  addDeckName("IKRORWX-");
  addDeckName("IKRORWY");
  addDeckName("IKRORWY-");
  addDeckName("IKRORWZ");
  addDeckName("IKRORWZ-");
  addDeckName("IKROX-");
  addDeckName("IKROY-");
  addDeckName("IKROZ-");
  addDeckName("IKRW");
  addDeckName("IKRWR");
  addDeckName("IKRWRX");
  addDeckName("IKRWRX-");
  addDeckName("IKRWRY");
  addDeckName("IKRWRY-");
  addDeckName("IKRWRZ");
  addDeckName("IKRWRZ-");
  addDeckName("IKRWX");
  addDeckName("IKRWX-");
  addDeckName("IKRWY");
  addDeckName("IKRWY-");
  addDeckName("IKRWZ");
  addDeckName("IKRWZ-");
  addDeckName("ISGCR");
  addDeckName("ISGCRX");
  addDeckName("ISGCRX-");
  addDeckName("ISGCRY");
  addDeckName("ISGCRY-");
  addDeckName("ISGCRZ");
  addDeckName("ISGCRZ-");
  addDeckName("ISGU");
  addDeckName("ISGUX");
  addDeckName("ISGUX-");
  addDeckName("ISGUY");
  addDeckName("ISGUY-");
  addDeckName("ISGUZ");
  addDeckName("ISGUZ-");
  addDeckName("ISOGCR");
  addDeckName("ISOGCRX");
  addDeckName("ISOGCRX-");
  addDeckName("ISOGCRY");
  addDeckName("ISOGCRY-");
  addDeckName("ISOGCRZ");
  addDeckName("ISOGCRZ-");
  addDeckName("ISOWCR");
  addDeckName("ISOWCRX");
  addDeckName("ISOWCRX-");
  addDeckName("ISOWCRY");
  addDeckName("ISOWCRY-");
  addDeckName("ISOWCRZ");
  addDeckName("ISOWCRZ-");
  addDeckName("ISWCR");
  addDeckName("ISWCRX");
  addDeckName("ISWCRX-");
  addDeckName("ISWCRZ");
  addDeckName("ISWCRZ-");
  addDeckName("ISWL");
  addDeckName("ISWLX");
  addDeckName("ISWLX-");
  addDeckName("ISWLY");
  addDeckName("ISWLY-");
  addDeckName("ISWLZ");
  addDeckName("ISWLZ-");
  addDeckName("ISWU");
  addDeckName("ISWUX");
  addDeckName("ISWUX-");
  addDeckName("ISWUY");
  addDeckName("ISWUY-");
  addDeckName("ISWUZ");
  addDeckName("ISWUZ-");
  addDeckName("KRG");
  addDeckName("KRGR");
  addDeckName("KRGRX");
  addDeckName("KRGRX-");
  addDeckName("KRGRY");
  addDeckName("KRGRY-");
  addDeckName("KRGRZ");
  addDeckName("KRGRZ-");
  addDeckName("KRGX");
  addDeckName("KRGX-");
  addDeckName("KRGY");
  addDeckName("KRGY-");
  addDeckName("KRGZ");
  addDeckName("KRGZ-");
  addDeckName("KRO");
  addDeckName("KRORG");
  addDeckName("KRORGX");
  addDeckName("KRORGX-");
  addDeckName("KRORGY");
  addDeckName("KRORGY-");
  addDeckName("KRORGZ");
  addDeckName("KRORGZ-");
  addDeckName("KRORW");
  addDeckName("KRORWX");
  addDeckName("KRORWX-");
  addDeckName("KRORWY");
  addDeckName("KRORWY-");
  addDeckName("KRORWZ");
  addDeckName("KRORWZ-");
  addDeckName("KROX");
  addDeckName("KROX-");
  addDeckName("KROY");
  addDeckName("KROY-");
  addDeckName("KROZ");
  addDeckName("KROZ-");
  addDeckName("KRW");
  addDeckName("KRWR");
  addDeckName("KRWRX");
  addDeckName("KRWRX-");
  addDeckName("KRWRY");
  addDeckName("KRWRY-");
  addDeckName("KRWRZ");
  addDeckName("KRWRZ-");
  addDeckName("KRWX");
  addDeckName("KRWX-");
  addDeckName("KRWY");
  addDeckName("KRWY-");
  addDeckName("KRWZ");
  addDeckName("KRWZ-");
  addDeckName("SGCR");
  addDeckName("SGCRX");
  addDeckName("SGCRX-");
  addDeckName("SGCRY");
  addDeckName("SGCRY-");
  addDeckName("SGCRZ");
  addDeckName("SGCRZ-");
  addDeckName("SGU");
  addDeckName("SGUX");
  addDeckName("SGUX-");
  addDeckName("SGUY");
  addDeckName("SGUY-");
  addDeckName("SGUZ");
  addDeckName("SGUZ-");
  addDeckName("SOGCR");
  addDeckName("SOGCRX");
  addDeckName("SOGCRX-");
  addDeckName("SOGCRY");
  addDeckName("SOGCRY-");
  addDeckName("SOGCRZ");
  addDeckName("SOGCRZ-");
  addDeckName("SOWCR");
  addDeckName("SOWCRX");
  addDeckName("SOWCRX-");
  addDeckName("SOWCRY");
  addDeckName("SOWCRY-");
  addDeckName("SOWCRZ");
  addDeckName("SOWCRZ-");
  addDeckName("SWCR");
  addDeckName("SWCRX");
  addDeckName("SWCRX-");
  addDeckName("SWCRY");
  addDeckName("SWCRY-");
  addDeckName("SWCRZ");
  addDeckName("SWCRZ-");
  addDeckName("SWL");
  addDeckName("SWLX");
  addDeckName("SWLX-");
  addDeckName("SWLY");
  addDeckName("SWLY-");
  addDeckName("SWLZ");
  addDeckName("SWLZ-");
  addDeckName("SWU");
  addDeckName("SWUX");
  addDeckName("SWUX-");
  addDeckName("SWUY");
  addDeckName("SWUY-");
  addDeckName("SWUZ");
  addDeckName("SWUZ-");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ENDPOINT_SPECIFIERS::keywordName = "ENDPOINT_SPECIFIERS";
const std::string ENDPOINT_SPECIFIERS::data::itemName = "data";


ENDSCALE::ENDSCALE( ) : ParserKeyword("ENDSCALE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ENDSCALE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("DIRECT",Opm::SINGLE,"NODIR"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("IRREVERS",Opm::SINGLE,"REVER"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_TABLES",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_NODES",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("COMB_MODE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENDSCALE::keywordName = "ENDSCALE";
const std::string ENDSCALE::DIRECT::itemName = "DIRECT";
const std::string ENDSCALE::DIRECT::defaultValue = "NODIR";
const std::string ENDSCALE::IRREVERS::itemName = "IRREVERS";
const std::string ENDSCALE::IRREVERS::defaultValue = "REVER";
const std::string ENDSCALE::NUM_TABLES::itemName = "NUM_TABLES";
const int ENDSCALE::NUM_TABLES::defaultValue = 1;
const std::string ENDSCALE::NUM_NODES::itemName = "NUM_NODES";
const int ENDSCALE::NUM_NODES::defaultValue = 20;
const std::string ENDSCALE::COMB_MODE::itemName = "COMB_MODE";
const int ENDSCALE::COMB_MODE::defaultValue = 0;


ENDSKIP::ENDSKIP( ) : ParserKeyword("ENDSKIP") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("ENDSKIP");
}
const std::string ENDSKIP::keywordName = "ENDSKIP";


ENKRVD::ENKRVD( ) : ParserKeyword("ENKRVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ENDSCALE","NUM_TABLES");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENKRVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL,-1));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENKRVD::keywordName = "ENKRVD";
const std::string ENKRVD::DATA::itemName = "DATA";
const double ENKRVD::DATA::defaultValue = -1;


ENPTVD::ENPTVD( ) : ParserKeyword("ENPTVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ENDSCALE","NUM_TABLES");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENPTVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL,-1));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENPTVD::keywordName = "ENPTVD";
const std::string ENPTVD::DATA::itemName = "DATA";
const double ENPTVD::DATA::defaultValue = -1;


EQLDIMS::EQLDIMS( ) : ParserKeyword("EQLDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("EQLDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NTEQUL",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("DEPTH_NODES_P",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("DEPTH_NODES_TAB",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTTRVD",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NSTRVD",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQLDIMS::keywordName = "EQLDIMS";
const std::string EQLDIMS::NTEQUL::itemName = "NTEQUL";
const int EQLDIMS::NTEQUL::defaultValue = 1;
const std::string EQLDIMS::DEPTH_NODES_P::itemName = "DEPTH_NODES_P";
const int EQLDIMS::DEPTH_NODES_P::defaultValue = 100;
const std::string EQLDIMS::DEPTH_NODES_TAB::itemName = "DEPTH_NODES_TAB";
const int EQLDIMS::DEPTH_NODES_TAB::defaultValue = 20;
const std::string EQLDIMS::NTTRVD::itemName = "NTTRVD";
const int EQLDIMS::NTTRVD::defaultValue = 1;
const std::string EQLDIMS::NSTRVD::itemName = "NSTRVD";
const int EQLDIMS::NSTRVD::defaultValue = 20;


EQLNUM::EQLNUM( ) : ParserKeyword("EQLNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("EQLNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string EQLNUM::keywordName = "EQLNUM";
const std::string EQLNUM::data::itemName = "data";


EQLOPTS::EQLOPTS( ) : ParserKeyword("EQLOPTS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("EQLOPTS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("OPTION1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OPTION2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OPTION3",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OPTION4",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQLOPTS::keywordName = "EQLOPTS";
const std::string EQLOPTS::OPTION1::itemName = "OPTION1";
const std::string EQLOPTS::OPTION2::itemName = "OPTION2";
const std::string EQLOPTS::OPTION3::itemName = "OPTION3";
const std::string EQLOPTS::OPTION4::itemName = "OPTION4";


EQUALREG::EQUALREG( ) : ParserKeyword("EQUALREG") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUALREG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("ARRAY",Opm::SINGLE));
        item->setDescription("The 3D array we will update");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VALUE",Opm::SINGLE,0));
        item->setDescription("The value we will assign");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REGION_NUMBER",Opm::SINGLE));
        item->setDescription("The region number we are interested in");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REGION_NAME",Opm::SINGLE,"M"));
        item->setDescription("The name of the region we are interested in");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQUALREG::keywordName = "EQUALREG";
const std::string EQUALREG::ARRAY::itemName = "ARRAY";
const std::string EQUALREG::VALUE::itemName = "VALUE";
const double EQUALREG::VALUE::defaultValue = 0;
const std::string EQUALREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string EQUALREG::REGION_NAME::itemName = "REGION_NAME";
const std::string EQUALREG::REGION_NAME::defaultValue = "M";


EQUALS::EQUALS( ) : ParserKeyword("EQUALS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUALS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("field",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("value",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQUALS::keywordName = "EQUALS";
const std::string EQUALS::field::itemName = "field";
const std::string EQUALS::value::itemName = "value";
const std::string EQUALS::I1::itemName = "I1";
const std::string EQUALS::I2::itemName = "I2";
const std::string EQUALS::J1::itemName = "J1";
const std::string EQUALS::J2::itemName = "J2";
const std::string EQUALS::K1::itemName = "K1";
const std::string EQUALS::K2::itemName = "K2";


EQUIL::EQUIL( ) : ParserKeyword("EQUIL") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("The EQUIL item is used when equilibrationg the model. The item should consist of one record for each PVT region");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUIL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATUM_DEPTH",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATUM_PRESSURE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OWC",Opm::SINGLE,0));
        item->setDescription("The OWC item is depth of the OIL Water contact. This should ...");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PC_OWC",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GOC",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PC_GOC",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BLACK_OIL_INIT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BLACK_OIL_INIT_WG",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("OIP_INIT",Opm::SINGLE,-5));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQUIL::keywordName = "EQUIL";
const std::string EQUIL::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const double EQUIL::DATUM_DEPTH::defaultValue = 0;
const std::string EQUIL::DATUM_PRESSURE::itemName = "DATUM_PRESSURE";
const std::string EQUIL::OWC::itemName = "OWC";
const double EQUIL::OWC::defaultValue = 0;
const std::string EQUIL::PC_OWC::itemName = "PC_OWC";
const double EQUIL::PC_OWC::defaultValue = 0;
const std::string EQUIL::GOC::itemName = "GOC";
const double EQUIL::GOC::defaultValue = 0;
const std::string EQUIL::PC_GOC::itemName = "PC_GOC";
const double EQUIL::PC_GOC::defaultValue = 0;
const std::string EQUIL::BLACK_OIL_INIT::itemName = "BLACK_OIL_INIT";
const int EQUIL::BLACK_OIL_INIT::defaultValue = 0;
const std::string EQUIL::BLACK_OIL_INIT_WG::itemName = "BLACK_OIL_INIT_WG";
const int EQUIL::BLACK_OIL_INIT_WG::defaultValue = 0;
const std::string EQUIL::OIP_INIT::itemName = "OIP_INIT";
const int EQUIL::OIP_INIT::defaultValue = -5;


EXCEL::EXCEL( ) : ParserKeyword("EXCEL") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("EXCEL");
}
const std::string EXCEL::keywordName = "EXCEL";


EXTRAPMS::EXTRAPMS( ) : ParserKeyword("EXTRAPMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("EXTRAPMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("LEVEL",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string EXTRAPMS::keywordName = "EXTRAPMS";
const std::string EXTRAPMS::LEVEL::itemName = "LEVEL";
const int EXTRAPMS::LEVEL::defaultValue = 0;


FAULTDIM::FAULTDIM( ) : ParserKeyword("FAULTDIM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FAULTDIM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MFSEGS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string FAULTDIM::keywordName = "FAULTDIM";
const std::string FAULTDIM::MFSEGS::itemName = "MFSEGS";
const int FAULTDIM::MFSEGS::defaultValue = 0;


FAULTS::FAULTS( ) : ParserKeyword("FAULTS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FAULTS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("NAME",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IX1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IX2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IY1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IY2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IZ1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IZ2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FACE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string FAULTS::keywordName = "FAULTS";
const std::string FAULTS::NAME::itemName = "NAME";
const std::string FAULTS::IX1::itemName = "IX1";
const std::string FAULTS::IX2::itemName = "IX2";
const std::string FAULTS::IY1::itemName = "IY1";
const std::string FAULTS::IY2::itemName = "IY2";
const std::string FAULTS::IZ1::itemName = "IZ1";
const std::string FAULTS::IZ2::itemName = "IZ2";
const std::string FAULTS::FACE::itemName = "FACE";


FIELD::FIELD( ) : ParserKeyword("FIELD") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FIELD");
}
const std::string FIELD::keywordName = "FIELD";


FIELD_PROBE::FIELD_PROBE( ) : ParserKeyword("FIELD_PROBE") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("FAPI");
  addDeckName("FAQR");
  addDeckName("FAQRG");
  addDeckName("FAQT");
  addDeckName("FAQTG");
  addDeckName("FCAD");
  addDeckName("FCGC");
  addDeckName("FCIC");
  addDeckName("FCIP");
  addDeckName("FCIR");
  addDeckName("FCIT");
  addDeckName("FCPC");
  addDeckName("FCPR");
  addDeckName("FCPT");
  addDeckName("FCSC");
  addDeckName("FEPR");
  addDeckName("FEPT");
  addDeckName("FGCR");
  addDeckName("FGCT");
  addDeckName("FGDC");
  addDeckName("FGDCQ");
  addDeckName("FGDEN");
  addDeckName("FGIMR");
  addDeckName("FGIMT");
  addDeckName("FGIP");
  addDeckName("FGIPG");
  addDeckName("FGIPL");
  addDeckName("FGIR");
  addDeckName("FGIRH");
  addDeckName("FGIRT");
  addDeckName("FGIT");
  addDeckName("FGITH");
  addDeckName("FGLIR");
  addDeckName("FGLR");
  addDeckName("FGLRH");
  addDeckName("FGOR");
  addDeckName("FGORH");
  addDeckName("FGPI");
  addDeckName("FGPI2");
  addDeckName("FGPP");
  addDeckName("FGPP2");
  addDeckName("FGPPF");
  addDeckName("FGPPF2");
  addDeckName("FGPPS");
  addDeckName("FGPPS2");
  addDeckName("FGPR");
  addDeckName("FGPRF");
  addDeckName("FGPRH");
  addDeckName("FGPRS");
  addDeckName("FGPRT");
  addDeckName("FGPT");
  addDeckName("FGPTF");
  addDeckName("FGPTH");
  addDeckName("FGPTS");
  addDeckName("FGPV");
  addDeckName("FGQ");
  addDeckName("FGSAT");
  addDeckName("FGSPR");
  addDeckName("FGSR");
  addDeckName("FGSRL");
  addDeckName("FGSRU");
  addDeckName("FGSSP");
  addDeckName("FGST");
  addDeckName("FGSTP");
  addDeckName("FGVIS");
  addDeckName("FHPV");
  addDeckName("FJPR");
  addDeckName("FJPRH");
  addDeckName("FJPRT");
  addDeckName("FJPT");
  addDeckName("FJPTH");
  addDeckName("FLPR");
  addDeckName("FLPRH");
  addDeckName("FLPRT");
  addDeckName("FLPT");
  addDeckName("FLPTH");
  addDeckName("FMCTG");
  addDeckName("FMCTP");
  addDeckName("FMCTW");
  addDeckName("FMIR");
  addDeckName("FMIT");
  addDeckName("FMPR");
  addDeckName("FMPT");
  addDeckName("FMWDR");
  addDeckName("FMWDT");
  addDeckName("FMWIA");
  addDeckName("FMWIG");
  addDeckName("FMWIN");
  addDeckName("FMWIP");
  addDeckName("FMWIS");
  addDeckName("FMWIT");
  addDeckName("FMWIU");
  addDeckName("FMWIV");
  addDeckName("FMWPA");
  addDeckName("FMWPG");
  addDeckName("FMWPL");
  addDeckName("FMWPO");
  addDeckName("FMWPP");
  addDeckName("FMWPR");
  addDeckName("FMWPS");
  addDeckName("FMWPT");
  addDeckName("FMWPU");
  addDeckName("FMWPV");
  addDeckName("FMWWO");
  addDeckName("FMWWT");
  addDeckName("FNIP");
  addDeckName("FNIR");
  addDeckName("FNIT");
  addDeckName("FNPR");
  addDeckName("FNPT");
  addDeckName("FNQR");
  addDeckName("FNQT");
  addDeckName("FODEN");
  addDeckName("FOE");
  addDeckName("FOEIG");
  addDeckName("FOEIW");
  addDeckName("FOEW");
  addDeckName("FOEWG");
  addDeckName("FOEWW");
  addDeckName("FOGR");
  addDeckName("FOGRH");
  addDeckName("FOIP");
  addDeckName("FOIPG");
  addDeckName("FOIPL");
  addDeckName("FOIR");
  addDeckName("FOIRH");
  addDeckName("FOIRT");
  addDeckName("FOIT");
  addDeckName("FOITH");
  addDeckName("FOPI");
  addDeckName("FOPI2");
  addDeckName("FOPP");
  addDeckName("FOPP2");
  addDeckName("FOPR");
  addDeckName("FOPRF");
  addDeckName("FOPRH");
  addDeckName("FOPRS");
  addDeckName("FOPRT");
  addDeckName("FOPT");
  addDeckName("FOPTF");
  addDeckName("FOPTH");
  addDeckName("FOPTS");
  addDeckName("FOPV");
  addDeckName("FORFE");
  addDeckName("FORFF");
  addDeckName("FORFG");
  addDeckName("FORFR");
  addDeckName("FORFS");
  addDeckName("FORFW");
  addDeckName("FORFX");
  addDeckName("FORFY");
  addDeckName("FORME");
  addDeckName("FORMF");
  addDeckName("FORMG");
  addDeckName("FORMR");
  addDeckName("FORMS");
  addDeckName("FORMW");
  addDeckName("FORMX");
  addDeckName("FORMY");
  addDeckName("FOSAT");
  addDeckName("FOSPR");
  addDeckName("FOSRL");
  addDeckName("FOSRU");
  addDeckName("FOSSP");
  addDeckName("FOSTP");
  addDeckName("FOVIS");
  addDeckName("FPPC");
  addDeckName("FPPG");
  addDeckName("FPPO");
  addDeckName("FPPW");
  addDeckName("FPR");
  addDeckName("FPRGZ");
  addDeckName("FPRH");
  addDeckName("FPRP");
  addDeckName("FRPV");
  addDeckName("FRS");
  addDeckName("FRTM");
  addDeckName("FRV");
  addDeckName("FSGR");
  addDeckName("FSGT");
  addDeckName("FSIC");
  addDeckName("FSIP");
  addDeckName("FSIR");
  addDeckName("FSIT");
  addDeckName("FSPC");
  addDeckName("FSPR");
  addDeckName("FSPT");
  addDeckName("FTADSFOA");
  addDeckName("FTADSUR");
  addDeckName("FTDCYFOA");
  addDeckName("FTICHEA");
  addDeckName("FTIPTFOA");
  addDeckName("FTIPTHEA");
  addDeckName("FTIPTSUR");
  addDeckName("FTIRALK");
  addDeckName("FTIRANI");
  addDeckName("FTIRCAT");
  addDeckName("FTIRFOA");
  addDeckName("FTIRHEA");
  addDeckName("FTIRSUR");
  addDeckName("FTITALK");
  addDeckName("FTITANI");
  addDeckName("FTITCAT");
  addDeckName("FTITFOA");
  addDeckName("FTITHEA");
  addDeckName("FTITSUR");
  addDeckName("FTMOBFOA");
  addDeckName("FTPCHEA");
  addDeckName("FTPRALK");
  addDeckName("FTPRANI");
  addDeckName("FTPRCAT");
  addDeckName("FTPRFOA");
  addDeckName("FTPRHEA");
  addDeckName("FTPRSUR");
  addDeckName("FTPTALK");
  addDeckName("FTPTANI");
  addDeckName("FTPTCAT");
  addDeckName("FTPTFOA");
  addDeckName("FTPTHEA");
  addDeckName("FTPTSUR");
  addDeckName("FVIR");
  addDeckName("FVIRT");
  addDeckName("FVIT");
  addDeckName("FVPR");
  addDeckName("FVPRT");
  addDeckName("FVPT");
  addDeckName("FWCT");
  addDeckName("FWCTH");
  addDeckName("FWDEN");
  addDeckName("FWGR");
  addDeckName("FWGRH");
  addDeckName("FWIP");
  addDeckName("FWIR");
  addDeckName("FWIRH");
  addDeckName("FWIRT");
  addDeckName("FWIT");
  addDeckName("FWITH");
  addDeckName("FWPI");
  addDeckName("FWPI2");
  addDeckName("FWPIR");
  addDeckName("FWPP");
  addDeckName("FWPP2");
  addDeckName("FWPR");
  addDeckName("FWPRH");
  addDeckName("FWPRT");
  addDeckName("FWPT");
  addDeckName("FWPTH");
  addDeckName("FWPV");
  addDeckName("FWSAT");
  addDeckName("FWSPR");
  addDeckName("FWSRL");
  addDeckName("FWSRU");
  addDeckName("FWSSP");
  addDeckName("FWSTP");
  addDeckName("FWVIS");
  addDeckName("PSSPR");
  addDeckName("PSSSC");
  addDeckName("PSSSG");
  addDeckName("PSSSO");
  addDeckName("PSSSW");
  setMatchRegex("FU.+|FTPR.+|FTPT.+|FTPC.+|FTIR.+|FTIT.+|FTIC.+|FTIPT.+|FTIPF.+|FTIPS|FTIP[1-9][0-9]*.+|FTPR.+|FTPT.+|FTPC.+|FTIR.+|FTIT.+|FTIC.+|FTADS.+|FTDCY.+|FTIRF.+|FTIRS.+|FTPRF.+|FTPRS.+|FTITF.+|FTITS.+|FTPTF.+|FTPTS.+|FTICF.+|FTICS.+|FTPCF.+|FTPCS.+");
}
const std::string FIELD_PROBE::keywordName = "FIELD_PROBE";


FILLEPS::FILLEPS( ) : ParserKeyword("FILLEPS") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FILLEPS");
}
const std::string FILLEPS::keywordName = "FILLEPS";


FIPNUM::FIPNUM( ) : ParserKeyword("FIPNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("FIPNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string FIPNUM::keywordName = "FIPNUM";
const std::string FIPNUM::data::itemName = "data";


FLUXNUM::FLUXNUM( ) : ParserKeyword("FLUXNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FLUXNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string FLUXNUM::keywordName = "FLUXNUM";
const std::string FLUXNUM::data::itemName = "data";


FMTIN::FMTIN( ) : ParserKeyword("FMTIN") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTIN");
}
const std::string FMTIN::keywordName = "FMTIN";


FMTOUT::FMTOUT( ) : ParserKeyword("FMTOUT") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTOUT");
}
const std::string FMTOUT::keywordName = "FMTOUT";


FULLIMP::FULLIMP( ) : ParserKeyword("FULLIMP") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FULLIMP");
}
const std::string FULLIMP::keywordName = "FULLIMP";


GAS::GAS( ) : ParserKeyword("GAS") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GAS");
}
const std::string GAS::keywordName = "GAS";


GASVISCT::GASVISCT( ) : ParserKeyword("GASVISCT") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("GASVISCT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GASVISCT::keywordName = "GASVISCT";
const std::string GASVISCT::DATA::itemName = "DATA";


GCOMPIDX::GCOMPIDX( ) : ParserKeyword("GCOMPIDX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GCOMPIDX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("GAS_COMPONENT_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCOMPIDX::keywordName = "GCOMPIDX";
const std::string GCOMPIDX::GAS_COMPONENT_INDEX::itemName = "GAS_COMPONENT_INDEX";


GCONINJE::GCONINJE( ) : ParserKeyword("GCONINJE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONINJE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PHASE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CONTROL_MODE",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_TARGET",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RESV_TARGET",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("ReservoirVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("REINJ_TARGET",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VOIDAGE_TARGET",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FREE",Opm::SINGLE,"YES"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GUIDE_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GUIDE_DEF",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REINJECT_GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("VOIDAGE_GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WETGAS_TARGET",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("GasSurfaceVolume/Time");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONINJE::keywordName = "GCONINJE";
const std::string GCONINJE::GROUP::itemName = "GROUP";
const std::string GCONINJE::PHASE::itemName = "PHASE";
const std::string GCONINJE::CONTROL_MODE::itemName = "CONTROL_MODE";
const std::string GCONINJE::CONTROL_MODE::defaultValue = "NONE";
const std::string GCONINJE::SURFACE_TARGET::itemName = "SURFACE_TARGET";
const double GCONINJE::SURFACE_TARGET::defaultValue = 0;
const std::string GCONINJE::RESV_TARGET::itemName = "RESV_TARGET";
const double GCONINJE::RESV_TARGET::defaultValue = 0;
const std::string GCONINJE::REINJ_TARGET::itemName = "REINJ_TARGET";
const double GCONINJE::REINJ_TARGET::defaultValue = 0;
const std::string GCONINJE::VOIDAGE_TARGET::itemName = "VOIDAGE_TARGET";
const double GCONINJE::VOIDAGE_TARGET::defaultValue = 0;
const std::string GCONINJE::FREE::itemName = "FREE";
const std::string GCONINJE::FREE::defaultValue = "YES";
const std::string GCONINJE::GUIDE_FRACTION::itemName = "GUIDE_FRACTION";
const double GCONINJE::GUIDE_FRACTION::defaultValue = 0;
const std::string GCONINJE::GUIDE_DEF::itemName = "GUIDE_DEF";
const std::string GCONINJE::REINJECT_GROUP::itemName = "REINJECT_GROUP";
const std::string GCONINJE::VOIDAGE_GROUP::itemName = "VOIDAGE_GROUP";
const std::string GCONINJE::WETGAS_TARGET::itemName = "WETGAS_TARGET";


GCONPROD::GCONPROD( ) : ParserKeyword("GCONPROD") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GCONPROD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CONTROL_MODE",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_TARGET",Opm::SINGLE,-9.9899999999999998e+102));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_TARGET",Opm::SINGLE,-9.9899999999999998e+102));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GAS_TARGET",Opm::SINGLE,-9.9899999999999998e+102));
        item->setDescription("");
        item->push_backDimension("GasSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LIQUID_TARGET",Opm::SINGLE,-9.9899999999999998e+102));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("EXCEED_PROC",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("RESPOND_TO_PARENT",Opm::SINGLE,"YES"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GUIDE_RATE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GUIDE_RATE_DEF",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("WATER_EXCEED_PROCEDURE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GAS_EXCEED_PROCEDURE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("LIQUID_EXCEED_PROCEDURE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RESERVOIR_FLUID_TARGET",Opm::SINGLE,-9.9899999999999998e+102));
        item->setDescription("");
        item->push_backDimension("ReservoirVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RESERVOIR_VOLUME_BALANCE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GCONPROD::keywordName = "GCONPROD";
const std::string GCONPROD::GROUP::itemName = "GROUP";
const std::string GCONPROD::CONTROL_MODE::itemName = "CONTROL_MODE";
const std::string GCONPROD::CONTROL_MODE::defaultValue = "NONE";
const std::string GCONPROD::OIL_TARGET::itemName = "OIL_TARGET";
const double GCONPROD::OIL_TARGET::defaultValue = -9.99e+102;
const std::string GCONPROD::WATER_TARGET::itemName = "WATER_TARGET";
const double GCONPROD::WATER_TARGET::defaultValue = -9.99e+102;
const std::string GCONPROD::GAS_TARGET::itemName = "GAS_TARGET";
const double GCONPROD::GAS_TARGET::defaultValue = -9.99e+102;
const std::string GCONPROD::LIQUID_TARGET::itemName = "LIQUID_TARGET";
const double GCONPROD::LIQUID_TARGET::defaultValue = -9.99e+102;
const std::string GCONPROD::EXCEED_PROC::itemName = "EXCEED_PROC";
const std::string GCONPROD::EXCEED_PROC::defaultValue = "NONE";
const std::string GCONPROD::RESPOND_TO_PARENT::itemName = "RESPOND_TO_PARENT";
const std::string GCONPROD::RESPOND_TO_PARENT::defaultValue = "YES";
const std::string GCONPROD::GUIDE_RATE::itemName = "GUIDE_RATE";
const std::string GCONPROD::GUIDE_RATE_DEF::itemName = "GUIDE_RATE_DEF";
const std::string GCONPROD::WATER_EXCEED_PROCEDURE::itemName = "WATER_EXCEED_PROCEDURE";
const std::string GCONPROD::GAS_EXCEED_PROCEDURE::itemName = "GAS_EXCEED_PROCEDURE";
const std::string GCONPROD::LIQUID_EXCEED_PROCEDURE::itemName = "LIQUID_EXCEED_PROCEDURE";
const std::string GCONPROD::RESERVOIR_FLUID_TARGET::itemName = "RESERVOIR_FLUID_TARGET";
const double GCONPROD::RESERVOIR_FLUID_TARGET::defaultValue = -9.99e+102;
const std::string GCONPROD::RESERVOIR_VOLUME_BALANCE::itemName = "RESERVOIR_VOLUME_BALANCE";


GDORIENT::GDORIENT( ) : ParserKeyword("GDORIENT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GDORIENT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("I",Opm::SINGLE,"INC"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("J",Opm::SINGLE,"INC"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("K",Opm::SINGLE,"INC"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("Z",Opm::SINGLE,"DOWN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("HAND",Opm::SINGLE,"RIGHT"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GDORIENT::keywordName = "GDORIENT";
const std::string GDORIENT::I::itemName = "I";
const std::string GDORIENT::I::defaultValue = "INC";
const std::string GDORIENT::J::itemName = "J";
const std::string GDORIENT::J::defaultValue = "INC";
const std::string GDORIENT::K::itemName = "K";
const std::string GDORIENT::K::defaultValue = "INC";
const std::string GDORIENT::Z::itemName = "Z";
const std::string GDORIENT::Z::defaultValue = "DOWN";
const std::string GDORIENT::HAND::itemName = "HAND";
const std::string GDORIENT::HAND::defaultValue = "RIGHT";


GECON::GECON( ) : ParserKeyword("GECON") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GECON");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MIN_OIL_RATE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MIN_GAS_RATE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_WCT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_GOR",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_WATER_GAS_RATIO",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("WORKOVER",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("END_RUN",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_OPEN_WELLS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GECON::keywordName = "GECON";
const std::string GECON::GROUP::itemName = "GROUP";
const std::string GECON::MIN_OIL_RATE::itemName = "MIN_OIL_RATE";
const double GECON::MIN_OIL_RATE::defaultValue = 0;
const std::string GECON::MIN_GAS_RATE::itemName = "MIN_GAS_RATE";
const double GECON::MIN_GAS_RATE::defaultValue = 0;
const std::string GECON::MAX_WCT::itemName = "MAX_WCT";
const double GECON::MAX_WCT::defaultValue = 0;
const std::string GECON::MAX_GOR::itemName = "MAX_GOR";
const double GECON::MAX_GOR::defaultValue = 0;
const std::string GECON::MAX_WATER_GAS_RATIO::itemName = "MAX_WATER_GAS_RATIO";
const double GECON::MAX_WATER_GAS_RATIO::defaultValue = 0;
const std::string GECON::WORKOVER::itemName = "WORKOVER";
const std::string GECON::WORKOVER::defaultValue = "NONE";
const std::string GECON::END_RUN::itemName = "END_RUN";
const std::string GECON::END_RUN::defaultValue = "NO";
const std::string GECON::MAX_OPEN_WELLS::itemName = "MAX_OPEN_WELLS";
const int GECON::MAX_OPEN_WELLS::defaultValue = 0;


GEFAC::GEFAC( ) : ParserKeyword("GEFAC") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GEFAC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("EFFICIENCY_FACTOR",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TRANSFER_EXT_NET",Opm::SINGLE,"YES"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GEFAC::keywordName = "GEFAC";
const std::string GEFAC::GROUP::itemName = "GROUP";
const std::string GEFAC::EFFICIENCY_FACTOR::itemName = "EFFICIENCY_FACTOR";
const double GEFAC::EFFICIENCY_FACTOR::defaultValue = 1;
const std::string GEFAC::TRANSFER_EXT_NET::itemName = "TRANSFER_EXT_NET";
const std::string GEFAC::TRANSFER_EXT_NET::defaultValue = "YES";


GRID::GRID( ) : ParserKeyword("GRID") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("GRID");
}
const std::string GRID::keywordName = "GRID";


GRIDFILE::GRIDFILE( ) : ParserKeyword("GRIDFILE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GRIDFILE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("GRID",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("EGRID",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDFILE::keywordName = "GRIDFILE";
const std::string GRIDFILE::GRID::itemName = "GRID";
const int GRIDFILE::GRID::defaultValue = 0;
const std::string GRIDFILE::EGRID::itemName = "EGRID";
const int GRIDFILE::EGRID::defaultValue = 1;


GRIDOPTS::GRIDOPTS( ) : ParserKeyword("GRIDOPTS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("GRIDOPTS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("TRANMULT",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NRMULT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NRPINC",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDOPTS::keywordName = "GRIDOPTS";
const std::string GRIDOPTS::TRANMULT::itemName = "TRANMULT";
const std::string GRIDOPTS::TRANMULT::defaultValue = "NO";
const std::string GRIDOPTS::NRMULT::itemName = "NRMULT";
const int GRIDOPTS::NRMULT::defaultValue = 0;
const std::string GRIDOPTS::NRPINC::itemName = "NRPINC";
const int GRIDOPTS::NRPINC::defaultValue = 0;


GRIDUNIT::GRIDUNIT( ) : ParserKeyword("GRIDUNIT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("GRIDUNIT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("LengthUnit",Opm::SINGLE,"METRES"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("MAP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRIDUNIT::keywordName = "GRIDUNIT";
const std::string GRIDUNIT::LengthUnit::itemName = "LengthUnit";
const std::string GRIDUNIT::LengthUnit::defaultValue = "METRES";
const std::string GRIDUNIT::MAP::itemName = "MAP";


GROUP_PROBE::GROUP_PROBE( ) : ParserKeyword("GROUP_PROBE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("GALQ");
  addDeckName("GAPI");
  addDeckName("GCIC");
  addDeckName("GCIR");
  addDeckName("GCIT");
  addDeckName("GCPC");
  addDeckName("GCPR");
  addDeckName("GCPT");
  addDeckName("GEDC");
  addDeckName("GEDCQ");
  addDeckName("GEFF");
  addDeckName("GEPR");
  addDeckName("GEPT");
  addDeckName("GESR");
  addDeckName("GEST");
  addDeckName("GFGR");
  addDeckName("GFGT");
  addDeckName("GFMF");
  addDeckName("GGCR");
  addDeckName("GGCT");
  addDeckName("GGCV");
  addDeckName("GGDC");
  addDeckName("GGDCQ");
  addDeckName("GGIGR");
  addDeckName("GGIMR");
  addDeckName("GGIMT");
  addDeckName("GGIR");
  addDeckName("GGIRH");
  addDeckName("GGIRL");
  addDeckName("GGIRNB");
  addDeckName("GGIRT");
  addDeckName("GGIT");
  addDeckName("GGITH");
  addDeckName("GGLIR");
  addDeckName("GGLR");
  addDeckName("GGLRH");
  addDeckName("GGOR");
  addDeckName("GGORH");
  addDeckName("GGPGR");
  addDeckName("GGPI");
  addDeckName("GGPI2");
  addDeckName("GGPP");
  addDeckName("GGPP2");
  addDeckName("GGPPF");
  addDeckName("GGPPF2");
  addDeckName("GGPPS");
  addDeckName("GGPPS2");
  addDeckName("GGPR");
  addDeckName("GGPRF");
  addDeckName("GGPRH");
  addDeckName("GGPRL");
  addDeckName("GGPRNB");
  addDeckName("GGPRNBFP");
  addDeckName("GGPRS");
  addDeckName("GGPRT");
  addDeckName("GGPT");
  addDeckName("GGPTF");
  addDeckName("GGPTH");
  addDeckName("GGPTS");
  addDeckName("GGQ");
  addDeckName("GGSPR");
  addDeckName("GGSR");
  addDeckName("GGSRL");
  addDeckName("GGSRU");
  addDeckName("GGSSP");
  addDeckName("GGST");
  addDeckName("GGSTP");
  addDeckName("GJPR");
  addDeckName("GJPRH");
  addDeckName("GJPRL");
  addDeckName("GJPRT");
  addDeckName("GJPT");
  addDeckName("GJPTH");
  addDeckName("GLPR");
  addDeckName("GLPRH");
  addDeckName("GLPRL");
  addDeckName("GLPRNB");
  addDeckName("GLPRT");
  addDeckName("GLPT");
  addDeckName("GLPTH");
  addDeckName("GMCPL");
  addDeckName("GMCTG");
  addDeckName("GMCTP");
  addDeckName("GMCTW");
  addDeckName("GMIR");
  addDeckName("GMIT");
  addDeckName("GMPR");
  addDeckName("GMPT");
  addDeckName("GMWDR");
  addDeckName("GMWDT");
  addDeckName("GMWIA");
  addDeckName("GMWIG");
  addDeckName("GMWIN");
  addDeckName("GMWIP");
  addDeckName("GMWIS");
  addDeckName("GMWIT");
  addDeckName("GMWIU");
  addDeckName("GMWIV");
  addDeckName("GMWPA");
  addDeckName("GMWPG");
  addDeckName("GMWPL");
  addDeckName("GMWPO");
  addDeckName("GMWPP");
  addDeckName("GMWPR");
  addDeckName("GMWPS");
  addDeckName("GMWPT");
  addDeckName("GMWPU");
  addDeckName("GMWPV");
  addDeckName("GMWWO");
  addDeckName("GMWWT");
  addDeckName("GNIR");
  addDeckName("GNIT");
  addDeckName("GNPR");
  addDeckName("GNPT");
  addDeckName("GOGR");
  addDeckName("GOGRH");
  addDeckName("GOIGR");
  addDeckName("GOIR");
  addDeckName("GOIRH");
  addDeckName("GOIRL");
  addDeckName("GOIRT");
  addDeckName("GOIT");
  addDeckName("GOITH");
  addDeckName("GOPGR");
  addDeckName("GOPI");
  addDeckName("GOPI2");
  addDeckName("GOPP");
  addDeckName("GOPP2");
  addDeckName("GOPR");
  addDeckName("GOPRF");
  addDeckName("GOPRH");
  addDeckName("GOPRL");
  addDeckName("GOPRNB");
  addDeckName("GOPRS");
  addDeckName("GOPRT");
  addDeckName("GOPT");
  addDeckName("GOPTF");
  addDeckName("GOPTH");
  addDeckName("GOPTS");
  addDeckName("GOSPR");
  addDeckName("GOSRL");
  addDeckName("GOSRU");
  addDeckName("GOSSP");
  addDeckName("GOSTP");
  addDeckName("GPR");
  addDeckName("GPRB");
  addDeckName("GPRBG");
  addDeckName("GPRBW");
  addDeckName("GPRDC");
  addDeckName("GPRFP");
  addDeckName("GPRG");
  addDeckName("GPRW");
  addDeckName("GSGR");
  addDeckName("GSGT");
  addDeckName("GSIC");
  addDeckName("GSIR");
  addDeckName("GSIT");
  addDeckName("GSMF");
  addDeckName("GSPC");
  addDeckName("GSPR");
  addDeckName("GSPT");
  addDeckName("GTICHEA");
  addDeckName("GTIRALK");
  addDeckName("GTIRANI");
  addDeckName("GTIRCAT");
  addDeckName("GTIRFOA");
  addDeckName("GTIRHEA");
  addDeckName("GTIRSUR");
  addDeckName("GTITALK");
  addDeckName("GTITANI");
  addDeckName("GTITCAT");
  addDeckName("GTITFOA");
  addDeckName("GTITHEA");
  addDeckName("GTITSUR");
  addDeckName("GTPCHEA");
  addDeckName("GTPRALK");
  addDeckName("GTPRANI");
  addDeckName("GTPRCAT");
  addDeckName("GTPRFOA");
  addDeckName("GTPRHEA");
  addDeckName("GTPRSUR");
  addDeckName("GTPTALK");
  addDeckName("GTPTANI");
  addDeckName("GTPTCAT");
  addDeckName("GTPTFOA");
  addDeckName("GTPTHEA");
  addDeckName("GTPTSUR");
  addDeckName("GVIR");
  addDeckName("GVIRL");
  addDeckName("GVIRT");
  addDeckName("GVIT");
  addDeckName("GVPGR");
  addDeckName("GVPR");
  addDeckName("GVPRL");
  addDeckName("GVPRT");
  addDeckName("GVPT");
  addDeckName("GWCT");
  addDeckName("GWCTH");
  addDeckName("GWGR");
  addDeckName("GWGRH");
  addDeckName("GWIGR");
  addDeckName("GWIR");
  addDeckName("GWIRH");
  addDeckName("GWIRL");
  addDeckName("GWIRNB");
  addDeckName("GWIRT");
  addDeckName("GWIT");
  addDeckName("GWITH");
  addDeckName("GWPGR");
  addDeckName("GWPI");
  addDeckName("GWPI2");
  addDeckName("GWPIR");
  addDeckName("GWPP");
  addDeckName("GWPP2");
  addDeckName("GWPR");
  addDeckName("GWPRH");
  addDeckName("GWPRL");
  addDeckName("GWPRNB");
  addDeckName("GWPRT");
  addDeckName("GWPT");
  addDeckName("GWPTH");
  addDeckName("GWSPR");
  addDeckName("GWSRL");
  addDeckName("GWSRU");
  addDeckName("GWSSP");
  addDeckName("GWSTP");
  setMatchRegex("GU.+|GTPR.+|GTPT.+|GTPC.+|GTIR.+|GTIT.+|GTIC.+|GTIRF.+|GTIRS.+|GTPRF.+|GTPRS.+|GTITF.+|GTITS.+|GTPTF.+|GTPTS.+|GTICF.+|GTICS.+|GTPCF.+|GTPCS.+");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("GROUPS",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GROUP_PROBE::keywordName = "GROUP_PROBE";
const std::string GROUP_PROBE::GROUPS::itemName = "GROUPS";


GRUPNET::GRUPNET( ) : ParserKeyword("GRUPNET") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("GRUPNET");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("NAME",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TERMINAL_PRESSURE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ALQ",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("SUB_SEA_MANIFOLD",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("LIFT_GAS_FLOW_THROUGH",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ALQ_SURFACE_EQV",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPNET::keywordName = "GRUPNET";
const std::string GRUPNET::NAME::itemName = "NAME";
const std::string GRUPNET::TERMINAL_PRESSURE::itemName = "TERMINAL_PRESSURE";
const std::string GRUPNET::VFP_TABLE::itemName = "VFP_TABLE";
const int GRUPNET::VFP_TABLE::defaultValue = 0;
const std::string GRUPNET::ALQ::itemName = "ALQ";
const double GRUPNET::ALQ::defaultValue = 0;
const std::string GRUPNET::SUB_SEA_MANIFOLD::itemName = "SUB_SEA_MANIFOLD";
const std::string GRUPNET::SUB_SEA_MANIFOLD::defaultValue = "NO";
const std::string GRUPNET::LIFT_GAS_FLOW_THROUGH::itemName = "LIFT_GAS_FLOW_THROUGH";
const std::string GRUPNET::LIFT_GAS_FLOW_THROUGH::defaultValue = "NO";
const std::string GRUPNET::ALQ_SURFACE_EQV::itemName = "ALQ_SURFACE_EQV";
const std::string GRUPNET::ALQ_SURFACE_EQV::defaultValue = "NONE";


GRUPTREE::GRUPTREE( ) : ParserKeyword("GRUPTREE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("GRUPTREE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("CHILD_GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PARENT_GROUP",Opm::SINGLE,"FIELD"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string GRUPTREE::keywordName = "GRUPTREE";
const std::string GRUPTREE::CHILD_GROUP::itemName = "CHILD_GROUP";
const std::string GRUPTREE::PARENT_GROUP::itemName = "PARENT_GROUP";
const std::string GRUPTREE::PARENT_GROUP::defaultValue = "FIELD";


IMBNUM::IMBNUM( ) : ParserKeyword("IMBNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("IMBNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string IMBNUM::keywordName = "IMBNUM";
const std::string IMBNUM::data::itemName = "data";


IMKRVD::IMKRVD( ) : ParserKeyword("IMKRVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ENDSCALE","NUM_TABLES");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMKRVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMKRVD::keywordName = "IMKRVD";
const std::string IMKRVD::DATA::itemName = "DATA";


IMPES::IMPES( ) : ParserKeyword("IMPES") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("IMPES");
}
const std::string IMPES::keywordName = "IMPES";


IMPTVD::IMPTVD( ) : ParserKeyword("IMPTVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ENDSCALE","NUM_TABLES");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMPTVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMPTVD::keywordName = "IMPTVD";
const std::string IMPTVD::DATA::itemName = "DATA";


INCLUDE::INCLUDE( ) : ParserKeyword("INCLUDE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("INCLUDE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("IncludeFile",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string INCLUDE::keywordName = "INCLUDE";
const std::string INCLUDE::IncludeFile::itemName = "IncludeFile";


INIT::INIT( ) : ParserKeyword("INIT") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("INIT");
}
const std::string INIT::keywordName = "INIT";


IPCG::IPCG( ) : ParserKeyword("IPCG") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IPCG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string IPCG::keywordName = "IPCG";
const std::string IPCG::data::itemName = "data";


IPCW::IPCW( ) : ParserKeyword("IPCW") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IPCW");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string IPCW::keywordName = "IPCW";
const std::string IPCW::data::itemName = "data";


ISGCR::ISGCR( ) : ParserKeyword("ISGCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISGCR::keywordName = "ISGCR";
const std::string ISGCR::data::itemName = "data";


ISGL::ISGL( ) : ParserKeyword("ISGL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISGL::keywordName = "ISGL";
const std::string ISGL::data::itemName = "data";


ISGU::ISGU( ) : ParserKeyword("ISGU") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGU");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISGU::keywordName = "ISGU";
const std::string ISGU::data::itemName = "data";


ISOGCR::ISOGCR( ) : ParserKeyword("ISOGCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISOGCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISOGCR::keywordName = "ISOGCR";
const std::string ISOGCR::data::itemName = "data";


ISOWCR::ISOWCR( ) : ParserKeyword("ISOWCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISOWCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISOWCR::keywordName = "ISOWCR";
const std::string ISOWCR::data::itemName = "data";


ISWCR::ISWCR( ) : ParserKeyword("ISWCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISWCR::keywordName = "ISWCR";
const std::string ISWCR::data::itemName = "data";


ISWL::ISWL( ) : ParserKeyword("ISWL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISWL::keywordName = "ISWL";
const std::string ISWL::data::itemName = "data";


ISWU::ISWU( ) : ParserKeyword("ISWU") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWU");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ISWU::keywordName = "ISWU";
const std::string ISWU::data::itemName = "data";


MAPAXES::MAPAXES( ) : ParserKeyword("MAPAXES") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MAPAXES");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("X1",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("Y1",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("X2",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("Y2",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("X3",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("Y3",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MAPAXES::keywordName = "MAPAXES";
const std::string MAPAXES::X1::itemName = "X1";
const std::string MAPAXES::Y1::itemName = "Y1";
const std::string MAPAXES::X2::itemName = "X2";
const std::string MAPAXES::Y2::itemName = "Y2";
const std::string MAPAXES::X3::itemName = "X3";
const std::string MAPAXES::Y3::itemName = "Y3";


MAPUNITS::MAPUNITS( ) : ParserKeyword("MAPUNITS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MAPUNITS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("UNIT",Opm::SINGLE,"METRES"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MAPUNITS::keywordName = "MAPUNITS";
const std::string MAPUNITS::UNIT::itemName = "UNIT";
const std::string MAPUNITS::UNIT::defaultValue = "METRES";


MAXVALUE::MAXVALUE( ) : ParserKeyword("MAXVALUE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MAXVALUE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("field",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("value",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MAXVALUE::keywordName = "MAXVALUE";
const std::string MAXVALUE::field::itemName = "field";
const std::string MAXVALUE::value::itemName = "value";
const std::string MAXVALUE::I1::itemName = "I1";
const std::string MAXVALUE::I2::itemName = "I2";
const std::string MAXVALUE::J1::itemName = "J1";
const std::string MAXVALUE::J2::itemName = "J2";
const std::string MAXVALUE::K1::itemName = "K1";
const std::string MAXVALUE::K2::itemName = "K2";


MEMORY::MEMORY( ) : ParserKeyword("MEMORY") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MEMORY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("UNUSED",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("THOUSANDS_CHAR8",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MEMORY::keywordName = "MEMORY";
const std::string MEMORY::UNUSED::itemName = "UNUSED";
const std::string MEMORY::THOUSANDS_CHAR8::itemName = "THOUSANDS_CHAR8";


MESSAGES::MESSAGES( ) : ParserKeyword("MESSAGES") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("MESSAGES");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MESSAGE_PRINT_LIMIT",Opm::SINGLE,1000000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("COMMENT_PRINT_LIMIT",Opm::SINGLE,1000000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("WARNING_PRINT_LIMIT",Opm::SINGLE,10000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PROBLEM_PRINT_LIMIT",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ERROR_PRINT_LIMIT",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BUG_PRINT_LIMIT",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MESSAGE_STOP_LIMIT",Opm::SINGLE,1000000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("COMMENT_STOP_LIMIT",Opm::SINGLE,1000000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("WARNING_STOP_LIMIT",Opm::SINGLE,10000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PROBLEM_STOP_LIMIT",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ERROR_STOP_LIMIT",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BUG_STOP_LIMIT",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MESSAGES::keywordName = "MESSAGES";
const std::string MESSAGES::MESSAGE_PRINT_LIMIT::itemName = "MESSAGE_PRINT_LIMIT";
const int MESSAGES::MESSAGE_PRINT_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::COMMENT_PRINT_LIMIT::itemName = "COMMENT_PRINT_LIMIT";
const int MESSAGES::COMMENT_PRINT_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::WARNING_PRINT_LIMIT::itemName = "WARNING_PRINT_LIMIT";
const int MESSAGES::WARNING_PRINT_LIMIT::defaultValue = 10000;
const std::string MESSAGES::PROBLEM_PRINT_LIMIT::itemName = "PROBLEM_PRINT_LIMIT";
const int MESSAGES::PROBLEM_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::ERROR_PRINT_LIMIT::itemName = "ERROR_PRINT_LIMIT";
const int MESSAGES::ERROR_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::BUG_PRINT_LIMIT::itemName = "BUG_PRINT_LIMIT";
const int MESSAGES::BUG_PRINT_LIMIT::defaultValue = 100;
const std::string MESSAGES::MESSAGE_STOP_LIMIT::itemName = "MESSAGE_STOP_LIMIT";
const int MESSAGES::MESSAGE_STOP_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::COMMENT_STOP_LIMIT::itemName = "COMMENT_STOP_LIMIT";
const int MESSAGES::COMMENT_STOP_LIMIT::defaultValue = 1000000;
const std::string MESSAGES::WARNING_STOP_LIMIT::itemName = "WARNING_STOP_LIMIT";
const int MESSAGES::WARNING_STOP_LIMIT::defaultValue = 10000;
const std::string MESSAGES::PROBLEM_STOP_LIMIT::itemName = "PROBLEM_STOP_LIMIT";
const int MESSAGES::PROBLEM_STOP_LIMIT::defaultValue = 100;
const std::string MESSAGES::ERROR_STOP_LIMIT::itemName = "ERROR_STOP_LIMIT";
const int MESSAGES::ERROR_STOP_LIMIT::defaultValue = 10;
const std::string MESSAGES::BUG_STOP_LIMIT::itemName = "BUG_STOP_LIMIT";
const int MESSAGES::BUG_STOP_LIMIT::defaultValue = 1;


METRIC::METRIC( ) : ParserKeyword("METRIC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("METRIC");
}
const std::string METRIC::keywordName = "METRIC";


MINPV::MINPV( ) : ParserKeyword("MINPV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("VALUE",Opm::SINGLE,9.9999999999999995e-07));
        item->setDescription("");
        item->push_backDimension("ReservoirVolume");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINPV::keywordName = "MINPV";
const std::string MINPV::VALUE::itemName = "VALUE";
const double MINPV::VALUE::defaultValue = 1e-06;


MINPVFIL::MINPVFIL( ) : ParserKeyword("MINPVFIL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MINPVFIL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("VALUE",Opm::SINGLE,9.9999999999999995e-07));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINPVFIL::keywordName = "MINPVFIL";
const std::string MINPVFIL::VALUE::itemName = "VALUE";
const double MINPVFIL::VALUE::defaultValue = 1e-06;


MINVALUE::MINVALUE( ) : ParserKeyword("MINVALUE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MINVALUE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("field",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("value",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MINVALUE::keywordName = "MINVALUE";
const std::string MINVALUE::field::itemName = "field";
const std::string MINVALUE::value::itemName = "value";
const std::string MINVALUE::I1::itemName = "I1";
const std::string MINVALUE::I2::itemName = "I2";
const std::string MINVALUE::J1::itemName = "J1";
const std::string MINVALUE::J2::itemName = "J2";
const std::string MINVALUE::K1::itemName = "K1";
const std::string MINVALUE::K2::itemName = "K2";


MISC::MISC( ) : ParserKeyword("MISC") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MISC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MISC::keywordName = "MISC";
const std::string MISC::DATA::itemName = "DATA";


MISCIBLE::MISCIBLE( ) : ParserKeyword("MISCIBLE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MISCIBLE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NTMISC",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NSMISC",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TWOPOINT",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MISCIBLE::keywordName = "MISCIBLE";
const std::string MISCIBLE::NTMISC::itemName = "NTMISC";
const int MISCIBLE::NTMISC::defaultValue = 1;
const std::string MISCIBLE::NSMISC::itemName = "NSMISC";
const int MISCIBLE::NSMISC::defaultValue = 20;
const std::string MISCIBLE::TWOPOINT::itemName = "TWOPOINT";
const std::string MISCIBLE::TWOPOINT::defaultValue = "NONE";


MISCNUM::MISCNUM( ) : ParserKeyword("MISCNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("MISCNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MISCNUM::keywordName = "MISCNUM";
const std::string MISCNUM::data::itemName = "data";


MONITOR::MONITOR( ) : ParserKeyword("MONITOR") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("MONITOR");
}
const std::string MONITOR::keywordName = "MONITOR";


MSFN::MSFN( ) : ParserKeyword("MSFN") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MSFN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MSFN::keywordName = "MSFN";
const std::string MSFN::table::itemName = "table";


MSGFILE::MSGFILE( ) : ParserKeyword("MSGFILE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("MSGFILE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("ENABLE_FLAG",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MSGFILE::keywordName = "MSGFILE";
const std::string MSGFILE::ENABLE_FLAG::itemName = "ENABLE_FLAG";


MULTFLT::MULTFLT( ) : ParserKeyword("MULTFLT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTFLT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("fault",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("factor",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTFLT::keywordName = "MULTFLT";
const std::string MULTFLT::fault::itemName = "fault";
const std::string MULTFLT::factor::itemName = "factor";


MULTIPLY::MULTIPLY( ) : ParserKeyword("MULTIPLY") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("MULTIPLY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("field",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("factor",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTIPLY::keywordName = "MULTIPLY";
const std::string MULTIPLY::field::itemName = "field";
const std::string MULTIPLY::factor::itemName = "factor";
const std::string MULTIPLY::I1::itemName = "I1";
const std::string MULTIPLY::I2::itemName = "I2";
const std::string MULTIPLY::J1::itemName = "J1";
const std::string MULTIPLY::J2::itemName = "J2";
const std::string MULTIPLY::K1::itemName = "K1";
const std::string MULTIPLY::K2::itemName = "K2";


MULTIREG::MULTIREG( ) : ParserKeyword("MULTIREG") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("MULTIREG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("ARRAY",Opm::SINGLE));
        item->setDescription("The 3D array we will update");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("FACTOR",Opm::SINGLE,0));
        item->setDescription("The value we will multiply with");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REGION_NUMBER",Opm::SINGLE));
        item->setDescription("The region number we are interested in");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REGION_NAME",Opm::SINGLE,"M"));
        item->setDescription("The name of the region we are interested in");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTIREG::keywordName = "MULTIREG";
const std::string MULTIREG::ARRAY::itemName = "ARRAY";
const std::string MULTIREG::FACTOR::itemName = "FACTOR";
const double MULTIREG::FACTOR::defaultValue = 0;
const std::string MULTIREG::REGION_NUMBER::itemName = "REGION_NUMBER";
const std::string MULTIREG::REGION_NAME::itemName = "REGION_NAME";
const std::string MULTIREG::REGION_NAME::defaultValue = "M";


MULTNUM::MULTNUM( ) : ParserKeyword("MULTNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MULTNUM::keywordName = "MULTNUM";
const std::string MULTNUM::data::itemName = "data";


MULTPV::MULTPV( ) : ParserKeyword("MULTPV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTPV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MULTPV::keywordName = "MULTPV";
const std::string MULTPV::data::itemName = "data";


MULTREGT::MULTREGT( ) : ParserKeyword("MULTREGT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("MULTREGT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("SRC_REGION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("TARGET_REGION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRAN_MULT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("DIRECTIONS",Opm::SINGLE,"XYZ"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("NNC_MULT",Opm::SINGLE,"ALL"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REGION_DEF",Opm::SINGLE,"M"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MULTREGT::keywordName = "MULTREGT";
const std::string MULTREGT::SRC_REGION::itemName = "SRC_REGION";
const std::string MULTREGT::TARGET_REGION::itemName = "TARGET_REGION";
const std::string MULTREGT::TRAN_MULT::itemName = "TRAN_MULT";
const std::string MULTREGT::DIRECTIONS::itemName = "DIRECTIONS";
const std::string MULTREGT::DIRECTIONS::defaultValue = "XYZ";
const std::string MULTREGT::NNC_MULT::itemName = "NNC_MULT";
const std::string MULTREGT::NNC_MULT::defaultValue = "ALL";
const std::string MULTREGT::REGION_DEF::itemName = "REGION_DEF";
const std::string MULTREGT::REGION_DEF::defaultValue = "M";


MULT_XYZ::MULT_XYZ( ) : ParserKeyword("MULT_XYZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("MULTX");
  addDeckName("MULTX-");
  addDeckName("MULTY");
  addDeckName("MULTY-");
  addDeckName("MULTZ");
  addDeckName("MULTZ-");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string MULT_XYZ::keywordName = "MULT_XYZ";
const std::string MULT_XYZ::data::itemName = "data";


MW::MW( ) : ParserKeyword("MW") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MW");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("MOLAR_WEIGHT",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MW::keywordName = "MW";
const std::string MW::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


MWS::MWS( ) : ParserKeyword("MWS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("MWS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("MOLAR_WEIGHT",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string MWS::keywordName = "MWS";
const std::string MWS::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


NETBALAN::NETBALAN( ) : ParserKeyword("NETBALAN") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("NETBALAN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TIME_INTERVAL",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PRESSURE_CONVERGENCE_LIMT",Opm::SINGLE,1.0000000000000001e-05));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ITER",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THP_CONVERGENCE_LIMIT",Opm::SINGLE,0.01));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ITER_THP",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TARGET_BALANCE_ERROR",Opm::SINGLE,1e+20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_BALANCE_ERROR",Opm::SINGLE,1e+20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MIN_TIME_STEP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NETBALAN::keywordName = "NETBALAN";
const std::string NETBALAN::TIME_INTERVAL::itemName = "TIME_INTERVAL";
const double NETBALAN::TIME_INTERVAL::defaultValue = 0;
const std::string NETBALAN::PRESSURE_CONVERGENCE_LIMT::itemName = "PRESSURE_CONVERGENCE_LIMT";
const double NETBALAN::PRESSURE_CONVERGENCE_LIMT::defaultValue = 1e-05;
const std::string NETBALAN::MAX_ITER::itemName = "MAX_ITER";
const int NETBALAN::MAX_ITER::defaultValue = 10;
const std::string NETBALAN::THP_CONVERGENCE_LIMIT::itemName = "THP_CONVERGENCE_LIMIT";
const double NETBALAN::THP_CONVERGENCE_LIMIT::defaultValue = 0.01;
const std::string NETBALAN::MAX_ITER_THP::itemName = "MAX_ITER_THP";
const int NETBALAN::MAX_ITER_THP::defaultValue = 10;
const std::string NETBALAN::TARGET_BALANCE_ERROR::itemName = "TARGET_BALANCE_ERROR";
const double NETBALAN::TARGET_BALANCE_ERROR::defaultValue = 1e+20;
const std::string NETBALAN::MAX_BALANCE_ERROR::itemName = "MAX_BALANCE_ERROR";
const double NETBALAN::MAX_BALANCE_ERROR::defaultValue = 1e+20;
const std::string NETBALAN::MIN_TIME_STEP::itemName = "MIN_TIME_STEP";


NEWTRAN::NEWTRAN( ) : ParserKeyword("NEWTRAN") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NEWTRAN");
}
const std::string NEWTRAN::keywordName = "NEWTRAN";


NEXTSTEP::NEXTSTEP( ) : ParserKeyword("NEXTSTEP") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NEXTSTEP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_STEP",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("APPLY_TO_ALL",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NEXTSTEP::keywordName = "NEXTSTEP";
const std::string NEXTSTEP::MAX_STEP::itemName = "MAX_STEP";
const std::string NEXTSTEP::APPLY_TO_ALL::itemName = "APPLY_TO_ALL";
const std::string NEXTSTEP::APPLY_TO_ALL::defaultValue = "NO";


NNC::NNC( ) : ParserKeyword("NNC") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NNC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRAN",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Transmissibility");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SIM_DEPENDENT1",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SIM_DEPENDENT2",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PRESSURE_TABLE1",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("PRESSURE_TABLE2",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("VE_FACE1",Opm::SINGLE,""));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("VE_FACE2",Opm::SINGLE,""));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DIFFUSIVITY",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SIM_DEPENDENT3",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VDFLOW_AREA",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VDFLOW_PERM",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NNC::keywordName = "NNC";
const std::string NNC::I1::itemName = "I1";
const std::string NNC::J1::itemName = "J1";
const std::string NNC::K1::itemName = "K1";
const std::string NNC::I2::itemName = "I2";
const std::string NNC::J2::itemName = "J2";
const std::string NNC::K2::itemName = "K2";
const std::string NNC::TRAN::itemName = "TRAN";
const double NNC::TRAN::defaultValue = 0;
const std::string NNC::SIM_DEPENDENT1::itemName = "SIM_DEPENDENT1";
const double NNC::SIM_DEPENDENT1::defaultValue = 0;
const std::string NNC::SIM_DEPENDENT2::itemName = "SIM_DEPENDENT2";
const double NNC::SIM_DEPENDENT2::defaultValue = 0;
const std::string NNC::PRESSURE_TABLE1::itemName = "PRESSURE_TABLE1";
const int NNC::PRESSURE_TABLE1::defaultValue = 0;
const std::string NNC::PRESSURE_TABLE2::itemName = "PRESSURE_TABLE2";
const int NNC::PRESSURE_TABLE2::defaultValue = 0;
const std::string NNC::VE_FACE1::itemName = "VE_FACE1";
const std::string NNC::VE_FACE1::defaultValue = "";
const std::string NNC::VE_FACE2::itemName = "VE_FACE2";
const std::string NNC::VE_FACE2::defaultValue = "";
const std::string NNC::DIFFUSIVITY::itemName = "DIFFUSIVITY";
const double NNC::DIFFUSIVITY::defaultValue = 0;
const std::string NNC::SIM_DEPENDENT3::itemName = "SIM_DEPENDENT3";
const double NNC::SIM_DEPENDENT3::defaultValue = 0;
const std::string NNC::VDFLOW_AREA::itemName = "VDFLOW_AREA";
const double NNC::VDFLOW_AREA::defaultValue = 0;
const std::string NNC::VDFLOW_PERM::itemName = "VDFLOW_PERM";
const double NNC::VDFLOW_PERM::defaultValue = 0;


NOCASC::NOCASC( ) : ParserKeyword("NOCASC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOCASC");
}
const std::string NOCASC::keywordName = "NOCASC";


NOECHO::NOECHO( ) : ParserKeyword("NOECHO") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NOECHO");
}
const std::string NOECHO::keywordName = "NOECHO";


NOGGF::NOGGF( ) : ParserKeyword("NOGGF") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NOGGF");
}
const std::string NOGGF::keywordName = "NOGGF";


NOGRAV::NOGRAV( ) : ParserKeyword("NOGRAV") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOGRAV");
}
const std::string NOGRAV::keywordName = "NOGRAV";


NOINSPEC::NOINSPEC( ) : ParserKeyword("NOINSPEC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NOINSPEC");
}
const std::string NOINSPEC::keywordName = "NOINSPEC";


NOMONITO::NOMONITO( ) : ParserKeyword("NOMONITO") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NOMONITO");
}
const std::string NOMONITO::keywordName = "NOMONITO";


NONNC::NONNC( ) : ParserKeyword("NONNC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NONNC");
}
const std::string NONNC::keywordName = "NONNC";


NORSSPEC::NORSSPEC( ) : ParserKeyword("NORSSPEC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NORSSPEC");
}
const std::string NORSSPEC::keywordName = "NORSSPEC";


NOSIM::NOSIM( ) : ParserKeyword("NOSIM") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NOSIM");
}
const std::string NOSIM::keywordName = "NOSIM";


NSTACK::NSTACK( ) : ParserKeyword("NSTACK") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NSTACK");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("LINEAR_SOLVER_SIZE",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NSTACK::keywordName = "NSTACK";
const std::string NSTACK::LINEAR_SOLVER_SIZE::itemName = "LINEAR_SOLVER_SIZE";
const int NSTACK::LINEAR_SOLVER_SIZE::defaultValue = 10;


NTG::NTG( ) : ParserKeyword("NTG") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("NTG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string NTG::keywordName = "NTG";
const std::string NTG::data::itemName = "data";


NUMRES::NUMRES( ) : ParserKeyword("NUMRES") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("NUMRES");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("num",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NUMRES::keywordName = "NUMRES";
const std::string NUMRES::num::itemName = "num";
const int NUMRES::num::defaultValue = 1;


NUPCOL::NUPCOL( ) : ParserKeyword("NUPCOL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("NUPCOL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NUM_ITER",Opm::SINGLE,3));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string NUPCOL::keywordName = "NUPCOL";
const std::string NUPCOL::NUM_ITER::itemName = "NUM_ITER";
const int NUPCOL::NUM_ITER::defaultValue = 3;


OCOMPIDX::OCOMPIDX( ) : ParserKeyword("OCOMPIDX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("OCOMPIDX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("OIL_COMPONENT_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OCOMPIDX::keywordName = "OCOMPIDX";
const std::string OCOMPIDX::OIL_COMPONENT_INDEX::itemName = "OIL_COMPONENT_INDEX";


OIL::OIL( ) : ParserKeyword("OIL") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("OIL");
}
const std::string OIL::keywordName = "OIL";


OILCOMPR::OILCOMPR( ) : ParserKeyword("OILCOMPR") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILCOMPR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("COMPRESSIBILITY",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("EXPANSION_COEFF_LINEAR",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1/AbsoluteTemperature");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("EXPANSION_COEFF_QUADRATIC",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1/AbsoluteTemperature*AbsoluteTemperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILCOMPR::keywordName = "OILCOMPR";
const std::string OILCOMPR::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";
const double OILCOMPR::COMPRESSIBILITY::defaultValue = 0;
const std::string OILCOMPR::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const double OILCOMPR::EXPANSION_COEFF_LINEAR::defaultValue = 0;
const std::string OILCOMPR::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";
const double OILCOMPR::EXPANSION_COEFF_QUADRATIC::defaultValue = 0;


OILMW::OILMW( ) : ParserKeyword("OILMW") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILMW");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("MOLAR_WEIGHT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILMW::keywordName = "OILMW";
const std::string OILMW::MOLAR_WEIGHT::itemName = "MOLAR_WEIGHT";


OILVISCT::OILVISCT( ) : ParserKeyword("OILVISCT") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILVISCT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Temperature");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILVISCT::keywordName = "OILVISCT";
const std::string OILVISCT::DATA::itemName = "DATA";


OILVTIM::OILVTIM( ) : ParserKeyword("OILVTIM") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("OILVTIM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("INTERPOLATION_METHOD",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OILVTIM::keywordName = "OILVTIM";
const std::string OILVTIM::INTERPOLATION_METHOD::itemName = "INTERPOLATION_METHOD";


OLDTRAN::OLDTRAN( ) : ParserKeyword("OLDTRAN") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("OLDTRAN");
}
const std::string OLDTRAN::keywordName = "OLDTRAN";


OPERATE::OPERATE( ) : ParserKeyword("OPERATE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("OPERATE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("RESULT_ARRAY",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OPERATION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ARRAY_ARG",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PARAM1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PARAM2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPERATE::keywordName = "OPERATE";
const std::string OPERATE::RESULT_ARRAY::itemName = "RESULT_ARRAY";
const std::string OPERATE::I1::itemName = "I1";
const std::string OPERATE::I2::itemName = "I2";
const std::string OPERATE::J1::itemName = "J1";
const std::string OPERATE::J2::itemName = "J2";
const std::string OPERATE::K1::itemName = "K1";
const std::string OPERATE::K2::itemName = "K2";
const std::string OPERATE::OPERATION::itemName = "OPERATION";
const std::string OPERATE::ARRAY_ARG::itemName = "ARRAY_ARG";
const std::string OPERATE::PARAM1::itemName = "PARAM1";
const std::string OPERATE::PARAM2::itemName = "PARAM2";


OPTIONS::OPTIONS( ) : ParserKeyword("OPTIONS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("OPTIONS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("ITEM1",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM2",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM3",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM4",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM5",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM6",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM7",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM8",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM9",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM10",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM11",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM12",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM13",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM14",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM15",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM16",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM17",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM18",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM19",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM20",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM21",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM22",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM23",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM24",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM25",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM26",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM27",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM28",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM29",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM30",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM31",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM32",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM33",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM34",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM35",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM36",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM37",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM38",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM39",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM40",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM41",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM42",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM43",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM44",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM45",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM46",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM47",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM48",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM49",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM50",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM51",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM52",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM53",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM54",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM55",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM56",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM57",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM58",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM59",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM60",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM61",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM62",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM63",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM64",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM65",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM66",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM67",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM68",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM69",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM70",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM71",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM72",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM73",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM74",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM75",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM76",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM77",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM78",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM79",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM80",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM81",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM82",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM83",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM84",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM85",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM86",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM87",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM88",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM89",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM90",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM91",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM92",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM93",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM94",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM95",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM96",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM97",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM98",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM99",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM100",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM101",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM102",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM103",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM104",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM105",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM106",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM107",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM108",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM109",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM110",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM111",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM112",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM113",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM114",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM115",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM116",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM117",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM118",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM119",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM120",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM121",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM122",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM123",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM124",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM125",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM126",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM127",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM128",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM129",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM130",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM131",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM132",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM133",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM134",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM135",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM136",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM137",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM138",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM139",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM140",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM141",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM142",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM143",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM144",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM145",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM146",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM147",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM148",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM149",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM150",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM151",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM152",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM153",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM154",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM155",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM156",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM157",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM158",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM159",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM160",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM161",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM162",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM163",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM164",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM165",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM166",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM167",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM168",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM169",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM170",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM171",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM172",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM173",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM174",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM175",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM176",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM177",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM178",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM179",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM180",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM181",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM182",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM183",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM184",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM185",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM186",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM187",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM188",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM189",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM190",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM191",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM192",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM193",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM194",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM195",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM196",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ITEM197",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string OPTIONS::keywordName = "OPTIONS";
const std::string OPTIONS::ITEM1::itemName = "ITEM1";
const int OPTIONS::ITEM1::defaultValue = 0;
const std::string OPTIONS::ITEM2::itemName = "ITEM2";
const int OPTIONS::ITEM2::defaultValue = 0;
const std::string OPTIONS::ITEM3::itemName = "ITEM3";
const int OPTIONS::ITEM3::defaultValue = 0;
const std::string OPTIONS::ITEM4::itemName = "ITEM4";
const int OPTIONS::ITEM4::defaultValue = 0;
const std::string OPTIONS::ITEM5::itemName = "ITEM5";
const int OPTIONS::ITEM5::defaultValue = 0;
const std::string OPTIONS::ITEM6::itemName = "ITEM6";
const int OPTIONS::ITEM6::defaultValue = 0;
const std::string OPTIONS::ITEM7::itemName = "ITEM7";
const int OPTIONS::ITEM7::defaultValue = 0;
const std::string OPTIONS::ITEM8::itemName = "ITEM8";
const int OPTIONS::ITEM8::defaultValue = 0;
const std::string OPTIONS::ITEM9::itemName = "ITEM9";
const int OPTIONS::ITEM9::defaultValue = 0;
const std::string OPTIONS::ITEM10::itemName = "ITEM10";
const int OPTIONS::ITEM10::defaultValue = 0;
const std::string OPTIONS::ITEM11::itemName = "ITEM11";
const int OPTIONS::ITEM11::defaultValue = 0;
const std::string OPTIONS::ITEM12::itemName = "ITEM12";
const int OPTIONS::ITEM12::defaultValue = 0;
const std::string OPTIONS::ITEM13::itemName = "ITEM13";
const int OPTIONS::ITEM13::defaultValue = 0;
const std::string OPTIONS::ITEM14::itemName = "ITEM14";
const int OPTIONS::ITEM14::defaultValue = 0;
const std::string OPTIONS::ITEM15::itemName = "ITEM15";
const int OPTIONS::ITEM15::defaultValue = 0;
const std::string OPTIONS::ITEM16::itemName = "ITEM16";
const int OPTIONS::ITEM16::defaultValue = 0;
const std::string OPTIONS::ITEM17::itemName = "ITEM17";
const int OPTIONS::ITEM17::defaultValue = 0;
const std::string OPTIONS::ITEM18::itemName = "ITEM18";
const int OPTIONS::ITEM18::defaultValue = 0;
const std::string OPTIONS::ITEM19::itemName = "ITEM19";
const int OPTIONS::ITEM19::defaultValue = 0;
const std::string OPTIONS::ITEM20::itemName = "ITEM20";
const int OPTIONS::ITEM20::defaultValue = 0;
const std::string OPTIONS::ITEM21::itemName = "ITEM21";
const int OPTIONS::ITEM21::defaultValue = 0;
const std::string OPTIONS::ITEM22::itemName = "ITEM22";
const int OPTIONS::ITEM22::defaultValue = 0;
const std::string OPTIONS::ITEM23::itemName = "ITEM23";
const int OPTIONS::ITEM23::defaultValue = 0;
const std::string OPTIONS::ITEM24::itemName = "ITEM24";
const int OPTIONS::ITEM24::defaultValue = 0;
const std::string OPTIONS::ITEM25::itemName = "ITEM25";
const int OPTIONS::ITEM25::defaultValue = 0;
const std::string OPTIONS::ITEM26::itemName = "ITEM26";
const int OPTIONS::ITEM26::defaultValue = 0;
const std::string OPTIONS::ITEM27::itemName = "ITEM27";
const int OPTIONS::ITEM27::defaultValue = 0;
const std::string OPTIONS::ITEM28::itemName = "ITEM28";
const int OPTIONS::ITEM28::defaultValue = 0;
const std::string OPTIONS::ITEM29::itemName = "ITEM29";
const int OPTIONS::ITEM29::defaultValue = 0;
const std::string OPTIONS::ITEM30::itemName = "ITEM30";
const int OPTIONS::ITEM30::defaultValue = 0;
const std::string OPTIONS::ITEM31::itemName = "ITEM31";
const int OPTIONS::ITEM31::defaultValue = 0;
const std::string OPTIONS::ITEM32::itemName = "ITEM32";
const int OPTIONS::ITEM32::defaultValue = 0;
const std::string OPTIONS::ITEM33::itemName = "ITEM33";
const int OPTIONS::ITEM33::defaultValue = 0;
const std::string OPTIONS::ITEM34::itemName = "ITEM34";
const int OPTIONS::ITEM34::defaultValue = 0;
const std::string OPTIONS::ITEM35::itemName = "ITEM35";
const int OPTIONS::ITEM35::defaultValue = 0;
const std::string OPTIONS::ITEM36::itemName = "ITEM36";
const int OPTIONS::ITEM36::defaultValue = 0;
const std::string OPTIONS::ITEM37::itemName = "ITEM37";
const int OPTIONS::ITEM37::defaultValue = 0;
const std::string OPTIONS::ITEM38::itemName = "ITEM38";
const int OPTIONS::ITEM38::defaultValue = 0;
const std::string OPTIONS::ITEM39::itemName = "ITEM39";
const int OPTIONS::ITEM39::defaultValue = 0;
const std::string OPTIONS::ITEM40::itemName = "ITEM40";
const int OPTIONS::ITEM40::defaultValue = 0;
const std::string OPTIONS::ITEM41::itemName = "ITEM41";
const int OPTIONS::ITEM41::defaultValue = 0;
const std::string OPTIONS::ITEM42::itemName = "ITEM42";
const int OPTIONS::ITEM42::defaultValue = 0;
const std::string OPTIONS::ITEM43::itemName = "ITEM43";
const int OPTIONS::ITEM43::defaultValue = 0;
const std::string OPTIONS::ITEM44::itemName = "ITEM44";
const int OPTIONS::ITEM44::defaultValue = 0;
const std::string OPTIONS::ITEM45::itemName = "ITEM45";
const int OPTIONS::ITEM45::defaultValue = 0;
const std::string OPTIONS::ITEM46::itemName = "ITEM46";
const int OPTIONS::ITEM46::defaultValue = 0;
const std::string OPTIONS::ITEM47::itemName = "ITEM47";
const int OPTIONS::ITEM47::defaultValue = 0;
const std::string OPTIONS::ITEM48::itemName = "ITEM48";
const int OPTIONS::ITEM48::defaultValue = 0;
const std::string OPTIONS::ITEM49::itemName = "ITEM49";
const int OPTIONS::ITEM49::defaultValue = 0;
const std::string OPTIONS::ITEM50::itemName = "ITEM50";
const int OPTIONS::ITEM50::defaultValue = 0;
const std::string OPTIONS::ITEM51::itemName = "ITEM51";
const int OPTIONS::ITEM51::defaultValue = 0;
const std::string OPTIONS::ITEM52::itemName = "ITEM52";
const int OPTIONS::ITEM52::defaultValue = 0;
const std::string OPTIONS::ITEM53::itemName = "ITEM53";
const int OPTIONS::ITEM53::defaultValue = 0;
const std::string OPTIONS::ITEM54::itemName = "ITEM54";
const int OPTIONS::ITEM54::defaultValue = 0;
const std::string OPTIONS::ITEM55::itemName = "ITEM55";
const int OPTIONS::ITEM55::defaultValue = 0;
const std::string OPTIONS::ITEM56::itemName = "ITEM56";
const int OPTIONS::ITEM56::defaultValue = 0;
const std::string OPTIONS::ITEM57::itemName = "ITEM57";
const int OPTIONS::ITEM57::defaultValue = 0;
const std::string OPTIONS::ITEM58::itemName = "ITEM58";
const int OPTIONS::ITEM58::defaultValue = 0;
const std::string OPTIONS::ITEM59::itemName = "ITEM59";
const int OPTIONS::ITEM59::defaultValue = 0;
const std::string OPTIONS::ITEM60::itemName = "ITEM60";
const int OPTIONS::ITEM60::defaultValue = 0;
const std::string OPTIONS::ITEM61::itemName = "ITEM61";
const int OPTIONS::ITEM61::defaultValue = 0;
const std::string OPTIONS::ITEM62::itemName = "ITEM62";
const int OPTIONS::ITEM62::defaultValue = 0;
const std::string OPTIONS::ITEM63::itemName = "ITEM63";
const int OPTIONS::ITEM63::defaultValue = 0;
const std::string OPTIONS::ITEM64::itemName = "ITEM64";
const int OPTIONS::ITEM64::defaultValue = 0;
const std::string OPTIONS::ITEM65::itemName = "ITEM65";
const int OPTIONS::ITEM65::defaultValue = 0;
const std::string OPTIONS::ITEM66::itemName = "ITEM66";
const int OPTIONS::ITEM66::defaultValue = 0;
const std::string OPTIONS::ITEM67::itemName = "ITEM67";
const int OPTIONS::ITEM67::defaultValue = 0;
const std::string OPTIONS::ITEM68::itemName = "ITEM68";
const int OPTIONS::ITEM68::defaultValue = 0;
const std::string OPTIONS::ITEM69::itemName = "ITEM69";
const int OPTIONS::ITEM69::defaultValue = 0;
const std::string OPTIONS::ITEM70::itemName = "ITEM70";
const int OPTIONS::ITEM70::defaultValue = 0;
const std::string OPTIONS::ITEM71::itemName = "ITEM71";
const int OPTIONS::ITEM71::defaultValue = 0;
const std::string OPTIONS::ITEM72::itemName = "ITEM72";
const int OPTIONS::ITEM72::defaultValue = 0;
const std::string OPTIONS::ITEM73::itemName = "ITEM73";
const int OPTIONS::ITEM73::defaultValue = 0;
const std::string OPTIONS::ITEM74::itemName = "ITEM74";
const int OPTIONS::ITEM74::defaultValue = 0;
const std::string OPTIONS::ITEM75::itemName = "ITEM75";
const int OPTIONS::ITEM75::defaultValue = 0;
const std::string OPTIONS::ITEM76::itemName = "ITEM76";
const int OPTIONS::ITEM76::defaultValue = 0;
const std::string OPTIONS::ITEM77::itemName = "ITEM77";
const int OPTIONS::ITEM77::defaultValue = 0;
const std::string OPTIONS::ITEM78::itemName = "ITEM78";
const int OPTIONS::ITEM78::defaultValue = 0;
const std::string OPTIONS::ITEM79::itemName = "ITEM79";
const int OPTIONS::ITEM79::defaultValue = 0;
const std::string OPTIONS::ITEM80::itemName = "ITEM80";
const int OPTIONS::ITEM80::defaultValue = 0;
const std::string OPTIONS::ITEM81::itemName = "ITEM81";
const int OPTIONS::ITEM81::defaultValue = 0;
const std::string OPTIONS::ITEM82::itemName = "ITEM82";
const int OPTIONS::ITEM82::defaultValue = 0;
const std::string OPTIONS::ITEM83::itemName = "ITEM83";
const int OPTIONS::ITEM83::defaultValue = 0;
const std::string OPTIONS::ITEM84::itemName = "ITEM84";
const int OPTIONS::ITEM84::defaultValue = 0;
const std::string OPTIONS::ITEM85::itemName = "ITEM85";
const int OPTIONS::ITEM85::defaultValue = 0;
const std::string OPTIONS::ITEM86::itemName = "ITEM86";
const int OPTIONS::ITEM86::defaultValue = 0;
const std::string OPTIONS::ITEM87::itemName = "ITEM87";
const int OPTIONS::ITEM87::defaultValue = 0;
const std::string OPTIONS::ITEM88::itemName = "ITEM88";
const int OPTIONS::ITEM88::defaultValue = 0;
const std::string OPTIONS::ITEM89::itemName = "ITEM89";
const int OPTIONS::ITEM89::defaultValue = 0;
const std::string OPTIONS::ITEM90::itemName = "ITEM90";
const int OPTIONS::ITEM90::defaultValue = 0;
const std::string OPTIONS::ITEM91::itemName = "ITEM91";
const int OPTIONS::ITEM91::defaultValue = 0;
const std::string OPTIONS::ITEM92::itemName = "ITEM92";
const int OPTIONS::ITEM92::defaultValue = 0;
const std::string OPTIONS::ITEM93::itemName = "ITEM93";
const int OPTIONS::ITEM93::defaultValue = 0;
const std::string OPTIONS::ITEM94::itemName = "ITEM94";
const int OPTIONS::ITEM94::defaultValue = 0;
const std::string OPTIONS::ITEM95::itemName = "ITEM95";
const int OPTIONS::ITEM95::defaultValue = 0;
const std::string OPTIONS::ITEM96::itemName = "ITEM96";
const int OPTIONS::ITEM96::defaultValue = 0;
const std::string OPTIONS::ITEM97::itemName = "ITEM97";
const int OPTIONS::ITEM97::defaultValue = 0;
const std::string OPTIONS::ITEM98::itemName = "ITEM98";
const int OPTIONS::ITEM98::defaultValue = 0;
const std::string OPTIONS::ITEM99::itemName = "ITEM99";
const int OPTIONS::ITEM99::defaultValue = 0;
const std::string OPTIONS::ITEM100::itemName = "ITEM100";
const int OPTIONS::ITEM100::defaultValue = 0;
const std::string OPTIONS::ITEM101::itemName = "ITEM101";
const int OPTIONS::ITEM101::defaultValue = 0;
const std::string OPTIONS::ITEM102::itemName = "ITEM102";
const int OPTIONS::ITEM102::defaultValue = 0;
const std::string OPTIONS::ITEM103::itemName = "ITEM103";
const int OPTIONS::ITEM103::defaultValue = 0;
const std::string OPTIONS::ITEM104::itemName = "ITEM104";
const int OPTIONS::ITEM104::defaultValue = 0;
const std::string OPTIONS::ITEM105::itemName = "ITEM105";
const int OPTIONS::ITEM105::defaultValue = 0;
const std::string OPTIONS::ITEM106::itemName = "ITEM106";
const int OPTIONS::ITEM106::defaultValue = 0;
const std::string OPTIONS::ITEM107::itemName = "ITEM107";
const int OPTIONS::ITEM107::defaultValue = 0;
const std::string OPTIONS::ITEM108::itemName = "ITEM108";
const int OPTIONS::ITEM108::defaultValue = 0;
const std::string OPTIONS::ITEM109::itemName = "ITEM109";
const int OPTIONS::ITEM109::defaultValue = 0;
const std::string OPTIONS::ITEM110::itemName = "ITEM110";
const int OPTIONS::ITEM110::defaultValue = 0;
const std::string OPTIONS::ITEM111::itemName = "ITEM111";
const int OPTIONS::ITEM111::defaultValue = 0;
const std::string OPTIONS::ITEM112::itemName = "ITEM112";
const int OPTIONS::ITEM112::defaultValue = 0;
const std::string OPTIONS::ITEM113::itemName = "ITEM113";
const int OPTIONS::ITEM113::defaultValue = 0;
const std::string OPTIONS::ITEM114::itemName = "ITEM114";
const int OPTIONS::ITEM114::defaultValue = 0;
const std::string OPTIONS::ITEM115::itemName = "ITEM115";
const int OPTIONS::ITEM115::defaultValue = 0;
const std::string OPTIONS::ITEM116::itemName = "ITEM116";
const int OPTIONS::ITEM116::defaultValue = 0;
const std::string OPTIONS::ITEM117::itemName = "ITEM117";
const int OPTIONS::ITEM117::defaultValue = 0;
const std::string OPTIONS::ITEM118::itemName = "ITEM118";
const int OPTIONS::ITEM118::defaultValue = 0;
const std::string OPTIONS::ITEM119::itemName = "ITEM119";
const int OPTIONS::ITEM119::defaultValue = 0;
const std::string OPTIONS::ITEM120::itemName = "ITEM120";
const int OPTIONS::ITEM120::defaultValue = 0;
const std::string OPTIONS::ITEM121::itemName = "ITEM121";
const int OPTIONS::ITEM121::defaultValue = 0;
const std::string OPTIONS::ITEM122::itemName = "ITEM122";
const int OPTIONS::ITEM122::defaultValue = 0;
const std::string OPTIONS::ITEM123::itemName = "ITEM123";
const int OPTIONS::ITEM123::defaultValue = 0;
const std::string OPTIONS::ITEM124::itemName = "ITEM124";
const int OPTIONS::ITEM124::defaultValue = 0;
const std::string OPTIONS::ITEM125::itemName = "ITEM125";
const int OPTIONS::ITEM125::defaultValue = 0;
const std::string OPTIONS::ITEM126::itemName = "ITEM126";
const int OPTIONS::ITEM126::defaultValue = 0;
const std::string OPTIONS::ITEM127::itemName = "ITEM127";
const int OPTIONS::ITEM127::defaultValue = 0;
const std::string OPTIONS::ITEM128::itemName = "ITEM128";
const int OPTIONS::ITEM128::defaultValue = 0;
const std::string OPTIONS::ITEM129::itemName = "ITEM129";
const int OPTIONS::ITEM129::defaultValue = 0;
const std::string OPTIONS::ITEM130::itemName = "ITEM130";
const int OPTIONS::ITEM130::defaultValue = 0;
const std::string OPTIONS::ITEM131::itemName = "ITEM131";
const int OPTIONS::ITEM131::defaultValue = 0;
const std::string OPTIONS::ITEM132::itemName = "ITEM132";
const int OPTIONS::ITEM132::defaultValue = 0;
const std::string OPTIONS::ITEM133::itemName = "ITEM133";
const int OPTIONS::ITEM133::defaultValue = 0;
const std::string OPTIONS::ITEM134::itemName = "ITEM134";
const int OPTIONS::ITEM134::defaultValue = 0;
const std::string OPTIONS::ITEM135::itemName = "ITEM135";
const int OPTIONS::ITEM135::defaultValue = 0;
const std::string OPTIONS::ITEM136::itemName = "ITEM136";
const int OPTIONS::ITEM136::defaultValue = 0;
const std::string OPTIONS::ITEM137::itemName = "ITEM137";
const int OPTIONS::ITEM137::defaultValue = 0;
const std::string OPTIONS::ITEM138::itemName = "ITEM138";
const int OPTIONS::ITEM138::defaultValue = 0;
const std::string OPTIONS::ITEM139::itemName = "ITEM139";
const int OPTIONS::ITEM139::defaultValue = 0;
const std::string OPTIONS::ITEM140::itemName = "ITEM140";
const int OPTIONS::ITEM140::defaultValue = 0;
const std::string OPTIONS::ITEM141::itemName = "ITEM141";
const int OPTIONS::ITEM141::defaultValue = 0;
const std::string OPTIONS::ITEM142::itemName = "ITEM142";
const int OPTIONS::ITEM142::defaultValue = 0;
const std::string OPTIONS::ITEM143::itemName = "ITEM143";
const int OPTIONS::ITEM143::defaultValue = 0;
const std::string OPTIONS::ITEM144::itemName = "ITEM144";
const int OPTIONS::ITEM144::defaultValue = 0;
const std::string OPTIONS::ITEM145::itemName = "ITEM145";
const int OPTIONS::ITEM145::defaultValue = 0;
const std::string OPTIONS::ITEM146::itemName = "ITEM146";
const int OPTIONS::ITEM146::defaultValue = 0;
const std::string OPTIONS::ITEM147::itemName = "ITEM147";
const int OPTIONS::ITEM147::defaultValue = 0;
const std::string OPTIONS::ITEM148::itemName = "ITEM148";
const int OPTIONS::ITEM148::defaultValue = 0;
const std::string OPTIONS::ITEM149::itemName = "ITEM149";
const int OPTIONS::ITEM149::defaultValue = 0;
const std::string OPTIONS::ITEM150::itemName = "ITEM150";
const int OPTIONS::ITEM150::defaultValue = 0;
const std::string OPTIONS::ITEM151::itemName = "ITEM151";
const int OPTIONS::ITEM151::defaultValue = 0;
const std::string OPTIONS::ITEM152::itemName = "ITEM152";
const int OPTIONS::ITEM152::defaultValue = 0;
const std::string OPTIONS::ITEM153::itemName = "ITEM153";
const int OPTIONS::ITEM153::defaultValue = 0;
const std::string OPTIONS::ITEM154::itemName = "ITEM154";
const int OPTIONS::ITEM154::defaultValue = 0;
const std::string OPTIONS::ITEM155::itemName = "ITEM155";
const int OPTIONS::ITEM155::defaultValue = 0;
const std::string OPTIONS::ITEM156::itemName = "ITEM156";
const int OPTIONS::ITEM156::defaultValue = 0;
const std::string OPTIONS::ITEM157::itemName = "ITEM157";
const int OPTIONS::ITEM157::defaultValue = 0;
const std::string OPTIONS::ITEM158::itemName = "ITEM158";
const int OPTIONS::ITEM158::defaultValue = 0;
const std::string OPTIONS::ITEM159::itemName = "ITEM159";
const int OPTIONS::ITEM159::defaultValue = 0;
const std::string OPTIONS::ITEM160::itemName = "ITEM160";
const int OPTIONS::ITEM160::defaultValue = 0;
const std::string OPTIONS::ITEM161::itemName = "ITEM161";
const int OPTIONS::ITEM161::defaultValue = 0;
const std::string OPTIONS::ITEM162::itemName = "ITEM162";
const int OPTIONS::ITEM162::defaultValue = 0;
const std::string OPTIONS::ITEM163::itemName = "ITEM163";
const int OPTIONS::ITEM163::defaultValue = 0;
const std::string OPTIONS::ITEM164::itemName = "ITEM164";
const int OPTIONS::ITEM164::defaultValue = 0;
const std::string OPTIONS::ITEM165::itemName = "ITEM165";
const int OPTIONS::ITEM165::defaultValue = 0;
const std::string OPTIONS::ITEM166::itemName = "ITEM166";
const int OPTIONS::ITEM166::defaultValue = 0;
const std::string OPTIONS::ITEM167::itemName = "ITEM167";
const int OPTIONS::ITEM167::defaultValue = 0;
const std::string OPTIONS::ITEM168::itemName = "ITEM168";
const int OPTIONS::ITEM168::defaultValue = 0;
const std::string OPTIONS::ITEM169::itemName = "ITEM169";
const int OPTIONS::ITEM169::defaultValue = 0;
const std::string OPTIONS::ITEM170::itemName = "ITEM170";
const int OPTIONS::ITEM170::defaultValue = 0;
const std::string OPTIONS::ITEM171::itemName = "ITEM171";
const int OPTIONS::ITEM171::defaultValue = 0;
const std::string OPTIONS::ITEM172::itemName = "ITEM172";
const int OPTIONS::ITEM172::defaultValue = 0;
const std::string OPTIONS::ITEM173::itemName = "ITEM173";
const int OPTIONS::ITEM173::defaultValue = 0;
const std::string OPTIONS::ITEM174::itemName = "ITEM174";
const int OPTIONS::ITEM174::defaultValue = 0;
const std::string OPTIONS::ITEM175::itemName = "ITEM175";
const int OPTIONS::ITEM175::defaultValue = 0;
const std::string OPTIONS::ITEM176::itemName = "ITEM176";
const int OPTIONS::ITEM176::defaultValue = 0;
const std::string OPTIONS::ITEM177::itemName = "ITEM177";
const int OPTIONS::ITEM177::defaultValue = 0;
const std::string OPTIONS::ITEM178::itemName = "ITEM178";
const int OPTIONS::ITEM178::defaultValue = 0;
const std::string OPTIONS::ITEM179::itemName = "ITEM179";
const int OPTIONS::ITEM179::defaultValue = 0;
const std::string OPTIONS::ITEM180::itemName = "ITEM180";
const int OPTIONS::ITEM180::defaultValue = 0;
const std::string OPTIONS::ITEM181::itemName = "ITEM181";
const int OPTIONS::ITEM181::defaultValue = 0;
const std::string OPTIONS::ITEM182::itemName = "ITEM182";
const int OPTIONS::ITEM182::defaultValue = 0;
const std::string OPTIONS::ITEM183::itemName = "ITEM183";
const int OPTIONS::ITEM183::defaultValue = 0;
const std::string OPTIONS::ITEM184::itemName = "ITEM184";
const int OPTIONS::ITEM184::defaultValue = 0;
const std::string OPTIONS::ITEM185::itemName = "ITEM185";
const int OPTIONS::ITEM185::defaultValue = 0;
const std::string OPTIONS::ITEM186::itemName = "ITEM186";
const int OPTIONS::ITEM186::defaultValue = 0;
const std::string OPTIONS::ITEM187::itemName = "ITEM187";
const int OPTIONS::ITEM187::defaultValue = 0;
const std::string OPTIONS::ITEM188::itemName = "ITEM188";
const int OPTIONS::ITEM188::defaultValue = 0;
const std::string OPTIONS::ITEM189::itemName = "ITEM189";
const int OPTIONS::ITEM189::defaultValue = 0;
const std::string OPTIONS::ITEM190::itemName = "ITEM190";
const int OPTIONS::ITEM190::defaultValue = 0;
const std::string OPTIONS::ITEM191::itemName = "ITEM191";
const int OPTIONS::ITEM191::defaultValue = 0;
const std::string OPTIONS::ITEM192::itemName = "ITEM192";
const int OPTIONS::ITEM192::defaultValue = 0;
const std::string OPTIONS::ITEM193::itemName = "ITEM193";
const int OPTIONS::ITEM193::defaultValue = 0;
const std::string OPTIONS::ITEM194::itemName = "ITEM194";
const int OPTIONS::ITEM194::defaultValue = 0;
const std::string OPTIONS::ITEM195::itemName = "ITEM195";
const int OPTIONS::ITEM195::defaultValue = 0;
const std::string OPTIONS::ITEM196::itemName = "ITEM196";
const int OPTIONS::ITEM196::defaultValue = 0;
const std::string OPTIONS::ITEM197::itemName = "ITEM197";
const int OPTIONS::ITEM197::defaultValue = 0;


PARALLEL::PARALLEL( ) : ParserKeyword("PARALLEL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PARALLEL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NDMAIN",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("MACHINE_TYPE",Opm::SINGLE,"DISTRIBUTED"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PARALLEL::keywordName = "PARALLEL";
const std::string PARALLEL::NDMAIN::itemName = "NDMAIN";
const int PARALLEL::NDMAIN::defaultValue = 1;
const std::string PARALLEL::MACHINE_TYPE::itemName = "MACHINE_TYPE";
const std::string PARALLEL::MACHINE_TYPE::defaultValue = "DISTRIBUTED";


PATHS::PATHS( ) : ParserKeyword("PATHS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PATHS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("PathName",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PathValue",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PATHS::keywordName = "PATHS";
const std::string PATHS::PathName::itemName = "PathName";
const std::string PATHS::PathValue::itemName = "PathValue";


PBVD::PBVD( ) : ParserKeyword("PBVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PBVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PBVD::keywordName = "PBVD";
const std::string PBVD::table::itemName = "table";


PCG::PCG( ) : ParserKeyword("PCG") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PCG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PCG::keywordName = "PCG";
const std::string PCG::data::itemName = "data";


PERFORMANCE_PROBE::PERFORMANCE_PROBE( ) : ParserKeyword("PERFORMANCE_PROBE") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
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


PERMX::PERMX( ) : ParserKeyword("PERMX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMX::keywordName = "PERMX";
const std::string PERMX::data::itemName = "data";


PERMXY::PERMXY( ) : ParserKeyword("PERMXY") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMXY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMXY::keywordName = "PERMXY";
const std::string PERMXY::data::itemName = "data";


PERMY::PERMY( ) : ParserKeyword("PERMY") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL,0));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMY::keywordName = "PERMY";
const std::string PERMY::data::itemName = "data";
const double PERMY::data::defaultValue = 0;


PERMYZ::PERMYZ( ) : ParserKeyword("PERMYZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMYZ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMYZ::keywordName = "PERMYZ";
const std::string PERMYZ::data::itemName = "data";


PERMZ::PERMZ( ) : ParserKeyword("PERMZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMZ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL,0));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMZ::keywordName = "PERMZ";
const std::string PERMZ::data::itemName = "data";
const double PERMZ::data::defaultValue = 0;


PERMZX::PERMZX( ) : ParserKeyword("PERMZX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PERMZX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Permeability");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PERMZX::keywordName = "PERMZX";
const std::string PERMZX::data::itemName = "data";


PIMTDIMS::PIMTDIMS( ) : ParserKeyword("PIMTDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("PIMTDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NTPIMT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NPPIMT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PIMTDIMS::keywordName = "PIMTDIMS";
const std::string PIMTDIMS::NTPIMT::itemName = "NTPIMT";
const int PIMTDIMS::NTPIMT::defaultValue = 0;
const std::string PIMTDIMS::NPPIMT::itemName = "NPPIMT";
const int PIMTDIMS::NPPIMT::defaultValue = 0;


PIMULTAB::PIMULTAB( ) : ParserKeyword("PIMULTAB") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("PIMTDIMS","NTPIMT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("PIMULTAB");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TABLE",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PIMULTAB::keywordName = "PIMULTAB";
const std::string PIMULTAB::TABLE::itemName = "TABLE";


PINCH::PINCH( ) : ParserKeyword("PINCH") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PINCH");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("THRESHOLD_THICKNESS",Opm::SINGLE,0.001));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CONTROL_OPTION",Opm::SINGLE,"GAP"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_EMPTY_GAP",Opm::SINGLE,1e+20));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PINCHOUT_OPTION",Opm::SINGLE,"TOPBOT"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("MULTZ_OPTION",Opm::SINGLE,"TOP"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PINCH::keywordName = "PINCH";
const std::string PINCH::THRESHOLD_THICKNESS::itemName = "THRESHOLD_THICKNESS";
const double PINCH::THRESHOLD_THICKNESS::defaultValue = 0.001;
const std::string PINCH::CONTROL_OPTION::itemName = "CONTROL_OPTION";
const std::string PINCH::CONTROL_OPTION::defaultValue = "GAP";
const std::string PINCH::MAX_EMPTY_GAP::itemName = "MAX_EMPTY_GAP";
const double PINCH::MAX_EMPTY_GAP::defaultValue = 1e+20;
const std::string PINCH::PINCHOUT_OPTION::itemName = "PINCHOUT_OPTION";
const std::string PINCH::PINCHOUT_OPTION::defaultValue = "TOPBOT";
const std::string PINCH::MULTZ_OPTION::itemName = "MULTZ_OPTION";
const std::string PINCH::MULTZ_OPTION::defaultValue = "TOP";


PLMIXPAR::PLMIXPAR( ) : ParserKeyword("PLMIXPAR") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NPLMIX");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLMIXPAR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TODD_LONGSTAFF",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLMIXPAR::keywordName = "PLMIXPAR";
const std::string PLMIXPAR::TODD_LONGSTAFF::itemName = "TODD_LONGSTAFF";


PLYADS::PLYADS( ) : ParserKeyword("PLYADS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYADS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYADS::keywordName = "PLYADS";
const std::string PLYADS::DATA::itemName = "DATA";


PLYADSS::PLYADSS( ) : ParserKeyword("PLYADSS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
setTableCollection( true );
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PLYADSS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("POLYMER_C",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("POLYMER_ADS_C",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYADSS::keywordName = "PLYADSS";
const std::string PLYADSS::POLYMER_C::itemName = "POLYMER_C";
const std::string PLYADSS::POLYMER_ADS_C::itemName = "POLYMER_ADS_C";
const std::string PLYADSS::DATA::itemName = "DATA";


PLYDHFLF::PLYDHFLF( ) : ParserKeyword("PLYDHFLF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYDHFLF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Temperature");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYDHFLF::keywordName = "PLYDHFLF";
const std::string PLYDHFLF::DATA::itemName = "DATA";


PLYMAX::PLYMAX( ) : ParserKeyword("PLYMAX") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("REGDIMS","NPLMIX");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYMAX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_POLYMER_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_SALT_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYMAX::keywordName = "PLYMAX";
const std::string PLYMAX::MAX_POLYMER_CONCENTRATION::itemName = "MAX_POLYMER_CONCENTRATION";
const std::string PLYMAX::MAX_SALT_CONCENTRATION::itemName = "MAX_SALT_CONCENTRATION";


PLYROCK::PLYROCK( ) : ParserKeyword("PLYROCK") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYROCK");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("IPV",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RRF",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ROCK_DENSITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("AI",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("MAX_ADSORPTION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYROCK::keywordName = "PLYROCK";
const std::string PLYROCK::IPV::itemName = "IPV";
const std::string PLYROCK::RRF::itemName = "RRF";
const std::string PLYROCK::ROCK_DENSITY::itemName = "ROCK_DENSITY";
const std::string PLYROCK::AI::itemName = "AI";
const double PLYROCK::AI::defaultValue = 1;
const std::string PLYROCK::MAX_ADSORPTION::itemName = "MAX_ADSORPTION";


PLYSHEAR::PLYSHEAR( ) : ParserKeyword("PLYSHEAR") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYSHEAR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_VELOCITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VRF",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYSHEAR::keywordName = "PLYSHEAR";
const std::string PLYSHEAR::WATER_VELOCITY::itemName = "WATER_VELOCITY";
const std::string PLYSHEAR::VRF::itemName = "VRF";


PLYSHLOG::PLYSHLOG( ) : ParserKeyword("PLYSHLOG") {
  setFixedSize( (size_t) 2);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYSHLOG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("REF_POLYMER_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("REF_SALINITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Salinity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("REF_TEMPERATURE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Temperature");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYSHLOG::keywordName = "PLYSHLOG";
const std::string PLYSHLOG::REF_POLYMER_CONCENTRATION::itemName = "REF_POLYMER_CONCENTRATION";
const std::string PLYSHLOG::REF_SALINITY::itemName = "REF_SALINITY";
const std::string PLYSHLOG::REF_TEMPERATURE::itemName = "REF_TEMPERATURE";
const std::string PLYSHLOG::DATA::itemName = "DATA";


PLYVISC::PLYVISC( ) : ParserKeyword("PLYVISC") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("PLYVISC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PLYVISC::keywordName = "PLYVISC";
const std::string PLYVISC::DATA::itemName = "DATA";


PMISC::PMISC( ) : ParserKeyword("PMISC") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PMISC");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PMISC::keywordName = "PMISC";
const std::string PMISC::DATA::itemName = "DATA";


POLYMER::POLYMER( ) : ParserKeyword("POLYMER") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("POLYMER");
}
const std::string POLYMER::keywordName = "POLYMER";


PORO::PORO( ) : ParserKeyword("PORO") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("PORO");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL,0));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PORO::keywordName = "PORO";
const std::string PORO::data::itemName = "data";
const double PORO::data::defaultValue = 0;


PORV::PORV( ) : ParserKeyword("PORV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("PORV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("ReservoirVolume");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PORV::keywordName = "PORV";
const std::string PORV::data::itemName = "data";


PREF::PREF( ) : ParserKeyword("PREF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PREF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("PRESSURE",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PREF::keywordName = "PREF";
const std::string PREF::PRESSURE::itemName = "PRESSURE";


PREFS::PREFS( ) : ParserKeyword("PREFS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PREFS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("PRESSURE",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PREFS::keywordName = "PREFS";
const std::string PREFS::PRESSURE::itemName = "PRESSURE";


PRESSURE::PRESSURE( ) : ParserKeyword("PRESSURE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("PRESSURE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PRESSURE::keywordName = "PRESSURE";
const std::string PRESSURE::data::itemName = "data";


PROPS::PROPS( ) : ParserKeyword("PROPS") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("PROPS");
}
const std::string PROPS::keywordName = "PROPS";


PVCDO::PVCDO( ) : ParserKeyword("PVCDO") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVCDO");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("P_REF",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_VOL_FACTOR",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_COMPRESSIBILITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_VISCOSITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_VISCOSIBILITY",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
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


PVDG::PVDG( ) : ParserKeyword("PVDG") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("OilDissolutionFactor");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDG::keywordName = "PVDG";
const std::string PVDG::data::itemName = "data";


PVDO::PVDO( ) : ParserKeyword("PVDO") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDO");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("1");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDO::keywordName = "PVDO";
const std::string PVDO::data::itemName = "data";


PVDS::PVDS( ) : ParserKeyword("PVDS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVDS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("OilDissolutionFactor");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVDS::keywordName = "PVDS";
const std::string PVDS::data::itemName = "data";


PVTG::PVTG( ) : ParserKeyword("PVTG") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
setTableCollection( true );
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("GAS_PRESSURE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("OilDissolutionFactor");
        item->push_backDimension("OilDissolutionFactor");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTG::keywordName = "PVTG";
const std::string PVTG::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string PVTG::DATA::itemName = "DATA";


PVTNUM::PVTNUM( ) : ParserKeyword("PVTNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("PVTNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string PVTNUM::keywordName = "PVTNUM";
const std::string PVTNUM::data::itemName = "data";


PVTO::PVTO( ) : ParserKeyword("PVTO") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
setTableCollection( true );
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTO");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("RS",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("GasDissolutionFactor");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("1");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTO::keywordName = "PVTO";
const std::string PVTO::RS::itemName = "RS";
const std::string PVTO::DATA::itemName = "DATA";


PVTW::PVTW( ) : ParserKeyword("PVTW") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("PVTW");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("P_REF",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_VOL_FACTOR",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_COMPRESSIBILITY",Opm::SINGLE,4.0000000000000003e-05));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_VISCOSITY",Opm::SINGLE,0.5));
        item->setDescription("");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WATER_VISCOSIBILITY",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string PVTW::keywordName = "PVTW";
const std::string PVTW::P_REF::itemName = "P_REF";
const std::string PVTW::WATER_VOL_FACTOR::itemName = "WATER_VOL_FACTOR";
const double PVTW::WATER_VOL_FACTOR::defaultValue = 1;
const std::string PVTW::WATER_COMPRESSIBILITY::itemName = "WATER_COMPRESSIBILITY";
const double PVTW::WATER_COMPRESSIBILITY::defaultValue = 4e-05;
const std::string PVTW::WATER_VISCOSITY::itemName = "WATER_VISCOSITY";
const double PVTW::WATER_VISCOSITY::defaultValue = 0.5;
const std::string PVTW::WATER_VISCOSIBILITY::itemName = "WATER_VISCOSIBILITY";
const double PVTW::WATER_VISCOSIBILITY::defaultValue = 0;


RADFIN4::RADFIN4( ) : ParserKeyword("RADFIN4") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("RADFIN4");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("NAME",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NR",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTHETA",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NZ",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NWMAX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
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


REGDIMS::REGDIMS( ) : ParserKeyword("REGDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("REGDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NTFIP",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NMFIPR",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NRFREG",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTFREG",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ETRACK",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTCREG",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_OPERNUM",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_OPERATE_DWORK",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_OPERATE_IWORK",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NPLMIX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
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


REGIONS::REGIONS( ) : ParserKeyword("REGIONS") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("REGIONS");
}
const std::string REGIONS::keywordName = "REGIONS";


REGION_PROBE::REGION_PROBE( ) : ParserKeyword("REGION_PROBE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RAPI");
  addDeckName("RCAD");
  addDeckName("RCFT");
  addDeckName("RCGC");
  addDeckName("RCIP");
  addDeckName("RCSC");
  addDeckName("RGDEN");
  addDeckName("RGFT");
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
  addDeckName("ROFT");
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
  addDeckName("RWFT");
  addDeckName("RWIP");
  addDeckName("RWIR");
  addDeckName("RWIT");
  addDeckName("RWP");
  addDeckName("RWPR");
  addDeckName("RWPT");
  addDeckName("RWPV");
  addDeckName("RWSAT");
  addDeckName("RWVIS");
  setMatchRegex("R[OGW]?[IP][PRT]_.+|RU.+|RTIPF.+|RTIPS.+|RTFTF.+|RTFTS.+|RTFTT.+|RTIPT.+|RTIPF.+|RTIPS.+|RTIP[1-9][0-9]*.+|RTFTT.+|RTFTF.+|RTFTS.+|RTFT[1-9][0-9]*.+|RTADS.+|RTDCY.+");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("REGIONS",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string REGION_PROBE::keywordName = "REGION_PROBE";
const std::string REGION_PROBE::REGIONS::itemName = "REGIONS";


RESTART::RESTART( ) : ParserKeyword("RESTART") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RESTART");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("ROOTNAME",Opm::SINGLE,"BASE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REPORTNUMBER",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("SAVEFILE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("SAVEFILE_FORMAT",Opm::SINGLE,"UNFORMATTED"));
        item->setDescription("");
        record->addItem(item);
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


RKTRMDIR::RKTRMDIR( ) : ParserKeyword("RKTRMDIR") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RKTRMDIR");
}
const std::string RKTRMDIR::keywordName = "RKTRMDIR";


ROCK::ROCK( ) : ParserKeyword("ROCK") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCK");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("PREF",Opm::SINGLE,1.0132000000000001));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("COMPRESSIBILITY",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("1/Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCK::keywordName = "ROCK";
const std::string ROCK::PREF::itemName = "PREF";
const double ROCK::PREF::defaultValue = 1.0132;
const std::string ROCK::COMPRESSIBILITY::itemName = "COMPRESSIBILITY";
const double ROCK::COMPRESSIBILITY::defaultValue = 0;


ROCKCOMP::ROCKCOMP( ) : ParserKeyword("ROCKCOMP") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ROCKCOMP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("HYSTERESIS",Opm::SINGLE,"REVERS"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTROCC",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("WATER_COMPACTION",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
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


ROCKOPTS::ROCKOPTS( ) : ParserKeyword("ROCKOPTS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKOPTS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("METHOD",Opm::SINGLE,"PRESSURE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("REF_PRESSURE",Opm::SINGLE,"NOSTORE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TABLE_TYPE",Opm::SINGLE,"PVTNUM"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("HYST_TYPE",Opm::SINGLE,"DEFLATION"));
        item->setDescription("");
        record->addItem(item);
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


ROCKTAB::ROCKTAB( ) : ParserKeyword("ROCKTAB") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("ROCKCOMP","NTROCC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ROCKTAB");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ROCKTAB::keywordName = "ROCKTAB";
const std::string ROCKTAB::DATA::itemName = "DATA";


RPTGRID::RPTGRID( ) : ParserKeyword("RPTGRID") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("RPTGRID");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("DATA",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTGRID::keywordName = "RPTGRID";
const std::string RPTGRID::DATA::itemName = "DATA";


RPTONLY::RPTONLY( ) : ParserKeyword("RPTONLY") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RPTONLY");
}
const std::string RPTONLY::keywordName = "RPTONLY";


RPTONLYO::RPTONLYO( ) : ParserKeyword("RPTONLYO") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RPTONLYO");
}
const std::string RPTONLYO::keywordName = "RPTONLYO";


RPTPROPS::RPTPROPS( ) : ParserKeyword("RPTPROPS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("RPTPROPS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("mnemonics",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTPROPS::keywordName = "RPTPROPS";
const std::string RPTPROPS::mnemonics::itemName = "mnemonics";


RPTRST::RPTRST( ) : ParserKeyword("RPTRST") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RPTRST");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("MNEMONIC_LIST",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTRST::keywordName = "RPTRST";
const std::string RPTRST::MNEMONIC_LIST::itemName = "MNEMONIC_LIST";


RPTRUNSP::RPTRUNSP( ) : ParserKeyword("RPTRUNSP") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("RPTRUNSP");
}
const std::string RPTRUNSP::keywordName = "RPTRUNSP";


RPTSCHED::RPTSCHED( ) : ParserKeyword("RPTSCHED") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("RPTSCHED");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("MNEMONIC_LIST",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTSCHED::keywordName = "RPTSCHED";
const std::string RPTSCHED::MNEMONIC_LIST::itemName = "MNEMONIC_LIST";


RPTSOL::RPTSOL( ) : ParserKeyword("RPTSOL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RPTSOL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("mnemonics",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RPTSOL::keywordName = "RPTSOL";
const std::string RPTSOL::mnemonics::itemName = "mnemonics";


RS::RS( ) : ParserKeyword("RS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("GasDissolutionFactor");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string RS::keywordName = "RS";
const std::string RS::data::itemName = "data";


RSVD::RSVD( ) : ParserKeyword("RSVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RSVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("GasDissolutionFactor");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RSVD::keywordName = "RSVD";
const std::string RSVD::table::itemName = "table";


RTEMPVD::RTEMPVD( ) : ParserKeyword("RTEMPVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RTEMPVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("Temperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RTEMPVD::keywordName = "RTEMPVD";
const std::string RTEMPVD::DATA::itemName = "DATA";


RUNSPEC::RUNSPEC( ) : ParserKeyword("RUNSPEC") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("RUNSPEC");
}
const std::string RUNSPEC::keywordName = "RUNSPEC";


RUNSUM::RUNSUM( ) : ParserKeyword("RUNSUM") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("RUNSUM");
}
const std::string RUNSUM::keywordName = "RUNSUM";


RV::RV( ) : ParserKeyword("RV") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RV");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("OilDissolutionFactor");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string RV::keywordName = "RV";
const std::string RV::data::itemName = "data";


RVVD::RVVD( ) : ParserKeyword("RVVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("RVVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("OilDissolutionFactor");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string RVVD::keywordName = "RVVD";
const std::string RVVD::table::itemName = "table";


SATNUM::SATNUM( ) : ParserKeyword("SATNUM") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("SATNUM");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SATNUM::keywordName = "SATNUM";
const std::string SATNUM::data::itemName = "data";


SATOPTS::SATOPTS( ) : ParserKeyword("SATOPTS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SATOPTS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("options",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SATOPTS::keywordName = "SATOPTS";
const std::string SATOPTS::options::itemName = "options";


SAVE::SAVE( ) : ParserKeyword("SAVE") {
  setSizeType(UNKNOWN);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SAVE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("FILE_TYPE",Opm::SINGLE,"UNFORMATTED"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SAVE::keywordName = "SAVE";
const std::string SAVE::FILE_TYPE::itemName = "FILE_TYPE";
const std::string SAVE::FILE_TYPE::defaultValue = "UNFORMATTED";


SCALECRS::SCALECRS( ) : ParserKeyword("SCALECRS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SCALECRS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("VALUE",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCALECRS::keywordName = "SCALECRS";
const std::string SCALECRS::VALUE::itemName = "VALUE";
const std::string SCALECRS::VALUE::defaultValue = "NO";


SCHEDULE::SCHEDULE( ) : ParserKeyword("SCHEDULE") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("SCHEDULE");
}
const std::string SCHEDULE::keywordName = "SCHEDULE";


SDENSITY::SDENSITY( ) : ParserKeyword("SDENSITY") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SDENSITY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("SOLVENT_DENSITY",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("Density");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SDENSITY::keywordName = "SDENSITY";
const std::string SDENSITY::SOLVENT_DENSITY::itemName = "SOLVENT_DENSITY";
const double SDENSITY::SOLVENT_DENSITY::defaultValue = 1;


SEPARATE::SEPARATE( ) : ParserKeyword("SEPARATE") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SEPARATE");
}
const std::string SEPARATE::keywordName = "SEPARATE";


SGAS::SGAS( ) : ParserKeyword("SGAS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SGAS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SGAS::keywordName = "SGAS";
const std::string SGAS::data::itemName = "data";


SGCR::SGCR( ) : ParserKeyword("SGCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SGCR::keywordName = "SGCR";
const std::string SGCR::data::itemName = "data";


SGCWMIS::SGCWMIS( ) : ParserKeyword("SGCWMIS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGCWMIS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGCWMIS::keywordName = "SGCWMIS";
const std::string SGCWMIS::DATA::itemName = "DATA";


SGFN::SGFN( ) : ParserKeyword("SGFN") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGFN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGFN::keywordName = "SGFN";
const std::string SGFN::DATA::itemName = "DATA";


SGL::SGL( ) : ParserKeyword("SGL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SGL::keywordName = "SGL";
const std::string SGL::data::itemName = "data";


SGOF::SGOF( ) : ParserKeyword("SGOF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGOF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGOF::keywordName = "SGOF";
const std::string SGOF::table::itemName = "table";


SGU::SGU( ) : ParserKeyword("SGU") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGU");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SGU::keywordName = "SGU";
const std::string SGU::data::itemName = "data";


SGWFN::SGWFN( ) : ParserKeyword("SGWFN") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGWFN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGWFN::keywordName = "SGWFN";
const std::string SGWFN::DATA::itemName = "DATA";


SHRATE::SHRATE( ) : ParserKeyword("SHRATE") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SHRATE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("SHEAR_RATE",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SHRATE::keywordName = "SHRATE";
const std::string SHRATE::SHEAR_RATE::itemName = "SHEAR_RATE";


SKIP::SKIP( ) : ParserKeyword("SKIP") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SKIP");
}
const std::string SKIP::keywordName = "SKIP";


SKIP100::SKIP100( ) : ParserKeyword("SKIP100") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SKIP100");
}
const std::string SKIP100::keywordName = "SKIP100";


SKIP300::SKIP300( ) : ParserKeyword("SKIP300") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SKIP300");
}
const std::string SKIP300::keywordName = "SKIP300";


SKIPREST::SKIPREST( ) : ParserKeyword("SKIPREST") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SKIPREST");
}
const std::string SKIPREST::keywordName = "SKIPREST";


SLGOF::SLGOF( ) : ParserKeyword("SLGOF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SLGOF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SLGOF::keywordName = "SLGOF";
const std::string SLGOF::DATA::itemName = "DATA";


SMRYDIMS::SMRYDIMS( ) : ParserKeyword("SMRYDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SMRYDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("DIMS",Opm::SINGLE,10000));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SMRYDIMS::keywordName = "SMRYDIMS";
const std::string SMRYDIMS::DIMS::itemName = "DIMS";
const int SMRYDIMS::DIMS::defaultValue = 10000;


SOF2::SOF2( ) : ParserKeyword("SOF2") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOF2");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOF2::keywordName = "SOF2";
const std::string SOF2::DATA::itemName = "DATA";


SOF3::SOF3( ) : ParserKeyword("SOF3") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOF3");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOF3::keywordName = "SOF3";
const std::string SOF3::DATA::itemName = "DATA";


SOGCR::SOGCR( ) : ParserKeyword("SOGCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOGCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SOGCR::keywordName = "SOGCR";
const std::string SOGCR::data::itemName = "data";


SOIL::SOIL( ) : ParserKeyword("SOIL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SOIL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SOIL::keywordName = "SOIL";
const std::string SOIL::data::itemName = "data";


SOLUTION::SOLUTION( ) : ParserKeyword("SOLUTION") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("SOLUTION");
}
const std::string SOLUTION::keywordName = "SOLUTION";


SOLVENT::SOLVENT( ) : ParserKeyword("SOLVENT") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SOLVENT");
}
const std::string SOLVENT::keywordName = "SOLVENT";


SORWMIS::SORWMIS( ) : ParserKeyword("SORWMIS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SORWMIS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SORWMIS::keywordName = "SORWMIS";
const std::string SORWMIS::DATA::itemName = "DATA";


SOWCR::SOWCR( ) : ParserKeyword("SOWCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOWCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SOWCR::keywordName = "SOWCR";
const std::string SOWCR::data::itemName = "data";


SPECGRID::SPECGRID( ) : ParserKeyword("SPECGRID") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SPECGRID");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NY",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NZ",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUMRES",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("COORD_TYPE",Opm::SINGLE,"F"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECGRID::keywordName = "SPECGRID";
const std::string SPECGRID::NX::itemName = "NX";
const int SPECGRID::NX::defaultValue = 1;
const std::string SPECGRID::NY::itemName = "NY";
const int SPECGRID::NY::defaultValue = 1;
const std::string SPECGRID::NZ::itemName = "NZ";
const int SPECGRID::NZ::defaultValue = 1;
const std::string SPECGRID::NUMRES::itemName = "NUMRES";
const int SPECGRID::NUMRES::defaultValue = 1;
const std::string SPECGRID::COORD_TYPE::itemName = "COORD_TYPE";
const std::string SPECGRID::COORD_TYPE::defaultValue = "F";


SPECHEAT::SPECHEAT( ) : ParserKeyword("SPECHEAT") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SPECHEAT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECHEAT::keywordName = "SPECHEAT";
const std::string SPECHEAT::DATA::itemName = "DATA";


SPECROCK::SPECROCK( ) : ParserKeyword("SPECROCK") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SPECROCK");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECROCK::keywordName = "SPECROCK";
const std::string SPECROCK::DATA::itemName = "DATA";


SSFN::SSFN( ) : ParserKeyword("SSFN") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSFN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SSFN::keywordName = "SSFN";
const std::string SSFN::DATA::itemName = "DATA";


SSOL::SSOL( ) : ParserKeyword("SSOL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SSOL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SSOL::keywordName = "SSOL";
const std::string SSOL::data::itemName = "data";


START::START( ) : ParserKeyword("START") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("START");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("DAY",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("MONTH",Opm::SINGLE,"JAN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("YEAR",Opm::SINGLE,1983));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TIME",Opm::SINGLE,"00:00:00.000"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string START::keywordName = "START";
const std::string START::DAY::itemName = "DAY";
const int START::DAY::defaultValue = 1;
const std::string START::MONTH::itemName = "MONTH";
const std::string START::MONTH::defaultValue = "JAN";
const std::string START::YEAR::itemName = "YEAR";
const int START::YEAR::defaultValue = 1983;
const std::string START::TIME::itemName = "TIME";
const std::string START::TIME::defaultValue = "00:00:00.000";


STCOND::STCOND( ) : ParserKeyword("STCOND") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STCOND");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TEMPERATURE",Opm::SINGLE,60));
        item->setDescription("");
        item->push_backDimension("Temperature");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PRESSURE",Opm::SINGLE,14.6959));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string STCOND::keywordName = "STCOND";
const std::string STCOND::TEMPERATURE::itemName = "TEMPERATURE";
const double STCOND::TEMPERATURE::defaultValue = 60;
const std::string STCOND::PRESSURE::itemName = "PRESSURE";
const double STCOND::PRESSURE::defaultValue = 14.6959;


STONE1::STONE1( ) : ParserKeyword("STONE1") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE1");
}
const std::string STONE1::keywordName = "STONE1";


STONE1EX::STONE1EX( ) : ParserKeyword("STONE1EX") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE1EX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("EXP_VALUE",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string STONE1EX::keywordName = "STONE1EX";
const std::string STONE1EX::EXP_VALUE::itemName = "EXP_VALUE";
const double STONE1EX::EXP_VALUE::defaultValue = 1;


SUMMARY::SUMMARY( ) : ParserKeyword("SUMMARY") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  clearDeckNames();
  addDeckName("SUMMARY");
}
const std::string SUMMARY::keywordName = "SUMMARY";


SUMTHIN::SUMTHIN( ) : ParserKeyword("SUMTHIN") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SUMTHIN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TIME",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SUMTHIN::keywordName = "SUMTHIN";
const std::string SUMTHIN::TIME::itemName = "TIME";


SWAT::SWAT( ) : ParserKeyword("SWAT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SWAT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SWAT::keywordName = "SWAT";
const std::string SWAT::data::itemName = "data";


SWATINIT::SWATINIT( ) : ParserKeyword("SWATINIT") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWATINIT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SWATINIT::keywordName = "SWATINIT";
const std::string SWATINIT::data::itemName = "data";


SWCR::SWCR( ) : ParserKeyword("SWCR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWCR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SWCR::keywordName = "SWCR";
const std::string SWCR::data::itemName = "data";


SWFN::SWFN( ) : ParserKeyword("SWFN") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWFN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWFN::keywordName = "SWFN";
const std::string SWFN::DATA::itemName = "DATA";


SWL::SWL( ) : ParserKeyword("SWL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SWL::keywordName = "SWL";
const std::string SWL::data::itemName = "data";


SWOF::SWOF( ) : ParserKeyword("SWOF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWOF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("1");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWOF::keywordName = "SWOF";
const std::string SWOF::DATA::itemName = "DATA";


SWU::SWU( ) : ParserKeyword("SWU") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWU");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SWU::keywordName = "SWU";
const std::string SWU::data::itemName = "data";


TABDIMS::TABDIMS( ) : ParserKeyword("TABDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TABDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NTSFUN",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTPVT",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NSSFUN",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NPPVT",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTFIP",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NRPVT",Opm::SINGLE,20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_RV_NODES",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTENDP",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_STATE_EQ",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_EOS_RES",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_EOS_SURFACE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_FLUX_REGIONS",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_THERMAL_REGIONS",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTROCC",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_PRESSURE_MAINTAINANCE_REGIONS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_KVALUE_TABLES",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NTALPHA",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPKDAM_MAX_ROWS",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPREWG_MAX_ROWS",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPVISO_MAX_ROWS",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ITEM20_NOT_USED",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPPW2D_MAX_COLUMNS",Opm::SINGLE,5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPPW2D_MAX_ROWS",Opm::SINGLE,5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ASPHALTENE_ASPWETF_MAX_ROWS",Opm::SINGLE,5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_KVALUE_TABLES",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("RESERVED",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TABDIMS::keywordName = "TABDIMS";
const std::string TABDIMS::NTSFUN::itemName = "NTSFUN";
const int TABDIMS::NTSFUN::defaultValue = 1;
const std::string TABDIMS::NTPVT::itemName = "NTPVT";
const int TABDIMS::NTPVT::defaultValue = 1;
const std::string TABDIMS::NSSFUN::itemName = "NSSFUN";
const int TABDIMS::NSSFUN::defaultValue = 20;
const std::string TABDIMS::NPPVT::itemName = "NPPVT";
const int TABDIMS::NPPVT::defaultValue = 20;
const std::string TABDIMS::NTFIP::itemName = "NTFIP";
const int TABDIMS::NTFIP::defaultValue = 1;
const std::string TABDIMS::NRPVT::itemName = "NRPVT";
const int TABDIMS::NRPVT::defaultValue = 20;
const std::string TABDIMS::MAX_RV_NODES::itemName = "MAX_RV_NODES";
const std::string TABDIMS::NTENDP::itemName = "NTENDP";
const int TABDIMS::NTENDP::defaultValue = 1;
const std::string TABDIMS::NUM_STATE_EQ::itemName = "NUM_STATE_EQ";
const int TABDIMS::NUM_STATE_EQ::defaultValue = 1;
const std::string TABDIMS::NUM_EOS_RES::itemName = "NUM_EOS_RES";
const int TABDIMS::NUM_EOS_RES::defaultValue = 1;
const std::string TABDIMS::NUM_EOS_SURFACE::itemName = "NUM_EOS_SURFACE";
const std::string TABDIMS::MAX_FLUX_REGIONS::itemName = "MAX_FLUX_REGIONS";
const int TABDIMS::MAX_FLUX_REGIONS::defaultValue = 10;
const std::string TABDIMS::MAX_THERMAL_REGIONS::itemName = "MAX_THERMAL_REGIONS";
const int TABDIMS::MAX_THERMAL_REGIONS::defaultValue = 1;
const std::string TABDIMS::NTROCC::itemName = "NTROCC";
const std::string TABDIMS::MAX_PRESSURE_MAINTAINANCE_REGIONS::itemName = "MAX_PRESSURE_MAINTAINANCE_REGIONS";
const int TABDIMS::MAX_PRESSURE_MAINTAINANCE_REGIONS::defaultValue = 0;
const std::string TABDIMS::MAX_KVALUE_TABLES::itemName = "MAX_KVALUE_TABLES";
const int TABDIMS::MAX_KVALUE_TABLES::defaultValue = 0;
const std::string TABDIMS::NTALPHA::itemName = "NTALPHA";
const std::string TABDIMS::ASPHALTENE_ASPKDAM_MAX_ROWS::itemName = "ASPHALTENE_ASPKDAM_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPKDAM_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ASPHALTENE_ASPREWG_MAX_ROWS::itemName = "ASPHALTENE_ASPREWG_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPREWG_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ASPHALTENE_ASPVISO_MAX_ROWS::itemName = "ASPHALTENE_ASPVISO_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPVISO_MAX_ROWS::defaultValue = 10;
const std::string TABDIMS::ITEM20_NOT_USED::itemName = "ITEM20_NOT_USED";
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_COLUMNS::itemName = "ASPHALTENE_ASPPW2D_MAX_COLUMNS";
const int TABDIMS::ASPHALTENE_ASPPW2D_MAX_COLUMNS::defaultValue = 5;
const std::string TABDIMS::ASPHALTENE_ASPPW2D_MAX_ROWS::itemName = "ASPHALTENE_ASPPW2D_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPPW2D_MAX_ROWS::defaultValue = 5;
const std::string TABDIMS::ASPHALTENE_ASPWETF_MAX_ROWS::itemName = "ASPHALTENE_ASPWETF_MAX_ROWS";
const int TABDIMS::ASPHALTENE_ASPWETF_MAX_ROWS::defaultValue = 5;
const std::string TABDIMS::NUM_KVALUE_TABLES::itemName = "NUM_KVALUE_TABLES";
const int TABDIMS::NUM_KVALUE_TABLES::defaultValue = 0;
const std::string TABDIMS::RESERVED::itemName = "RESERVED";


TEMP::TEMP( ) : ParserKeyword("TEMP") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TEMP");
}
const std::string TEMP::keywordName = "TEMP";


TEMPI::TEMPI( ) : ParserKeyword("TEMPI") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("TEMPI");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Temperature");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TEMPI::keywordName = "TEMPI";
const std::string TEMPI::data::itemName = "data";


TEMPVD::TEMPVD( ) : ParserKeyword("TEMPVD") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTEQUL");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("TEMPVD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("Temperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TEMPVD::keywordName = "TEMPVD";
const std::string TEMPVD::DATA::itemName = "DATA";


THCONR::THCONR( ) : ParserKeyword("THCONR") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("THCONR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string THCONR::keywordName = "THCONR";
const std::string THCONR::data::itemName = "data";


THERMAL::THERMAL( ) : ParserKeyword("THERMAL") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("THERMAL");
}
const std::string THERMAL::keywordName = "THERMAL";


THERMEX1::THERMEX1( ) : ParserKeyword("THERMEX1") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("THERMEX1");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("EXPANSION_COEFF",Opm::ALL,0));
        item->setDescription("");
        item->push_backDimension("1/AbsoluteTemperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string THERMEX1::keywordName = "THERMEX1";
const std::string THERMEX1::EXPANSION_COEFF::itemName = "EXPANSION_COEFF";
const double THERMEX1::EXPANSION_COEFF::defaultValue = 0;


THPRES::THPRES( ) : ParserKeyword("THPRES") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("THPRES");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("REGION1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("REGION2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VALUE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string THPRES::keywordName = "THPRES";
const std::string THPRES::REGION1::itemName = "REGION1";
const std::string THPRES::REGION2::itemName = "REGION2";
const std::string THPRES::VALUE::itemName = "VALUE";


TITLE::TITLE( ) : ParserKeyword("TITLE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TITLE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("TitleText",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TITLE::keywordName = "TITLE";
const std::string TITLE::TitleText::itemName = "TitleText";


TLMIXPAR::TLMIXPAR( ) : ParserKeyword("TLMIXPAR") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TLMIXPAR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TL_VISCOSITY_PARAMETER",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TL_DENSITY_PARAMETER",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TLMIXPAR::keywordName = "TLMIXPAR";
const std::string TLMIXPAR::TL_VISCOSITY_PARAMETER::itemName = "TL_VISCOSITY_PARAMETER";
const std::string TLMIXPAR::TL_DENSITY_PARAMETER::itemName = "TL_DENSITY_PARAMETER";


TLPMIXPA::TLPMIXPA( ) : ParserKeyword("TLPMIXPA") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("MISCIBLE","NTMISC");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TLPMIXPA");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Pressure");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TLPMIXPA::keywordName = "TLPMIXPA";
const std::string TLPMIXPA::DATA::itemName = "DATA";


TOPS::TOPS( ) : ParserKeyword("TOPS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("TOPS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TOPS::keywordName = "TOPS";
const std::string TOPS::data::itemName = "data";


TRACER::TRACER( ) : ParserKeyword("TRACER") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TRACER");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("NAME",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FLUID",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("UNIT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("SOLUTION_PHASE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NUM_PART_TABLE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ADSORB_PHASE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACER::keywordName = "TRACER";
const std::string TRACER::NAME::itemName = "NAME";
const std::string TRACER::FLUID::itemName = "FLUID";
const std::string TRACER::UNIT::itemName = "UNIT";
const std::string TRACER::SOLUTION_PHASE::itemName = "SOLUTION_PHASE";
const std::string TRACER::NUM_PART_TABLE::itemName = "NUM_PART_TABLE";
const std::string TRACER::ADSORB_PHASE::itemName = "ADSORB_PHASE";


TRACERS::TRACERS( ) : ParserKeyword("TRACERS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("TRACERS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAX_OIL_TRACERS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_WATER_TRACERS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_GAS_TRACERS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ENV_TRACERS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("NUMERIC_DIFF",Opm::SINGLE,"NODIFF"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ITER",Opm::SINGLE,12));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MIN_ITER",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TRACERS::keywordName = "TRACERS";
const std::string TRACERS::MAX_OIL_TRACERS::itemName = "MAX_OIL_TRACERS";
const int TRACERS::MAX_OIL_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_WATER_TRACERS::itemName = "MAX_WATER_TRACERS";
const int TRACERS::MAX_WATER_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_GAS_TRACERS::itemName = "MAX_GAS_TRACERS";
const int TRACERS::MAX_GAS_TRACERS::defaultValue = 0;
const std::string TRACERS::MAX_ENV_TRACERS::itemName = "MAX_ENV_TRACERS";
const int TRACERS::MAX_ENV_TRACERS::defaultValue = 0;
const std::string TRACERS::NUMERIC_DIFF::itemName = "NUMERIC_DIFF";
const std::string TRACERS::NUMERIC_DIFF::defaultValue = "NODIFF";
const std::string TRACERS::MAX_ITER::itemName = "MAX_ITER";
const int TRACERS::MAX_ITER::defaultValue = 12;
const std::string TRACERS::MIN_ITER::itemName = "MIN_ITER";
const int TRACERS::MIN_ITER::defaultValue = 1;


TRANX::TRANX( ) : ParserKeyword("TRANX") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANX");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Transmissibility");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANX::keywordName = "TRANX";
const std::string TRANX::data::itemName = "data";


TRANY::TRANY( ) : ParserKeyword("TRANY") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANY");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Transmissibility");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANY::keywordName = "TRANY";
const std::string TRANY::data::itemName = "data";


TRANZ::TRANZ( ) : ParserKeyword("TRANZ") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("TRANZ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Transmissibility");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string TRANZ::keywordName = "TRANZ";
const std::string TRANZ::data::itemName = "data";


TREF::TREF( ) : ParserKeyword("TREF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TREF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TEMPERATURE",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("AbsoluteTemperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TREF::keywordName = "TREF";
const std::string TREF::TEMPERATURE::itemName = "TEMPERATURE";


TREFS::TREFS( ) : ParserKeyword("TREFS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("TREFS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TEMPERATURE",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("AbsoluteTemperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TREFS::keywordName = "TREFS";
const std::string TREFS::TEMPERATURE::itemName = "TEMPERATURE";


TSTEP::TSTEP( ) : ParserKeyword("TSTEP") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TSTEP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("step_list",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Timestep");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TSTEP::keywordName = "TSTEP";
const std::string TSTEP::step_list::itemName = "step_list";


TUNING::TUNING( ) : ParserKeyword("TUNING") {
  setFixedSize( (size_t) 3);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("TUNING");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TSINIT",Opm::SINGLE,1));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSMAXZ",Opm::SINGLE,365));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSMINZ",Opm::SINGLE,0.10000000000000001));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSMCHP",Opm::SINGLE,0.14999999999999999));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSFMAX",Opm::SINGLE,3));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSFMIN",Opm::SINGLE,0.30000000000000004));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TSFCNV",Opm::SINGLE,0.10000000000000001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TFDIFF",Opm::SINGLE,1.25));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THRUPT",Opm::SINGLE,1e+20));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TMAXWC",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Time");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("TRGTTE",Opm::SINGLE,0.10000000000000001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGCNV",Opm::SINGLE,0.001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGMBE",Opm::SINGLE,9.9999999999999995e-08));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGLCV",Opm::SINGLE,0.0001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXTTE",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXCNV",Opm::SINGLE,0.01));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXMBE",Opm::SINGLE,9.9999999999999995e-07));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXLCV",Opm::SINGLE,0.001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXWFL",Opm::SINGLE,0.001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGFIP",Opm::SINGLE,0.025000000000000001));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGSFT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THIONX",Opm::SINGLE,0.01));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("TRWGHT",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NEWTMX",Opm::SINGLE,12));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NEWTMN",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("LITMAX",Opm::SINGLE,25));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("LITMIN",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MXWSIT",Opm::SINGLE,8));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MXWPIT",Opm::SINGLE,8));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DDPLIM",Opm::SINGLE,1000000));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DDSLIM",Opm::SINGLE,1000000));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TRGDPR",Opm::SINGLE,1000000));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("XXXDPR",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TUNING::keywordName = "TUNING";
const std::string TUNING::TSINIT::itemName = "TSINIT";
const double TUNING::TSINIT::defaultValue = 1;
const std::string TUNING::TSMAXZ::itemName = "TSMAXZ";
const double TUNING::TSMAXZ::defaultValue = 365;
const std::string TUNING::TSMINZ::itemName = "TSMINZ";
const double TUNING::TSMINZ::defaultValue = 0.1;
const std::string TUNING::TSMCHP::itemName = "TSMCHP";
const double TUNING::TSMCHP::defaultValue = 0.15;
const std::string TUNING::TSFMAX::itemName = "TSFMAX";
const double TUNING::TSFMAX::defaultValue = 3;
const std::string TUNING::TSFMIN::itemName = "TSFMIN";
const double TUNING::TSFMIN::defaultValue = 0.3;
const std::string TUNING::TSFCNV::itemName = "TSFCNV";
const double TUNING::TSFCNV::defaultValue = 0.1;
const std::string TUNING::TFDIFF::itemName = "TFDIFF";
const double TUNING::TFDIFF::defaultValue = 1.25;
const std::string TUNING::THRUPT::itemName = "THRUPT";
const double TUNING::THRUPT::defaultValue = 1e+20;
const std::string TUNING::TMAXWC::itemName = "TMAXWC";
const std::string TUNING::TRGTTE::itemName = "TRGTTE";
const double TUNING::TRGTTE::defaultValue = 0.1;
const std::string TUNING::TRGCNV::itemName = "TRGCNV";
const double TUNING::TRGCNV::defaultValue = 0.001;
const std::string TUNING::TRGMBE::itemName = "TRGMBE";
const double TUNING::TRGMBE::defaultValue = 1e-07;
const std::string TUNING::TRGLCV::itemName = "TRGLCV";
const double TUNING::TRGLCV::defaultValue = 0.0001;
const std::string TUNING::XXXTTE::itemName = "XXXTTE";
const double TUNING::XXXTTE::defaultValue = 10;
const std::string TUNING::XXXCNV::itemName = "XXXCNV";
const double TUNING::XXXCNV::defaultValue = 0.01;
const std::string TUNING::XXXMBE::itemName = "XXXMBE";
const double TUNING::XXXMBE::defaultValue = 1e-06;
const std::string TUNING::XXXLCV::itemName = "XXXLCV";
const double TUNING::XXXLCV::defaultValue = 0.001;
const std::string TUNING::XXXWFL::itemName = "XXXWFL";
const double TUNING::XXXWFL::defaultValue = 0.001;
const std::string TUNING::TRGFIP::itemName = "TRGFIP";
const double TUNING::TRGFIP::defaultValue = 0.025;
const std::string TUNING::TRGSFT::itemName = "TRGSFT";
const std::string TUNING::THIONX::itemName = "THIONX";
const double TUNING::THIONX::defaultValue = 0.01;
const std::string TUNING::TRWGHT::itemName = "TRWGHT";
const int TUNING::TRWGHT::defaultValue = 1;
const std::string TUNING::NEWTMX::itemName = "NEWTMX";
const int TUNING::NEWTMX::defaultValue = 12;
const std::string TUNING::NEWTMN::itemName = "NEWTMN";
const int TUNING::NEWTMN::defaultValue = 1;
const std::string TUNING::LITMAX::itemName = "LITMAX";
const int TUNING::LITMAX::defaultValue = 25;
const std::string TUNING::LITMIN::itemName = "LITMIN";
const int TUNING::LITMIN::defaultValue = 1;
const std::string TUNING::MXWSIT::itemName = "MXWSIT";
const int TUNING::MXWSIT::defaultValue = 8;
const std::string TUNING::MXWPIT::itemName = "MXWPIT";
const int TUNING::MXWPIT::defaultValue = 8;
const std::string TUNING::DDPLIM::itemName = "DDPLIM";
const double TUNING::DDPLIM::defaultValue = 1e+06;
const std::string TUNING::DDSLIM::itemName = "DDSLIM";
const double TUNING::DDSLIM::defaultValue = 1e+06;
const std::string TUNING::TRGDPR::itemName = "TRGDPR";
const double TUNING::TRGDPR::defaultValue = 1e+06;
const std::string TUNING::XXXDPR::itemName = "XXXDPR";


TVDP::TVDP( ) : ParserKeyword("TVDP") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("EQLDIMS","NTTRVD");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SOLUTION");
  clearDeckNames();
  setMatchRegex("TVDP.+");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("table",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string TVDP::keywordName = "TVDP";
const std::string TVDP::table::itemName = "table";


UDADIMS::UDADIMS( ) : ParserKeyword("UDADIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDADIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NUM_UDQ_REPLACE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("IGNORED",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("TOTAL_UDQ_UNIQUE",Opm::SINGLE,100));
        item->setDescription("");
        record->addItem(item);
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


UDQDIMS::UDQDIMS( ) : ParserKeyword("UDQDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UDQDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAX_FUNCTIONS",Opm::SINGLE,16));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ITEMS",Opm::SINGLE,16));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_CONNECTIONS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_FIELDS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_GROUP",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_REGION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_SEGMENT",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_WELL",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_AQUIFER",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_BLOCK",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("RESTART_NEW_SEED",Opm::SINGLE,"N"));
        item->setDescription("");
        record->addItem(item);
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


UNIFIN::UNIFIN( ) : ParserKeyword("UNIFIN") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFIN");
}
const std::string UNIFIN::keywordName = "UNIFIN";


UNIFOUT::UNIFOUT( ) : ParserKeyword("UNIFOUT") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("UNIFOUT");
}
const std::string UNIFOUT::keywordName = "UNIFOUT";


VAPOIL::VAPOIL( ) : ParserKeyword("VAPOIL") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VAPOIL");
}
const std::string VAPOIL::keywordName = "VAPOIL";


VAPPARS::VAPPARS( ) : ParserKeyword("VAPPARS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("VAPPARS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_VAP_PROPENSITY",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_DENSITY_PROPENSITY",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VAPPARS::keywordName = "VAPPARS";
const std::string VAPPARS::OIL_VAP_PROPENSITY::itemName = "OIL_VAP_PROPENSITY";
const std::string VAPPARS::OIL_DENSITY_PROPENSITY::itemName = "OIL_DENSITY_PROPENSITY";


VFPIDIMS::VFPIDIMS( ) : ParserKeyword("VFPIDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VFPIDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAX_FLOW_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_THP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_INJ_VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPIDIMS::keywordName = "VFPIDIMS";
const std::string VFPIDIMS::MAX_FLOW_TABLE::itemName = "MAX_FLOW_TABLE";
const int VFPIDIMS::MAX_FLOW_TABLE::defaultValue = 0;
const std::string VFPIDIMS::MAX_THP_TABLE::itemName = "MAX_THP_TABLE";
const int VFPIDIMS::MAX_THP_TABLE::defaultValue = 0;
const std::string VFPIDIMS::MAX_INJ_VFP_TABLE::itemName = "MAX_INJ_VFP_TABLE";
const int VFPIDIMS::MAX_INJ_VFP_TABLE::defaultValue = 0;


VFPINJ::VFPINJ( ) : ParserKeyword("VFPINJ") {
  setSizeType(UNKNOWN);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPINJ");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("TABLE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATUM_DEPTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("RATE_TYPE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PRESSURE_DEF",Opm::SINGLE,"THP"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("UNITS",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("BODY_DEF",Opm::SINGLE,"BHP"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("FLOW_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("THP_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("THP_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPINJ::keywordName = "VFPINJ";
const std::string VFPINJ::TABLE::itemName = "TABLE";
const std::string VFPINJ::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string VFPINJ::RATE_TYPE::itemName = "RATE_TYPE";
const std::string VFPINJ::PRESSURE_DEF::itemName = "PRESSURE_DEF";
const std::string VFPINJ::PRESSURE_DEF::defaultValue = "THP";
const std::string VFPINJ::UNITS::itemName = "UNITS";
const std::string VFPINJ::BODY_DEF::itemName = "BODY_DEF";
const std::string VFPINJ::BODY_DEF::defaultValue = "BHP";
const std::string VFPINJ::FLOW_VALUES::itemName = "FLOW_VALUES";
const std::string VFPINJ::THP_VALUES::itemName = "THP_VALUES";
const std::string VFPINJ::THP_INDEX::itemName = "THP_INDEX";
const std::string VFPINJ::VALUES::itemName = "VALUES";


VFPPDIMS::VFPPDIMS( ) : ParserKeyword("VFPPDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("VFPPDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAX_FLOW_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_THP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_WCT_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_GCT_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_ALQ_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_PROD_VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPPDIMS::keywordName = "VFPPDIMS";
const std::string VFPPDIMS::MAX_FLOW_TABLE::itemName = "MAX_FLOW_TABLE";
const int VFPPDIMS::MAX_FLOW_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_THP_TABLE::itemName = "MAX_THP_TABLE";
const int VFPPDIMS::MAX_THP_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_WCT_TABLE::itemName = "MAX_WCT_TABLE";
const int VFPPDIMS::MAX_WCT_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_GCT_TABLE::itemName = "MAX_GCT_TABLE";
const int VFPPDIMS::MAX_GCT_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_ALQ_TABLE::itemName = "MAX_ALQ_TABLE";
const int VFPPDIMS::MAX_ALQ_TABLE::defaultValue = 0;
const std::string VFPPDIMS::MAX_PROD_VFP_TABLE::itemName = "MAX_PROD_VFP_TABLE";
const int VFPPDIMS::MAX_PROD_VFP_TABLE::defaultValue = 0;


VFPPROD::VFPPROD( ) : ParserKeyword("VFPPROD") {
  setSizeType(UNKNOWN);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("VFPPROD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("TABLE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DATUM_DEPTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("RATE_TYPE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("WFR",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GFR",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PRESSURE_DEF",Opm::SINGLE,"THP"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("ALQ_DEF",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("UNITS",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("BODY_DEF",Opm::SINGLE,"BHP"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("FLOW_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("THP_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("WFR_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("GFR_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("ALQ_VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("THP_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("WFR_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("GFR_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("ALQ_INDEX",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VALUES",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VFPPROD::keywordName = "VFPPROD";
const std::string VFPPROD::TABLE::itemName = "TABLE";
const std::string VFPPROD::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string VFPPROD::RATE_TYPE::itemName = "RATE_TYPE";
const std::string VFPPROD::WFR::itemName = "WFR";
const std::string VFPPROD::GFR::itemName = "GFR";
const std::string VFPPROD::PRESSURE_DEF::itemName = "PRESSURE_DEF";
const std::string VFPPROD::PRESSURE_DEF::defaultValue = "THP";
const std::string VFPPROD::ALQ_DEF::itemName = "ALQ_DEF";
const std::string VFPPROD::UNITS::itemName = "UNITS";
const std::string VFPPROD::BODY_DEF::itemName = "BODY_DEF";
const std::string VFPPROD::BODY_DEF::defaultValue = "BHP";
const std::string VFPPROD::FLOW_VALUES::itemName = "FLOW_VALUES";
const std::string VFPPROD::THP_VALUES::itemName = "THP_VALUES";
const std::string VFPPROD::WFR_VALUES::itemName = "WFR_VALUES";
const std::string VFPPROD::GFR_VALUES::itemName = "GFR_VALUES";
const std::string VFPPROD::ALQ_VALUES::itemName = "ALQ_VALUES";
const std::string VFPPROD::THP_INDEX::itemName = "THP_INDEX";
const std::string VFPPROD::WFR_INDEX::itemName = "WFR_INDEX";
const std::string VFPPROD::GFR_INDEX::itemName = "GFR_INDEX";
const std::string VFPPROD::ALQ_INDEX::itemName = "ALQ_INDEX";
const std::string VFPPROD::VALUES::itemName = "VALUES";


VISCREF::VISCREF( ) : ParserKeyword("VISCREF") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("VISCREF");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("REFERENCE_PRESSURE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("REFERENCE_RS",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("GasDissolutionFactor");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string VISCREF::keywordName = "VISCREF";
const std::string VISCREF::REFERENCE_PRESSURE::itemName = "REFERENCE_PRESSURE";
const std::string VISCREF::REFERENCE_RS::itemName = "REFERENCE_RS";


WATDENT::WATDENT( ) : ParserKeyword("WATDENT") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("WATDENT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("REFERENCE_TEMPERATURE",Opm::SINGLE,527.66999999999996));
        item->setDescription("");
        item->push_backDimension("AbsoluteTemperature");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("EXPANSION_COEFF_LINEAR",Opm::SINGLE,0.00016699999999999999));
        item->setDescription("");
        item->push_backDimension("1/AbsoluteTemperature");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("EXPANSION_COEFF_QUADRATIC",Opm::SINGLE,9.2600000000000001e-07));
        item->setDescription("");
        item->push_backDimension("1/AbsoluteTemperature*AbsoluteTemperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WATDENT::keywordName = "WATDENT";
const std::string WATDENT::REFERENCE_TEMPERATURE::itemName = "REFERENCE_TEMPERATURE";
const double WATDENT::REFERENCE_TEMPERATURE::defaultValue = 527.67;
const std::string WATDENT::EXPANSION_COEFF_LINEAR::itemName = "EXPANSION_COEFF_LINEAR";
const double WATDENT::EXPANSION_COEFF_LINEAR::defaultValue = 0.000167;
const std::string WATDENT::EXPANSION_COEFF_QUADRATIC::itemName = "EXPANSION_COEFF_QUADRATIC";
const double WATDENT::EXPANSION_COEFF_QUADRATIC::defaultValue = 9.26e-07;


WATER::WATER( ) : ParserKeyword("WATER") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WATER");
}
const std::string WATER::keywordName = "WATER";


WATVISCT::WATVISCT( ) : ParserKeyword("WATVISCT") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("WATVISCT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("DATA",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Temperature");
        item->push_backDimension("Viscosity");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WATVISCT::keywordName = "WATVISCT";
const std::string WATVISCT::DATA::itemName = "DATA";


WCONHIST::WCONHIST( ) : ParserKeyword("WCONHIST") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONHIST");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATUS",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ORAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("GasSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("VFPTable",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("Lift",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THP",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("BHP",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("NGLRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONHIST::keywordName = "WCONHIST";
const std::string WCONHIST::WELL::itemName = "WELL";
const std::string WCONHIST::STATUS::itemName = "STATUS";
const std::string WCONHIST::STATUS::defaultValue = "OPEN";
const std::string WCONHIST::CMODE::itemName = "CMODE";
const std::string WCONHIST::ORAT::itemName = "ORAT";
const double WCONHIST::ORAT::defaultValue = 0;
const std::string WCONHIST::WRAT::itemName = "WRAT";
const double WCONHIST::WRAT::defaultValue = 0;
const std::string WCONHIST::GRAT::itemName = "GRAT";
const double WCONHIST::GRAT::defaultValue = 0;
const std::string WCONHIST::VFPTable::itemName = "VFPTable";
const int WCONHIST::VFPTable::defaultValue = 0;
const std::string WCONHIST::Lift::itemName = "Lift";
const double WCONHIST::Lift::defaultValue = 0;
const std::string WCONHIST::THP::itemName = "THP";
const double WCONHIST::THP::defaultValue = 0;
const std::string WCONHIST::BHP::itemName = "BHP";
const double WCONHIST::BHP::defaultValue = 0;
const std::string WCONHIST::NGLRAT::itemName = "NGLRAT";
const double WCONHIST::NGLRAT::defaultValue = 0;


WCONINJ::WCONINJ( ) : ParserKeyword("WCONINJ") {
  setFixedSize( (size_t) 0);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJ");
}
const std::string WCONINJ::keywordName = "WCONINJ";


WCONINJE::WCONINJE( ) : ParserKeyword("WCONINJE") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TYPE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATUS",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RATE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RESV",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("ReservoirVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("BHP",Opm::SINGLE,6891));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THP",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VAPOIL_C",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GAS_STEAM_RATIO",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_OIL_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_GAS_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("OIL_STEAM_RATIO",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONINJE::keywordName = "WCONINJE";
const std::string WCONINJE::WELL::itemName = "WELL";
const std::string WCONINJE::TYPE::itemName = "TYPE";
const std::string WCONINJE::STATUS::itemName = "STATUS";
const std::string WCONINJE::STATUS::defaultValue = "OPEN";
const std::string WCONINJE::CMODE::itemName = "CMODE";
const std::string WCONINJE::RATE::itemName = "RATE";
const std::string WCONINJE::RESV::itemName = "RESV";
const std::string WCONINJE::BHP::itemName = "BHP";
const double WCONINJE::BHP::defaultValue = 6891;
const std::string WCONINJE::THP::itemName = "THP";
const std::string WCONINJE::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONINJE::VFP_TABLE::defaultValue = 0;
const std::string WCONINJE::VAPOIL_C::itemName = "VAPOIL_C";
const double WCONINJE::VAPOIL_C::defaultValue = 0;
const std::string WCONINJE::GAS_STEAM_RATIO::itemName = "GAS_STEAM_RATIO";
const double WCONINJE::GAS_STEAM_RATIO::defaultValue = 0;
const std::string WCONINJE::SURFACE_OIL_FRACTION::itemName = "SURFACE_OIL_FRACTION";
const double WCONINJE::SURFACE_OIL_FRACTION::defaultValue = 0;
const std::string WCONINJE::SURFACE_GAS_FRACTION::itemName = "SURFACE_GAS_FRACTION";
const double WCONINJE::SURFACE_GAS_FRACTION::defaultValue = 0;
const std::string WCONINJE::OIL_STEAM_RATIO::itemName = "OIL_STEAM_RATIO";
const double WCONINJE::OIL_STEAM_RATIO::defaultValue = 0;


WCONINJH::WCONINJH( ) : ParserKeyword("WCONINJH") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONINJH");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TYPE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATUS",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RATE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("ContextDependent");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("BHP",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THP",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VAPOIL_C",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_OIL_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_WATER_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SURFACE_GAS_FRACTION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE,"RATE"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONINJH::keywordName = "WCONINJH";
const std::string WCONINJH::WELL::itemName = "WELL";
const std::string WCONINJH::TYPE::itemName = "TYPE";
const std::string WCONINJH::STATUS::itemName = "STATUS";
const std::string WCONINJH::STATUS::defaultValue = "OPEN";
const std::string WCONINJH::RATE::itemName = "RATE";
const std::string WCONINJH::BHP::itemName = "BHP";
const std::string WCONINJH::THP::itemName = "THP";
const std::string WCONINJH::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONINJH::VFP_TABLE::defaultValue = 0;
const std::string WCONINJH::VAPOIL_C::itemName = "VAPOIL_C";
const double WCONINJH::VAPOIL_C::defaultValue = 0;
const std::string WCONINJH::SURFACE_OIL_FRACTION::itemName = "SURFACE_OIL_FRACTION";
const double WCONINJH::SURFACE_OIL_FRACTION::defaultValue = 0;
const std::string WCONINJH::SURFACE_WATER_FRACTION::itemName = "SURFACE_WATER_FRACTION";
const double WCONINJH::SURFACE_WATER_FRACTION::defaultValue = 0;
const std::string WCONINJH::SURFACE_GAS_FRACTION::itemName = "SURFACE_GAS_FRACTION";
const double WCONINJH::SURFACE_GAS_FRACTION::defaultValue = 0;
const std::string WCONINJH::CMODE::itemName = "CMODE";
const std::string WCONINJH::CMODE::defaultValue = "RATE";


WCONPROD::WCONPROD( ) : ParserKeyword("WCONPROD") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WCONPROD");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATUS",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ORAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("GasSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LRAT",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("RESV",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("LiquidSurfaceVolume/Time");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("BHP",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("THP",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Pressure");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("VFP_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ALQ",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM13",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM14",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM15",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM16",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM17",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM18",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM19",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("E300_ITEM20",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WCONPROD::keywordName = "WCONPROD";
const std::string WCONPROD::WELL::itemName = "WELL";
const std::string WCONPROD::STATUS::itemName = "STATUS";
const std::string WCONPROD::STATUS::defaultValue = "OPEN";
const std::string WCONPROD::CMODE::itemName = "CMODE";
const std::string WCONPROD::ORAT::itemName = "ORAT";
const double WCONPROD::ORAT::defaultValue = 0;
const std::string WCONPROD::WRAT::itemName = "WRAT";
const double WCONPROD::WRAT::defaultValue = 0;
const std::string WCONPROD::GRAT::itemName = "GRAT";
const double WCONPROD::GRAT::defaultValue = 0;
const std::string WCONPROD::LRAT::itemName = "LRAT";
const double WCONPROD::LRAT::defaultValue = 0;
const std::string WCONPROD::RESV::itemName = "RESV";
const double WCONPROD::RESV::defaultValue = 0;
const std::string WCONPROD::BHP::itemName = "BHP";
const double WCONPROD::BHP::defaultValue = 0;
const std::string WCONPROD::THP::itemName = "THP";
const double WCONPROD::THP::defaultValue = 0;
const std::string WCONPROD::VFP_TABLE::itemName = "VFP_TABLE";
const int WCONPROD::VFP_TABLE::defaultValue = 0;
const std::string WCONPROD::ALQ::itemName = "ALQ";
const double WCONPROD::ALQ::defaultValue = 0;
const std::string WCONPROD::E300_ITEM13::itemName = "E300_ITEM13";
const std::string WCONPROD::E300_ITEM14::itemName = "E300_ITEM14";
const std::string WCONPROD::E300_ITEM15::itemName = "E300_ITEM15";
const std::string WCONPROD::E300_ITEM16::itemName = "E300_ITEM16";
const std::string WCONPROD::E300_ITEM17::itemName = "E300_ITEM17";
const std::string WCONPROD::E300_ITEM18::itemName = "E300_ITEM18";
const std::string WCONPROD::E300_ITEM19::itemName = "E300_ITEM19";
const std::string WCONPROD::E300_ITEM20::itemName = "E300_ITEM20";


WELLDIMS::WELLDIMS( ) : ParserKeyword("WELLDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WELLDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("MAXWELLS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAXCONN",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAXGROUPS",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_GROUPSIZE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_STAGES",Opm::SINGLE,5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_STREAMS",Opm::SINGLE,10));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_MIXTURES",Opm::SINGLE,5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_SEPARATORS",Opm::SINGLE,4));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_MIXTURE_ITEMS",Opm::SINGLE,3));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_COMPLETION_X",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_WELLIST_PR_WELL",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_DYNAMIC_WELLIST",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("MAX_SECONDARY_WELLS",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELLDIMS::keywordName = "WELLDIMS";
const std::string WELLDIMS::MAXWELLS::itemName = "MAXWELLS";
const int WELLDIMS::MAXWELLS::defaultValue = 0;
const std::string WELLDIMS::MAXCONN::itemName = "MAXCONN";
const int WELLDIMS::MAXCONN::defaultValue = 0;
const std::string WELLDIMS::MAXGROUPS::itemName = "MAXGROUPS";
const int WELLDIMS::MAXGROUPS::defaultValue = 0;
const std::string WELLDIMS::MAX_GROUPSIZE::itemName = "MAX_GROUPSIZE";
const int WELLDIMS::MAX_GROUPSIZE::defaultValue = 0;
const std::string WELLDIMS::MAX_STAGES::itemName = "MAX_STAGES";
const int WELLDIMS::MAX_STAGES::defaultValue = 5;
const std::string WELLDIMS::MAX_STREAMS::itemName = "MAX_STREAMS";
const int WELLDIMS::MAX_STREAMS::defaultValue = 10;
const std::string WELLDIMS::MAX_MIXTURES::itemName = "MAX_MIXTURES";
const int WELLDIMS::MAX_MIXTURES::defaultValue = 5;
const std::string WELLDIMS::MAX_SEPARATORS::itemName = "MAX_SEPARATORS";
const int WELLDIMS::MAX_SEPARATORS::defaultValue = 4;
const std::string WELLDIMS::MAX_MIXTURE_ITEMS::itemName = "MAX_MIXTURE_ITEMS";
const int WELLDIMS::MAX_MIXTURE_ITEMS::defaultValue = 3;
const std::string WELLDIMS::MAX_COMPLETION_X::itemName = "MAX_COMPLETION_X";
const int WELLDIMS::MAX_COMPLETION_X::defaultValue = 0;
const std::string WELLDIMS::MAX_WELLIST_PR_WELL::itemName = "MAX_WELLIST_PR_WELL";
const int WELLDIMS::MAX_WELLIST_PR_WELL::defaultValue = 1;
const std::string WELLDIMS::MAX_DYNAMIC_WELLIST::itemName = "MAX_DYNAMIC_WELLIST";
const int WELLDIMS::MAX_DYNAMIC_WELLIST::defaultValue = 1;
const std::string WELLDIMS::MAX_SECONDARY_WELLS::itemName = "MAX_SECONDARY_WELLS";
const int WELLDIMS::MAX_SECONDARY_WELLS::defaultValue = 1;


WELL_PROBE::WELL_PROBE( ) : ParserKeyword("WELL_PROBE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("NGOPAS");
  addDeckName("WALQ");
  addDeckName("WAPI");
  addDeckName("WBGLR");
  addDeckName("WBHP");
  addDeckName("WBHPFP");
  addDeckName("WBHPH");
  addDeckName("WBP");
  addDeckName("WBP4");
  addDeckName("WBP5");
  addDeckName("WBP9");
  addDeckName("WCIC");
  addDeckName("WCIR");
  addDeckName("WCIT");
  addDeckName("WCPC");
  addDeckName("WCPR");
  addDeckName("WCPT");
  addDeckName("WDRPR");
  addDeckName("WEDC");
  addDeckName("WEFF");
  addDeckName("WEFFG");
  addDeckName("WEPR");
  addDeckName("WEPT");
  addDeckName("WGCV");
  addDeckName("WGDC");
  addDeckName("WGFRL");
  addDeckName("WGIGR");
  addDeckName("WGIP");
  addDeckName("WGIP2");
  addDeckName("WGIR");
  addDeckName("WGIRH");
  addDeckName("WGIRL");
  addDeckName("WGIRT");
  addDeckName("WGIT");
  addDeckName("WGITH");
  addDeckName("WGITL");
  addDeckName("WGLIR");
  addDeckName("WGLR");
  addDeckName("WGLRH");
  addDeckName("WGLRL");
  addDeckName("WGOR");
  addDeckName("WGORH");
  addDeckName("WGORL");
  addDeckName("WGPGR");
  addDeckName("WGPI");
  addDeckName("WGPI2");
  addDeckName("WGPP");
  addDeckName("WGPP2");
  addDeckName("WGPPF");
  addDeckName("WGPPF2");
  addDeckName("WGPPS");
  addDeckName("WGPPS2");
  addDeckName("WGPR");
  addDeckName("WGPRF");
  addDeckName("WGPRFP");
  addDeckName("WGPRH");
  addDeckName("WGPRL");
  addDeckName("WGPRS");
  addDeckName("WGPRT");
  addDeckName("WGPT");
  addDeckName("WGPTF");
  addDeckName("WGPTH");
  addDeckName("WGPTL");
  addDeckName("WGPTS");
  addDeckName("WGQ");
  addDeckName("WHD");
  addDeckName("WHDF");
  addDeckName("WJPR");
  addDeckName("WJPRH");
  addDeckName("WJPRT");
  addDeckName("WJPT");
  addDeckName("WJPTH");
  addDeckName("WLFRL");
  addDeckName("WLPR");
  addDeckName("WLPRH");
  addDeckName("WLPRT");
  addDeckName("WLPT");
  addDeckName("WLPTH");
  addDeckName("WLPTL");
  addDeckName("WMCON");
  addDeckName("WMCTL");
  addDeckName("WMIR");
  addDeckName("WMIT");
  addDeckName("WMMW");
  addDeckName("WMPR");
  addDeckName("WMPT");
  addDeckName("WMVFP");
  addDeckName("WNIR");
  addDeckName("WNIT");
  addDeckName("WNPR");
  addDeckName("WNPT");
  addDeckName("WOFRL");
  addDeckName("WOGLR");
  addDeckName("WOGR");
  addDeckName("WOGRH");
  addDeckName("WOGRL");
  addDeckName("WOIGR");
  addDeckName("WOIR");
  addDeckName("WOIRH");
  addDeckName("WOIRT");
  addDeckName("WOIT");
  addDeckName("WOITH");
  addDeckName("WOITL");
  addDeckName("WOPGR");
  addDeckName("WOPI");
  addDeckName("WOPI2");
  addDeckName("WOPP");
  addDeckName("WOPP2");
  addDeckName("WOPR");
  addDeckName("WOPRF");
  addDeckName("WOPRH");
  addDeckName("WOPRL");
  addDeckName("WOPRS");
  addDeckName("WOPRT");
  addDeckName("WOPT");
  addDeckName("WOPTF");
  addDeckName("WOPTH");
  addDeckName("WOPTL");
  addDeckName("WOPTS");
  addDeckName("WPI");
  addDeckName("WPI1");
  addDeckName("WPI4");
  addDeckName("WPI5");
  addDeckName("WPI9");
  addDeckName("WPWE0");
  addDeckName("WPWE1");
  addDeckName("WPWE2");
  addDeckName("WPWE3");
  addDeckName("WPWE4");
  addDeckName("WPWE5");
  addDeckName("WPWE6");
  addDeckName("WPWE7");
  addDeckName("WPWEM");
  addDeckName("WSIC");
  addDeckName("WSIR");
  addDeckName("WSIT");
  addDeckName("WSPC");
  addDeckName("WSPR");
  addDeckName("WSPT");
  addDeckName("WSTAT");
  addDeckName("WTHP");
  addDeckName("WTHPFP");
  addDeckName("WTHPH");
  addDeckName("WTHT");
  addDeckName("WTIC");
  addDeckName("WTICF");
  addDeckName("WTICHEA");
  addDeckName("WTICS");
  addDeckName("WTIR");
  addDeckName("WTIRALK");
  addDeckName("WTIRANI");
  addDeckName("WTIRCAT");
  addDeckName("WTIRF");
  addDeckName("WTIRFOA");
  addDeckName("WTIRHEA");
  addDeckName("WTIRS");
  addDeckName("WTIRSUR");
  addDeckName("WTIT");
  addDeckName("WTITALK");
  addDeckName("WTITANI");
  addDeckName("WTITCAT");
  addDeckName("WTITF");
  addDeckName("WTITFOA");
  addDeckName("WTITHEA");
  addDeckName("WTITS");
  addDeckName("WTITSUR");
  addDeckName("WTPC");
  addDeckName("WTPCF");
  addDeckName("WTPCHEA");
  addDeckName("WTPCS");
  addDeckName("WTPR");
  addDeckName("WTPRALK");
  addDeckName("WTPRANI");
  addDeckName("WTPRCAT");
  addDeckName("WTPRF");
  addDeckName("WTPRFOA");
  addDeckName("WTPRHEA");
  addDeckName("WTPRS");
  addDeckName("WTPRSUR");
  addDeckName("WTPT");
  addDeckName("WTPTALK");
  addDeckName("WTPTANI");
  addDeckName("WTPTCAT");
  addDeckName("WTPTF");
  addDeckName("WTPTFOA");
  addDeckName("WTPTHEA");
  addDeckName("WTPTS");
  addDeckName("WTPTSUR");
  addDeckName("WVFRL");
  addDeckName("WVIR");
  addDeckName("WVIRL");
  addDeckName("WVIRT");
  addDeckName("WVIT");
  addDeckName("WVITL");
  addDeckName("WVPGR");
  addDeckName("WVPR");
  addDeckName("WVPRT");
  addDeckName("WVPT");
  addDeckName("WVPTL");
  addDeckName("WWCT");
  addDeckName("WWCTH");
  addDeckName("WWCTL");
  addDeckName("WWFRL");
  addDeckName("WWGR");
  addDeckName("WWGRH");
  addDeckName("WWGRL");
  addDeckName("WWIGR");
  addDeckName("WWIP");
  addDeckName("WWIP2");
  addDeckName("WWIR");
  addDeckName("WWIRH");
  addDeckName("WWIRL");
  addDeckName("WWIRT");
  addDeckName("WWIT");
  addDeckName("WWITH");
  addDeckName("WWITL");
  addDeckName("WWPGR");
  addDeckName("WWPI");
  addDeckName("WWPI2");
  addDeckName("WWPIR");
  addDeckName("WWPP");
  addDeckName("WWPP2");
  addDeckName("WWPR");
  addDeckName("WWPRH");
  addDeckName("WWPRL");
  addDeckName("WWPRT");
  addDeckName("WWPT");
  addDeckName("WWPTH");
  addDeckName("WWPTL");
  setMatchRegex("WU.+|(WBHWC|WGFWC|WOFWC|WWFWC)[1-9][0-9]?|WTPR.+|WTPT.+|WTPC.+|WTIR.+|WTIT.+|WTIC.+");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELLS",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELL_PROBE::keywordName = "WELL_PROBE";
const std::string WELL_PROBE::WELLS::itemName = "WELLS";


WELOPEN::WELOPEN( ) : ParserKeyword("WELOPEN") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELOPEN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("STATUS",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("C1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("C2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELOPEN::keywordName = "WELOPEN";
const std::string WELOPEN::WELL::itemName = "WELL";
const std::string WELOPEN::STATUS::itemName = "STATUS";
const std::string WELOPEN::STATUS::defaultValue = "OPEN";
const std::string WELOPEN::I::itemName = "I";
const std::string WELOPEN::J::itemName = "J";
const std::string WELOPEN::K::itemName = "K";
const std::string WELOPEN::C1::itemName = "C1";
const std::string WELOPEN::C2::itemName = "C2";


WELSEGS::WELSEGS( ) : ParserKeyword("WELSEGS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSEGS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DEPTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LENGTH",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WELLBORE_VOLUME",Opm::SINGLE,1.0000000000000001e-05));
        item->setDescription("");
        item->push_backDimension("Length*Length*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("INFO_TYPE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PRESSURE_COMPONENTS",Opm::SINGLE,"HFA"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FLOW_MODEL",Opm::SINGLE,"HO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TOP_X",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TOP_Y",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     addRecord( record );
  }
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("SEGMENT1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("SEGMENT2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("BRANCH",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("JOIN_SEGMENT",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SEGMENT_LENGTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DEPTH_CHANGE",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("DIAMETER",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("ROUGHNESS",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("AREA",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("VOLUME",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length*Length*Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LENGTH_X",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("LENGTH_Y",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSEGS::keywordName = "WELSEGS";
const std::string WELSEGS::WELL::itemName = "WELL";
const std::string WELSEGS::DEPTH::itemName = "DEPTH";
const std::string WELSEGS::LENGTH::itemName = "LENGTH";
const double WELSEGS::LENGTH::defaultValue = 0;
const std::string WELSEGS::WELLBORE_VOLUME::itemName = "WELLBORE_VOLUME";
const double WELSEGS::WELLBORE_VOLUME::defaultValue = 1e-05;
const std::string WELSEGS::INFO_TYPE::itemName = "INFO_TYPE";
const std::string WELSEGS::PRESSURE_COMPONENTS::itemName = "PRESSURE_COMPONENTS";
const std::string WELSEGS::PRESSURE_COMPONENTS::defaultValue = "HFA";
const std::string WELSEGS::FLOW_MODEL::itemName = "FLOW_MODEL";
const std::string WELSEGS::FLOW_MODEL::defaultValue = "HO";
const std::string WELSEGS::TOP_X::itemName = "TOP_X";
const double WELSEGS::TOP_X::defaultValue = 0;
const std::string WELSEGS::TOP_Y::itemName = "TOP_Y";
const double WELSEGS::TOP_Y::defaultValue = 0;
const std::string WELSEGS::SEGMENT1::itemName = "SEGMENT1";
const std::string WELSEGS::SEGMENT2::itemName = "SEGMENT2";
const std::string WELSEGS::BRANCH::itemName = "BRANCH";
const std::string WELSEGS::JOIN_SEGMENT::itemName = "JOIN_SEGMENT";
const std::string WELSEGS::SEGMENT_LENGTH::itemName = "SEGMENT_LENGTH";
const std::string WELSEGS::DEPTH_CHANGE::itemName = "DEPTH_CHANGE";
const std::string WELSEGS::DIAMETER::itemName = "DIAMETER";
const std::string WELSEGS::ROUGHNESS::itemName = "ROUGHNESS";
const std::string WELSEGS::AREA::itemName = "AREA";
const std::string WELSEGS::VOLUME::itemName = "VOLUME";
const std::string WELSEGS::LENGTH_X::itemName = "LENGTH_X";
const double WELSEGS::LENGTH_X::defaultValue = 0;
const std::string WELSEGS::LENGTH_Y::itemName = "LENGTH_Y";
const double WELSEGS::LENGTH_Y::defaultValue = 0;


WELSPECS::WELSPECS( ) : ParserKeyword("WELSPECS") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELSPECS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("HEAD_I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("HEAD_J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("REF_DEPTH",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PHASE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("D_RADIUS",Opm::SINGLE,0));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("INFLOW_EQ",Opm::SINGLE,"STD"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("AUTO_SHUTIN",Opm::SINGLE,"SHUT"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CROSSFLOW",Opm::SINGLE,"YES"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("P_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("DENSITY_CALC",Opm::SINGLE,"SEG"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("FIP_REGION",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FRONTSIM1",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("FRONTSIM2",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("well_model",Opm::SINGLE,"STD"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("POLYMER_TABLE",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELSPECS::keywordName = "WELSPECS";
const std::string WELSPECS::WELL::itemName = "WELL";
const std::string WELSPECS::GROUP::itemName = "GROUP";
const std::string WELSPECS::HEAD_I::itemName = "HEAD_I";
const std::string WELSPECS::HEAD_J::itemName = "HEAD_J";
const std::string WELSPECS::REF_DEPTH::itemName = "REF_DEPTH";
const std::string WELSPECS::PHASE::itemName = "PHASE";
const std::string WELSPECS::D_RADIUS::itemName = "D_RADIUS";
const double WELSPECS::D_RADIUS::defaultValue = 0;
const std::string WELSPECS::INFLOW_EQ::itemName = "INFLOW_EQ";
const std::string WELSPECS::INFLOW_EQ::defaultValue = "STD";
const std::string WELSPECS::AUTO_SHUTIN::itemName = "AUTO_SHUTIN";
const std::string WELSPECS::AUTO_SHUTIN::defaultValue = "SHUT";
const std::string WELSPECS::CROSSFLOW::itemName = "CROSSFLOW";
const std::string WELSPECS::CROSSFLOW::defaultValue = "YES";
const std::string WELSPECS::P_TABLE::itemName = "P_TABLE";
const int WELSPECS::P_TABLE::defaultValue = 0;
const std::string WELSPECS::DENSITY_CALC::itemName = "DENSITY_CALC";
const std::string WELSPECS::DENSITY_CALC::defaultValue = "SEG";
const std::string WELSPECS::FIP_REGION::itemName = "FIP_REGION";
const int WELSPECS::FIP_REGION::defaultValue = 0;
const std::string WELSPECS::FRONTSIM1::itemName = "FRONTSIM1";
const std::string WELSPECS::FRONTSIM2::itemName = "FRONTSIM2";
const std::string WELSPECS::well_model::itemName = "well_model";
const std::string WELSPECS::well_model::defaultValue = "STD";
const std::string WELSPECS::POLYMER_TABLE::itemName = "POLYMER_TABLE";
const int WELSPECS::POLYMER_TABLE::defaultValue = 0;


WELTARG::WELTARG( ) : ParserKeyword("WELTARG") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WELTARG");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("NEW_VALUE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WELTARG::keywordName = "WELTARG";
const std::string WELTARG::WELL::itemName = "WELL";
const std::string WELTARG::CMODE::itemName = "CMODE";
const std::string WELTARG::NEW_VALUE::itemName = "NEW_VALUE";


WGRUPCON::WGRUPCON( ) : ParserKeyword("WGRUPCON") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WGRUPCON");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GROUP_CONTROLLED",Opm::SINGLE,"YES"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("GUIDE_RATE",Opm::SINGLE,-1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PHASE",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SCALING_FACTOR",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WGRUPCON::keywordName = "WGRUPCON";
const std::string WGRUPCON::WELL::itemName = "WELL";
const std::string WGRUPCON::GROUP_CONTROLLED::itemName = "GROUP_CONTROLLED";
const std::string WGRUPCON::GROUP_CONTROLLED::defaultValue = "YES";
const std::string WGRUPCON::GUIDE_RATE::itemName = "GUIDE_RATE";
const double WGRUPCON::GUIDE_RATE::defaultValue = -1;
const std::string WGRUPCON::PHASE::itemName = "PHASE";
const std::string WGRUPCON::SCALING_FACTOR::itemName = "SCALING_FACTOR";
const double WGRUPCON::SCALING_FACTOR::defaultValue = 1;


WHISTCTL::WHISTCTL( ) : ParserKeyword("WHISTCTL") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WHISTCTL");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("CMODE",Opm::SINGLE,"NONE"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("BPH_TERMINATE",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WHISTCTL::keywordName = "WHISTCTL";
const std::string WHISTCTL::CMODE::itemName = "CMODE";
const std::string WHISTCTL::CMODE::defaultValue = "NONE";
const std::string WHISTCTL::BPH_TERMINATE::itemName = "BPH_TERMINATE";
const std::string WHISTCTL::BPH_TERMINATE::defaultValue = "NO";


WPAVE::WPAVE( ) : ParserKeyword("WPAVE") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPAVE");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("WEIGTH_FACTOR1",Opm::SINGLE,0.5));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WEIGTH_FACTOR2",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("DEPTH_CORRECTION",Opm::SINGLE,"WELL"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("CONNECTION",Opm::SINGLE,"OPEN"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPAVE::keywordName = "WPAVE";
const std::string WPAVE::WEIGTH_FACTOR1::itemName = "WEIGTH_FACTOR1";
const double WPAVE::WEIGTH_FACTOR1::defaultValue = 0.5;
const std::string WPAVE::WEIGTH_FACTOR2::itemName = "WEIGTH_FACTOR2";
const double WPAVE::WEIGTH_FACTOR2::defaultValue = 1;
const std::string WPAVE::DEPTH_CORRECTION::itemName = "DEPTH_CORRECTION";
const std::string WPAVE::DEPTH_CORRECTION::defaultValue = "WELL";
const std::string WPAVE::CONNECTION::itemName = "CONNECTION";
const std::string WPAVE::CONNECTION::defaultValue = "OPEN";


WPIMULT::WPIMULT( ) : ParserKeyword("WPIMULT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPIMULT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("WELLPI",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("I",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("J",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("K",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("FIRST",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("LAST",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPIMULT::keywordName = "WPIMULT";
const std::string WPIMULT::WELL::itemName = "WELL";
const std::string WPIMULT::WELLPI::itemName = "WELLPI";
const double WPIMULT::WELLPI::defaultValue = 1;
const std::string WPIMULT::I::itemName = "I";
const std::string WPIMULT::J::itemName = "J";
const std::string WPIMULT::K::itemName = "K";
const std::string WPIMULT::FIRST::itemName = "FIRST";
const std::string WPIMULT::LAST::itemName = "LAST";


WPITAB::WPITAB( ) : ParserKeyword("WPITAB") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WPITAB");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("PI",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPITAB::keywordName = "WPITAB";
const std::string WPITAB::WELL::itemName = "WELL";
const std::string WPITAB::PI::itemName = "PI";
const double WPITAB::PI::defaultValue = 0;


WPOLYMER::WPOLYMER( ) : ParserKeyword("WPOLYMER") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("WPOLYMER");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("POLYMER_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SALT_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("PolymerDensity");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GROUP_POLYMER_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("GROUP_SALT_CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WPOLYMER::keywordName = "WPOLYMER";
const std::string WPOLYMER::WELL::itemName = "WELL";
const std::string WPOLYMER::POLYMER_CONCENTRATION::itemName = "POLYMER_CONCENTRATION";
const std::string WPOLYMER::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";
const std::string WPOLYMER::GROUP_POLYMER_CONCENTRATION::itemName = "GROUP_POLYMER_CONCENTRATION";
const std::string WPOLYMER::GROUP_SALT_CONCENTRATION::itemName = "GROUP_SALT_CONCENTRATION";


WRFT::WRFT( ) : ParserKeyword("WRFT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WRFT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WRFT::keywordName = "WRFT";
const std::string WRFT::WELL::itemName = "WELL";


WRFTPLT::WRFTPLT( ) : ParserKeyword("WRFTPLT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WRFTPLT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OUTPUT_RFT",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OUTPUT_PLT",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("OUTPUT_SEGMENT",Opm::SINGLE,"NO"));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WRFTPLT::keywordName = "WRFTPLT";
const std::string WRFTPLT::WELL::itemName = "WELL";
const std::string WRFTPLT::OUTPUT_RFT::itemName = "OUTPUT_RFT";
const std::string WRFTPLT::OUTPUT_RFT::defaultValue = "NO";
const std::string WRFTPLT::OUTPUT_PLT::itemName = "OUTPUT_PLT";
const std::string WRFTPLT::OUTPUT_PLT::defaultValue = "NO";
const std::string WRFTPLT::OUTPUT_SEGMENT::itemName = "OUTPUT_SEGMENT";
const std::string WRFTPLT::OUTPUT_SEGMENT::defaultValue = "NO";


WSEGDIMS::WSEGDIMS( ) : ParserKeyword("WSEGDIMS") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("WSEGDIMS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserIntItem("NSWLMX",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NSEGMX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NLBRMX",Opm::SINGLE,1));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("NCRDMX",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSEGDIMS::keywordName = "WSEGDIMS";
const std::string WSEGDIMS::NSWLMX::itemName = "NSWLMX";
const int WSEGDIMS::NSWLMX::defaultValue = 0;
const std::string WSEGDIMS::NSEGMX::itemName = "NSEGMX";
const int WSEGDIMS::NSEGMX::defaultValue = 1;
const std::string WSEGDIMS::NLBRMX::itemName = "NLBRMX";
const int WSEGDIMS::NLBRMX::defaultValue = 1;
const std::string WSEGDIMS::NCRDMX::itemName = "NCRDMX";
const int WSEGDIMS::NCRDMX::defaultValue = 0;


WSOLVENT::WSOLVENT( ) : ParserKeyword("WSOLVENT") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WSOLVENT");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("SOLVENT_FRACTION",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WSOLVENT::keywordName = "WSOLVENT";
const std::string WSOLVENT::WELL::itemName = "WELL";
const std::string WSOLVENT::SOLVENT_FRACTION::itemName = "SOLVENT_FRACTION";


WTEMP::WTEMP( ) : ParserKeyword("WTEMP") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTEMP");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("TEMP",Opm::SINGLE));
        item->setDescription("");
        item->push_backDimension("Temperature");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTEMP::keywordName = "WTEMP";
const std::string WTEMP::WELL::itemName = "WELL";
const std::string WTEMP::TEMP::itemName = "TEMP";


WTEST::WTEST( ) : ParserKeyword("WTEST") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTEST");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("well",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("interval",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("reason",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserIntItem("TEST_NUM",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("START_TIME",Opm::SINGLE,0));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTEST::keywordName = "WTEST";
const std::string WTEST::well::itemName = "well";
const std::string WTEST::interval::itemName = "interval";
const std::string WTEST::reason::itemName = "reason";
const std::string WTEST::TEST_NUM::itemName = "TEST_NUM";
const int WTEST::TEST_NUM::defaultValue = 0;
const std::string WTEST::START_TIME::itemName = "START_TIME";
const double WTEST::START_TIME::defaultValue = 0;


WTRACER::WTRACER( ) : ParserKeyword("WTRACER") {
  setSizeType(SLASH_TERMINATED);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("WTRACER");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("WELL",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("TRACER",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("CONCENTRATION",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserDoubleItem("CUM_TRACER_FACTOR",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     {
        ParserItemPtr item(new ParserStringItem("PRODUCTION_GROUP",Opm::SINGLE));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string WTRACER::keywordName = "WTRACER";
const std::string WTRACER::WELL::itemName = "WELL";
const std::string WTRACER::TRACER::itemName = "TRACER";
const std::string WTRACER::CONCENTRATION::itemName = "CONCENTRATION";
const std::string WTRACER::CUM_TRACER_FACTOR::itemName = "CUM_TRACER_FACTOR";
const std::string WTRACER::PRODUCTION_GROUP::itemName = "PRODUCTION_GROUP";


ZCORN::ZCORN( ) : ParserKeyword("ZCORN") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ZCORN");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("data",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("Length");
        record->addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string ZCORN::keywordName = "ZCORN";
const std::string ZCORN::data::itemName = "data";


ZFACT1::ZFACT1( ) : ParserKeyword("ZFACT1") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACT1");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("Z0",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACT1::keywordName = "ZFACT1";
const std::string ZFACT1::Z0::itemName = "Z0";


ZFACT1S::ZFACT1S( ) : ParserKeyword("ZFACT1S") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACT1S");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("Z0",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACT1S::keywordName = "ZFACT1S";
const std::string ZFACT1S::Z0::itemName = "Z0";


ZFACTOR::ZFACTOR( ) : ParserKeyword("ZFACTOR") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACTOR");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("Z0",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACTOR::keywordName = "ZFACTOR";
const std::string ZFACTOR::Z0::itemName = "Z0";


ZFACTORS::ZFACTORS( ) : ParserKeyword("ZFACTORS") {
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NUM_STATE_EQ");
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ZFACTORS");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserDoubleItem("Z0",Opm::ALL));
        item->setDescription("");
        item->push_backDimension("1");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZFACTORS::keywordName = "ZFACTORS";
const std::string ZFACTORS::Z0::itemName = "Z0";


ZIPPY2::ZIPPY2( ) : ParserKeyword("ZIPPY2") {
  setFixedSize( (size_t) 1);
  setDescription("");
  clearValidSectionNames();
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ZIPPY2");
  {
     std::shared_ptr<ParserRecord> record = std::make_shared<ParserRecord>();
     {
        ParserItemPtr item(new ParserStringItem("SETTINGS",Opm::ALL));
        item->setDescription("");
        record->addItem(item);
     }
     addRecord( record );
  }
}
const std::string ZIPPY2::keywordName = "ZIPPY2";
const std::string ZIPPY2::SETTINGS::itemName = "SETTINGS";


void addDefaultKeywords0(Parser& p);
void addDefaultKeywords1(Parser& p);
void addDefaultKeywords2(Parser& p);
void addDefaultKeywords3(Parser& p);
}
void Parser::addDefaultKeywords() {
Opm::ParserKeywords::addDefaultKeywords0(*this);
Opm::ParserKeywords::addDefaultKeywords1(*this);
Opm::ParserKeywords::addDefaultKeywords2(*this);
Opm::ParserKeywords::addDefaultKeywords3(*this);
}}
