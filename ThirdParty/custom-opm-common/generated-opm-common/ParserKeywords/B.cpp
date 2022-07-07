
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/B.hpp>
namespace Opm {
namespace ParserKeywords {
BC::BC() : ParserKeyword("BC", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("BC");
  {
     ParserRecord record;
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
        ParserItem item("TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("COMPONENT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Mass/Time*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BC::keywordName = "BC";
const std::string BC::I1::itemName = "I1";
const std::string BC::I2::itemName = "I2";
const std::string BC::J1::itemName = "J1";
const std::string BC::J2::itemName = "J2";
const std::string BC::K1::itemName = "K1";
const std::string BC::K2::itemName = "K2";
const std::string BC::TYPE::itemName = "TYPE";
const std::string BC::DIRECTION::itemName = "DIRECTION";
const std::string BC::COMPONENT::itemName = "COMPONENT";
const std::string BC::COMPONENT::defaultValue = "NONE";
const std::string BC::RATE::itemName = "RATE";
const double BC::RATE::defaultValue = 0;


BDENSITY::BDENSITY() : ParserKeyword("BDENSITY", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("BDENSITY");
  {
     ParserRecord record;
     {
        ParserItem item("BRINE_DENSITY", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BDENSITY::keywordName = "BDENSITY";
const std::string BDENSITY::BRINE_DENSITY::itemName = "BRINE_DENSITY";


BGGI::BGGI() : ParserKeyword("BGGI", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("BGGI");
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
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BGGI::keywordName = "BGGI";
const std::string BGGI::GAS_PRESSURE::itemName = "GAS_PRESSURE";
const std::string BGGI::DATA::itemName = "DATA";


BIGMODEL::BIGMODEL() : ParserKeyword("BIGMODEL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BIGMODEL");
}
const std::string BIGMODEL::keywordName = "BIGMODEL";


BLACKOIL::BLACKOIL() : ParserKeyword("BLACKOIL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BLACKOIL");
}
const std::string BLACKOIL::keywordName = "BLACKOIL";


BLOCK_PROBE::BLOCK_PROBE() : ParserKeyword("BLOCK_PROBE", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("BVELWK");
  addDeckName("BFLOOK");
  addDeckName("BSOIL");
  addDeckName("BOSAT");
  addDeckName("BGIP");
  addDeckName("BVOIL");
  addDeckName("BGKR");
  addDeckName("BOIP");
  addDeckName("BOIPL");
  addDeckName("BGPR");
  addDeckName("BSCN_X");
  addDeckName("BGIPG");
  addDeckName("BOVIS");
  addDeckName("BOIPG");
  addDeckName("BWVIS");
  addDeckName("BRSSAT");
  addDeckName("BPPO");
  addDeckName("BPDEW");
  addDeckName("BDENO");
  addDeckName("BVELOJ");
  addDeckName("BODEN");
  addDeckName("BVELWI");
  addDeckName("BFLOOI");
  addDeckName("BGDEN");
  addDeckName("BVELWJ");
  addDeckName("BFLOOJ");
  addDeckName("BFLOWI");
  addDeckName("LBHDF_X");
  addDeckName("BVELOI");
  addDeckName("BVELOK");
  addDeckName("BPPC");
  addDeckName("BTRADCAT");
  addDeckName("BFLOGK");
  addDeckName("BWSAT");
  addDeckName("BSWAT");
  addDeckName("BGSAT");
  addDeckName("BPR");
  addDeckName("BWIP");
  addDeckName("BEWV_SAL");
  addDeckName("BPPW");
  addDeckName("BWKR");
  addDeckName("BVWAT");
  addDeckName("BDENW");
  addDeckName("BKRO");
  addDeckName("BVELGJ");
  addDeckName("BWDEN");
  addDeckName("BSGAS");
  addDeckName("BGIPL");
  addDeckName("BPPG");
  addDeckName("BKROG");
  addDeckName("BVGAS");
  addDeckName("BGVIS");
  addDeckName("BDENG");
  addDeckName("BFLOGI");
  addDeckName("BKRG");
  addDeckName("BFLOGJ");
  addDeckName("BPBUB");
  addDeckName("BVELGI");
  addDeckName("BRS");
  addDeckName("BVELGK");
  addDeckName("BWPR");
  addDeckName("BRV");
  addDeckName("BRVSAT");
  addDeckName("BSTATE");
  addDeckName("BOKR");
  addDeckName("BKROW");
  addDeckName("BKRW");
  addDeckName("BWPC");
  addDeckName("BGPC");
  addDeckName("BGTRP");
  addDeckName("BGTPD");
  addDeckName("BOPV");
  addDeckName("BGSHY");
  addDeckName("BGSTRP");
  addDeckName("BWSHY");
  addDeckName("BWSMA");
  addDeckName("BHD");
  addDeckName("BHDF");
  addDeckName("BPR_X");
  addDeckName("BHD_X");
  addDeckName("BHDF_X");
  addDeckName("BCTRA_X");
  addDeckName("LBPR_X");
  addDeckName("LBHD_X");
  addDeckName("LBSCN_X");
  addDeckName("LBCTRA_X");
  addDeckName("BRPV");
  addDeckName("BWPV");
  addDeckName("BGPV");
  addDeckName("BHPV");
  addDeckName("BRTM");
  addDeckName("BPORVMOD");
  addDeckName("BPERMMOD");
  addDeckName("BSCN");
  addDeckName("BAPI");
  addDeckName("BSIP");
  addDeckName("BCAD");
  addDeckName("BTCNFANI");
  addDeckName("BTCNFCAT");
  addDeckName("BTCASUR");
  addDeckName("BTSADCAT");
  addDeckName("BESALSUR");
  addDeckName("BESALPLY");
  addDeckName("BTCNFHEA");
  addDeckName("BTIPTHEA");
  addDeckName("BCGC");
  addDeckName("BCSC");
  addDeckName("BTCNFFOA");
  addDeckName("BTCNMFOA");
  addDeckName("BTIPTFOA");
  addDeckName("BTADSFOA");
  addDeckName("BTDCYFOA");
  addDeckName("BTMOBFOA");
  addDeckName("BTHLFFOA");
  addDeckName("BGI");
  addDeckName("BNSAT");
  addDeckName("BNIP");
  addDeckName("BNKR");
  addDeckName("BTPADALK");
  addDeckName("BTCNFSUR");
  addDeckName("BTIPTSUR");
  addDeckName("BTADSUR");
  addDeckName("BTSTSUR");
  addDeckName("BEWV_SUR");
  addDeckName("BTCNFALK");
  addDeckName("BTADSALK");
  addDeckName("BTSTMALK");
  addDeckName("BTSADALK");
  setMatchRegex("BU.+|BTIPF.+|BTIPS.+|BTCNF.+|BTCNS.+|BTCN[1-9][0-9]*.+|BTIPT.+|BTIPF.+|BTIPS.+|BTIP[1-9][0-9]*.+|BTADS.+|BTDCY");
  {
     ParserRecord record;
     {
        ParserItem item("I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BLOCK_PROBE::keywordName = "BLOCK_PROBE";
const std::string BLOCK_PROBE::I::itemName = "I";
const std::string BLOCK_PROBE::J::itemName = "J";
const std::string BLOCK_PROBE::K::itemName = "K";


BLOCK_PROBE300::BLOCK_PROBE300() : ParserKeyword("BLOCK_PROBE300", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("BTEMP");
  {
     ParserRecord record;
     {
        ParserItem item("I", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BLOCK_PROBE300::keywordName = "BLOCK_PROBE300";
const std::string BLOCK_PROBE300::I::itemName = "I";
const std::string BLOCK_PROBE300::J::itemName = "J";
const std::string BLOCK_PROBE300::K::itemName = "K";


BOGI::BOGI() : ParserKeyword("BOGI", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("BOGI");
  {
     ParserRecord record;
     {
        ParserItem item("OIL_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BOGI::keywordName = "BOGI";
const std::string BOGI::OIL_PRESSURE::itemName = "OIL_PRESSURE";
const std::string BOGI::DATA::itemName = "DATA";


BOUNDARY::BOUNDARY() : ParserKeyword("BOUNDARY", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("BOUNDARY");
  {
     ParserRecord record;
     {
        ParserItem item("IX1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KZ1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KZ2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ORIENTATION_INDEX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("DUAL_PORO_FLAG", ParserItem::itype::STRING);
        item.setDefault( std::string("BOTH") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BOUNDARY::keywordName = "BOUNDARY";
const std::string BOUNDARY::IX1::itemName = "IX1";
const std::string BOUNDARY::IX2::itemName = "IX2";
const std::string BOUNDARY::JY1::itemName = "JY1";
const std::string BOUNDARY::JY2::itemName = "JY2";
const std::string BOUNDARY::KZ1::itemName = "KZ1";
const std::string BOUNDARY::KZ2::itemName = "KZ2";
const std::string BOUNDARY::ORIENTATION_INDEX::itemName = "ORIENTATION_INDEX";
const int BOUNDARY::ORIENTATION_INDEX::defaultValue = 1;
const std::string BOUNDARY::DUAL_PORO_FLAG::itemName = "DUAL_PORO_FLAG";
const std::string BOUNDARY::DUAL_PORO_FLAG::defaultValue = "BOTH";


BOX::BOX() : ParserKeyword("BOX", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("BOX");
  {
     ParserRecord record;
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


BPARA::BPARA() : ParserKeyword("BPARA", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BPARA");
}
const std::string BPARA::keywordName = "BPARA";


BPIDIMS::BPIDIMS() : ParserKeyword("BPIDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BPIDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MXNBIP", ParserItem::itype::INT);
        item.setDefault( 10 );
        record.addItem(item);
     }
     {
        ParserItem item("MXNLBI", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BPIDIMS::keywordName = "BPIDIMS";
const std::string BPIDIMS::MXNBIP::itemName = "MXNBIP";
const int BPIDIMS::MXNBIP::defaultValue = 10;
const std::string BPIDIMS::MXNLBI::itemName = "MXNLBI";
const int BPIDIMS::MXNLBI::defaultValue = 1;


BRANPROP::BRANPROP() : ParserKeyword("BRANPROP", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("BRANPROP");
  {
     ParserRecord record;
     {
        ParserItem item("DOWNTREE_NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("UPTREE_NODE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("VFP_TABLE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ALQ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("ALQ_SURFACE_DENSITY", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BRANPROP::keywordName = "BRANPROP";
const std::string BRANPROP::DOWNTREE_NODE::itemName = "DOWNTREE_NODE";
const std::string BRANPROP::UPTREE_NODE::itemName = "UPTREE_NODE";
const std::string BRANPROP::VFP_TABLE::itemName = "VFP_TABLE";
const std::string BRANPROP::ALQ::itemName = "ALQ";
const double BRANPROP::ALQ::defaultValue = 0;
const std::string BRANPROP::ALQ_SURFACE_DENSITY::itemName = "ALQ_SURFACE_DENSITY";
const std::string BRANPROP::ALQ_SURFACE_DENSITY::defaultValue = "NONE";


BRINE::BRINE() : ParserKeyword("BRINE", KeywordSize(0, 1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BRINE");
  {
     ParserRecord record;
     {
        ParserItem item("SALTS", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BRINE::keywordName = "BRINE";
const std::string BRINE::SALTS::itemName = "SALTS";


BTOBALFA::BTOBALFA() : ParserKeyword("BTOBALFA", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("BTOBALFA");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BTOBALFA::keywordName = "BTOBALFA";
const std::string BTOBALFA::VALUE::itemName = "VALUE";


BTOBALFV::BTOBALFV() : ParserKeyword("BTOBALFV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("BTOBALFV");
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
const std::string BTOBALFV::keywordName = "BTOBALFV";
const std::string BTOBALFV::data::itemName = "data";


}
}
