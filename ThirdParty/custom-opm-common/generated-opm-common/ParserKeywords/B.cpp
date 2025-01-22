
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
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TYPE", ParserItem::itype::STRING);
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
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
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
const std::string BC::DIRECTION::itemName = "DIRECTION";
const std::string BC::TYPE::itemName = "TYPE";
const std::string BC::COMPONENT::itemName = "COMPONENT";
const std::string BC::COMPONENT::defaultValue = "NONE";
const std::string BC::RATE::itemName = "RATE";
const std::string BC::PRESSURE::itemName = "PRESSURE";
const std::string BC::TEMPERATURE::itemName = "TEMPERATURE";


BCCON::BCCON() : ParserKeyword("BCCON", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("BCCON");
  {
     ParserRecord record;
     {
        ParserItem item("INDEX", ParserItem::itype::INT);
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
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BCCON::keywordName = "BCCON";
const std::string BCCON::INDEX::itemName = "INDEX";
const std::string BCCON::I1::itemName = "I1";
const std::string BCCON::I2::itemName = "I2";
const std::string BCCON::J1::itemName = "J1";
const std::string BCCON::J2::itemName = "J2";
const std::string BCCON::K1::itemName = "K1";
const std::string BCCON::K2::itemName = "K2";
const std::string BCCON::DIRECTION::itemName = "DIRECTION";


BCPROP::BCPROP() : ParserKeyword("BCPROP", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("BCPROP");
  {
     ParserRecord record;
     {
        ParserItem item("INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TYPE", ParserItem::itype::STRING);
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
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("MECHTYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("FIXEDX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("FIXEDY", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("FIXEDZ", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("STRESSXX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSYY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSZZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DISPX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DISPY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DISPZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BCPROP::keywordName = "BCPROP";
const std::string BCPROP::INDEX::itemName = "INDEX";
const std::string BCPROP::TYPE::itemName = "TYPE";
const std::string BCPROP::COMPONENT::itemName = "COMPONENT";
const std::string BCPROP::COMPONENT::defaultValue = "NONE";
const std::string BCPROP::RATE::itemName = "RATE";
const std::string BCPROP::PRESSURE::itemName = "PRESSURE";
const std::string BCPROP::TEMPERATURE::itemName = "TEMPERATURE";
const std::string BCPROP::MECHTYPE::itemName = "MECHTYPE";
const std::string BCPROP::MECHTYPE::defaultValue = "NONE";
const std::string BCPROP::FIXEDX::itemName = "FIXEDX";
const std::string BCPROP::FIXEDY::itemName = "FIXEDY";
const std::string BCPROP::FIXEDZ::itemName = "FIXEDZ";
const std::string BCPROP::STRESSXX::itemName = "STRESSXX";
const std::string BCPROP::STRESSYY::itemName = "STRESSYY";
const std::string BCPROP::STRESSZZ::itemName = "STRESSZZ";
const std::string BCPROP::DISPX::itemName = "DISPX";
const std::string BCPROP::DISPY::itemName = "DISPY";
const std::string BCPROP::DISPZ::itemName = "DISPZ";


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


BIC::BIC() : ParserKeyword("BIC", KeywordSize("TABDIMS", "NUM_EOS_RES", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("BIC");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string BIC::keywordName = "BIC";
const std::string BIC::DATA::itemName = "DATA";


BIGMODEL::BIGMODEL() : ParserKeyword("BIGMODEL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("BIGMODEL");
}
const std::string BIGMODEL::keywordName = "BIGMODEL";


BIOTCOEF::BIOTCOEF() : ParserKeyword("BIOTCOEF", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("BIOTCOEF");
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
const std::string BIOTCOEF::keywordName = "BIOTCOEF";
const std::string BIOTCOEF::data::itemName = "data";


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
  addDeckName("BOSAT");
  addDeckName("BSOIL");
  addDeckName("BGIP");
  addDeckName("BVOIL");
  addDeckName("BGKR");
  addDeckName("BOIP");
  addDeckName("BOIPL");
  addDeckName("BGPR");
  addDeckName("BGIPG");
  addDeckName("BSCN_X");
  addDeckName("BOVIS");
  addDeckName("BWVIS");
  addDeckName("BOIPG");
  addDeckName("BRSSAT");
  addDeckName("BPPO");
  addDeckName("BPDEW");
  addDeckName("BDENO");
  addDeckName("BFLOWJ");
  addDeckName("BODEN");
  addDeckName("BVELOJ");
  addDeckName("BVELWI");
  addDeckName("BFLOOI");
  addDeckName("BGDEN");
  addDeckName("BVELWJ");
  addDeckName("BFLOOJ");
  addDeckName("BFLOWI");
  addDeckName("BVELOI");
  addDeckName("LBHDF_X");
  addDeckName("BFLOWK");
  addDeckName("BVELOK");
  addDeckName("BPPC");
  addDeckName("BFLOGK");
  addDeckName("BTRADCAT");
  addDeckName("BWSAT");
  addDeckName("BGSAT");
  addDeckName("BSWAT");
  addDeckName("BPR");
  addDeckName("BWIP");
  addDeckName("BPPW");
  addDeckName("BEWV_SAL");
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
  addDeckName("BGSHY");
  addDeckName("BOPV");
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
  addDeckName("BAPI");
  addDeckName("BSCN");
  addDeckName("BSIP");
  addDeckName("BCAD");
  addDeckName("BTCNFANI");
  addDeckName("BTCNFCAT");
  addDeckName("BTSADCAT");
  addDeckName("BTCASUR");
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
  addDeckName("BTCNFSUR");
  addDeckName("BTPADALK");
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
const std::string BPIDIMS::MXNLBI::itemName = "MXNLBI";


BRANPROP::BRANPROP() : ParserKeyword("BRANPROP", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  setProhibitedKeywords({
    "GRUPNET",
  });
  setRequiredKeywords({
    "NETWORK",
  });
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
