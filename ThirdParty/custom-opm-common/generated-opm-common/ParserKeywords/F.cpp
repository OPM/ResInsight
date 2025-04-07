
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/F.hpp>
namespace Opm {
namespace ParserKeywords {
FAULTDIM::FAULTDIM() : ParserKeyword("FAULTDIM", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FAULTDIM");
  {
     ParserRecord record;
     {
        ParserItem item("MFSEGS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FAULTDIM::keywordName = "FAULTDIM";
const std::string FAULTDIM::MFSEGS::itemName = "MFSEGS";


FAULTS::FAULTS() : ParserKeyword("FAULTS", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FAULTS");
  {
     ParserRecord record;
     {
        ParserItem item("NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("IX1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IY1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IY2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IZ1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IZ2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE", ParserItem::itype::STRING);
        record.addItem(item);
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


FBHPDEF::FBHPDEF() : ParserKeyword("FBHPDEF", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("FBHPDEF");
  {
     ParserRecord record;
     {
        ParserItem item("TARGET_BHP", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.013250) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_BHP", ParserItem::itype::DOUBLE);
        item.setDefault( double(6895.000000) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FBHPDEF::keywordName = "FBHPDEF";
const std::string FBHPDEF::TARGET_BHP::itemName = "TARGET_BHP";
const std::string FBHPDEF::LIMIT_BHP::itemName = "LIMIT_BHP";


FHERCHBL::FHERCHBL() : ParserKeyword("FHERCHBL", KeywordSize("NNEWTF", "NTHRBL", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FHERCHBL");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FHERCHBL::keywordName = "FHERCHBL";
const std::string FHERCHBL::DATA::itemName = "DATA";


FIELD::FIELD() : ParserKeyword("FIELD", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FIELD");
}
const std::string FIELD::keywordName = "FIELD";


FIELDSEP::FIELDSEP() : ParserKeyword("FIELDSEP", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("FIELDSEP");
  {
     ParserRecord record;
     {
        ParserItem item("STAGE", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(15.560000) );
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.013250) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("LIQ_DESTINATION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("VAP_DESTINATION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("KVALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("EOS_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("REF_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("REF_PRESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FIELDSEP::keywordName = "FIELDSEP";
const std::string FIELDSEP::STAGE::itemName = "STAGE";
const std::string FIELDSEP::TEMPERATURE::itemName = "TEMPERATURE";
const std::string FIELDSEP::PRESSURE::itemName = "PRESSURE";
const std::string FIELDSEP::LIQ_DESTINATION::itemName = "LIQ_DESTINATION";
const std::string FIELDSEP::VAP_DESTINATION::itemName = "VAP_DESTINATION";
const std::string FIELDSEP::KVALUE::itemName = "KVALUE";
const std::string FIELDSEP::TABLE_NUM::itemName = "TABLE_NUM";
const std::string FIELDSEP::EOS_NUM::itemName = "EOS_NUM";
const std::string FIELDSEP::REF_TEMP::itemName = "REF_TEMP";
const std::string FIELDSEP::REF_PRESS::itemName = "REF_PRESS";


FIELD_PROBE::FIELD_PROBE() : ParserKeyword("FIELD_PROBE", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("FOPR");
  addDeckName("FGCDI");
  addDeckName("FORFF");
  addDeckName("FOPRH");
  addDeckName("FCSC");
  addDeckName("FOPTF");
  addDeckName("FGIMT");
  addDeckName("FGPTF");
  addDeckName("FSPR");
  addDeckName("FGPP2");
  addDeckName("FOPRT");
  addDeckName("FGLRH");
  addDeckName("FGPRF");
  addDeckName("FOPT");
  addDeckName("FOPRF");
  addDeckName("FGST");
  addDeckName("FMWPL");
  addDeckName("FOPI2");
  addDeckName("FOPRS");
  addDeckName("FOPI");
  addDeckName("FGPP");
  addDeckName("FOPTH");
  addDeckName("FGIRT");
  addDeckName("FOPTS");
  addDeckName("FCIP");
  addDeckName("FOIR");
  addDeckName("FOIRH");
  addDeckName("FCIT");
  addDeckName("FWIT");
  addDeckName("FOIRT");
  addDeckName("FOIT");
  addDeckName("FGPRS");
  addDeckName("FWPI2");
  addDeckName("FOITH");
  addDeckName("FOPP");
  addDeckName("FWPRT");
  addDeckName("FOPP2");
  addDeckName("FWPRH");
  addDeckName("FWPR");
  addDeckName("FMWPO");
  addDeckName("FWPT");
  addDeckName("FWPTH");
  addDeckName("FCIC");
  addDeckName("FWIR");
  addDeckName("FGSR");
  addDeckName("FWIRH");
  addDeckName("FWIRT");
  addDeckName("FWITH");
  addDeckName("FGIMR");
  addDeckName("FSPT");
  addDeckName("FWPP");
  addDeckName("FWPP2");
  addDeckName("FGPI2");
  addDeckName("FORFE");
  addDeckName("FWPI");
  addDeckName("FMWPT");
  addDeckName("FGPI");
  addDeckName("FWPIR");
  addDeckName("FGPPF");
  addDeckName("FGPR");
  addDeckName("FGPRH");
  addDeckName("FGPRT");
  addDeckName("FGPT");
  addDeckName("FGSAT");
  addDeckName("FGPTH");
  addDeckName("FGPTS");
  addDeckName("FGIR");
  addDeckName("FGCR");
  addDeckName("FSGR");
  addDeckName("FMWIA");
  addDeckName("FGPPS2");
  addDeckName("FGIRH");
  addDeckName("FGIT");
  addDeckName("FGITH");
  addDeckName("FOSRL");
  addDeckName("FGPPS");
  addDeckName("FGPPF2");
  addDeckName("FGCT");
  addDeckName("FSGT");
  addDeckName("FMWIG");
  addDeckName("FWCD");
  addDeckName("FGLIR");
  addDeckName("FJPRH");
  addDeckName("FGQ");
  addDeckName("FLPR");
  addDeckName("FLPRH");
  addDeckName("FSIP");
  addDeckName("FTICHEA");
  addDeckName("FLPRT");
  addDeckName("FLPT");
  addDeckName("FLPTH");
  addDeckName("FOEIW");
  addDeckName("FJPR");
  addDeckName("FJPRT");
  addDeckName("FJPT");
  addDeckName("FJPTH");
  addDeckName("FEIR");
  addDeckName("FVPR");
  addDeckName("FVPRT");
  addDeckName("FVPT");
  addDeckName("FGDEN");
  addDeckName("FVIR");
  addDeckName("FVIRT");
  addDeckName("FVIT");
  addDeckName("FWCT");
  addDeckName("FWCTH");
  addDeckName("FORFG");
  addDeckName("FGOR");
  addDeckName("FGORH");
  addDeckName("FOGR");
  addDeckName("FOGRH");
  addDeckName("FORMX");
  addDeckName("FWGR");
  addDeckName("FWGRH");
  addDeckName("FGLR");
  addDeckName("FMPT");
  addDeckName("FMCTP");
  addDeckName("FPRP");
  addDeckName("FMCTW");
  addDeckName("FMCTG");
  addDeckName("FOVIS");
  addDeckName("FMWPR");
  addDeckName("FMWPA");
  addDeckName("FMWPU");
  addDeckName("FMWPG");
  addDeckName("FMWPS");
  addDeckName("FMWPV");
  addDeckName("FMWPP");
  addDeckName("FMWIT");
  addDeckName("FMWIN");
  addDeckName("FMWIU");
  addDeckName("FMWIS");
  addDeckName("FMWIV");
  addDeckName("FMWIP");
  addDeckName("FMWDR");
  addDeckName("FMWDT");
  addDeckName("FMWWO");
  addDeckName("FMWWT");
  addDeckName("FEPR");
  addDeckName("FEPT");
  addDeckName("FTITSUR");
  addDeckName("FGCDM");
  addDeckName("FOPV");
  addDeckName("FGSPR");
  addDeckName("FGSRL");
  addDeckName("FGSRU");
  addDeckName("FGSSP");
  addDeckName("FGSTP");
  addDeckName("FMIR");
  addDeckName("FOSPR");
  addDeckName("FOSRU");
  addDeckName("FOSSP");
  addDeckName("FOSTP");
  addDeckName("FWIPG");
  addDeckName("FWIPL");
  addDeckName("FWSPR");
  addDeckName("FWSRL");
  addDeckName("FTPTCAT");
  addDeckName("FWSRU");
  addDeckName("FWSSP");
  addDeckName("FWSTP");
  addDeckName("FOSAT");
  addDeckName("FOIP");
  addDeckName("FOIPR");
  addDeckName("FOIPL");
  addDeckName("FOIPG");
  addDeckName("FPPO");
  addDeckName("FODEN");
  addDeckName("FMIT");
  addDeckName("FWSAT");
  addDeckName("FWIP");
  addDeckName("FWIPR");
  addDeckName("FPPW");
  addDeckName("FWVIS");
  addDeckName("FORMS");
  addDeckName("FWDEN");
  addDeckName("FGIP");
  addDeckName("FGIPR");
  addDeckName("FGIPL");
  addDeckName("FGIPG");
  addDeckName("FPPG");
  addDeckName("FGVIS");
  addDeckName("FPR");
  addDeckName("FCAD");
  addDeckName("FPRH");
  addDeckName("FPRGZ");
  addDeckName("FRS");
  addDeckName("FRV");
  addDeckName("FSIR");
  addDeckName("FPPC");
  addDeckName("FRPV");
  addDeckName("FWPV");
  addDeckName("FGPV");
  addDeckName("FHPV");
  addDeckName("FRTM");
  addDeckName("FOE");
  addDeckName("FOEW");
  addDeckName("FOEWW");
  addDeckName("FAPI");
  addDeckName("FOEIG");
  addDeckName("FOEWG");
  addDeckName("FORMR");
  addDeckName("FTPTALK");
  addDeckName("FORMW");
  addDeckName("FORMG");
  addDeckName("FNIT");
  addDeckName("FORME");
  addDeckName("FORMF");
  addDeckName("FORMY");
  addDeckName("FORFR");
  addDeckName("FORFW");
  addDeckName("FORFS");
  addDeckName("FORFX");
  addDeckName("FORFY");
  addDeckName("FAQR");
  addDeckName("FAQT");
  addDeckName("FAQRG");
  addDeckName("FAQTG");
  addDeckName("FNQR");
  addDeckName("FNQT");
  addDeckName("FSIT");
  addDeckName("FSPC");
  addDeckName("FSIC");
  addDeckName("FTPRANI");
  addDeckName("FTPTANI");
  addDeckName("FTIRANI");
  addDeckName("FTITANI");
  addDeckName("FTPRCAT");
  addDeckName("FTPTSUR");
  addDeckName("FTIRCAT");
  addDeckName("FTITCAT");
  addDeckName("FTPCHEA");
  addDeckName("FTPRHEA");
  addDeckName("FTPTHEA");
  addDeckName("FTIRHEA");
  addDeckName("FTITHEA");
  addDeckName("FTIPTHEA");
  addDeckName("FMPR");
  addDeckName("FCGC");
  addDeckName("FTPRFOA");
  addDeckName("FTPTFOA");
  addDeckName("FTIRFOA");
  addDeckName("FTITFOA");
  addDeckName("FTIPTFOA");
  addDeckName("FNIP");
  addDeckName("FTADSFOA");
  addDeckName("FTDCYFOA");
  addDeckName("FTMOBFOA");
  addDeckName("FGDC");
  addDeckName("FGDCQ");
  addDeckName("FEIT");
  addDeckName("FCPR");
  addDeckName("FCPC");
  addDeckName("FCPT");
  addDeckName("FCIR");
  addDeckName("PSSPR");
  addDeckName("PSSSO");
  addDeckName("PSSSW");
  addDeckName("PSSSG");
  addDeckName("PSSSC");
  addDeckName("FNPR");
  addDeckName("FNPT");
  addDeckName("FNIR");
  addDeckName("FTPRSUR");
  addDeckName("FTIRSUR");
  addDeckName("FTIRALK");
  addDeckName("FTIPTSUR");
  addDeckName("FTADSUR");
  addDeckName("FTPRALK");
  addDeckName("FTITALK");
  setMatchRegex("FU.+|FTPR.+|FTPT.+|FTPC.+|FTIR.+|FTIT.+|FTIC.+|FTIPT.+|FTIPF.+|FTIPS|FTIP[1-9][0-9]*.+|FTPR.+|FTPT.+|FTPC.+|FTIR.+|FTIT.+|FTIC.+|FTADS.+|FTDCY.+|FTIRF.+|FTIRS.+|FTPRF.+|FTPRS.+|FTITF.+|FTITS.+|FTPTF.+|FTPTS.+|FTICF.+|FTICS.+|FTPCF.+|FTPCS.+");
}
const std::string FIELD_PROBE::keywordName = "FIELD_PROBE";


FIELD_PROBE_OPM::FIELD_PROBE_OPM() : ParserKeyword("FIELD_PROBE_OPM", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("FGMIP");
  addDeckName("FGKDI");
  addDeckName("FGMDS");
  addDeckName("FGKDM");
  addDeckName("FGKMO");
  addDeckName("FGKTR");
  addDeckName("FGMIR");
  addDeckName("FGMGP");
  addDeckName("FGMIT");
  addDeckName("FGMMO");
  addDeckName("FGMST");
  addDeckName("FGMTR");
  addDeckName("FGMUS");
}
const std::string FIELD_PROBE_OPM::keywordName = "FIELD_PROBE_OPM";


FILEUNIT::FILEUNIT() : ParserKeyword("FILEUNIT", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("FILEUNIT");
  {
     ParserRecord record;
     {
        ParserItem item("FILE_UNIT_SYSTEM", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FILEUNIT::keywordName = "FILEUNIT";
const std::string FILEUNIT::FILE_UNIT_SYSTEM::itemName = "FILE_UNIT_SYSTEM";


FILLEPS::FILLEPS() : ParserKeyword("FILLEPS", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FILLEPS");
}
const std::string FILLEPS::keywordName = "FILLEPS";


FIPNUM::FIPNUM() : ParserKeyword("FIPNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("FIPNUM");
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
const std::string FIPNUM::keywordName = "FIPNUM";
const std::string FIPNUM::data::itemName = "data";


FIPOWG::FIPOWG() : ParserKeyword("FIPOWG", KeywordSize(0, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("FIPOWG");
}
const std::string FIPOWG::keywordName = "FIPOWG";


FIPSEP::FIPSEP() : ParserKeyword("FIPSEP", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("FIPSEP");
  {
     ParserRecord record;
     {
        ParserItem item("FLUID_IN_PLACE_REGION", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("STAGE_INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("STAGE_TEMPERATURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(15.560000) );
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("STAGE_PRESSURE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.013250) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("DESTINATION_OUPUT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("DESTINATION_STAGE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("K_VAL_TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("GAS_PLANT_TABLE_NUM", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("SURF_EQ_STATE_NUM", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DENSITY_EVAL_GAS_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     {
        ParserItem item("DENSITY_EVAL_PRESSURE_TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FIPSEP::keywordName = "FIPSEP";
const std::string FIPSEP::FLUID_IN_PLACE_REGION::itemName = "FLUID_IN_PLACE_REGION";
const std::string FIPSEP::STAGE_INDEX::itemName = "STAGE_INDEX";
const std::string FIPSEP::STAGE_TEMPERATURE::itemName = "STAGE_TEMPERATURE";
const std::string FIPSEP::STAGE_PRESSURE::itemName = "STAGE_PRESSURE";
const std::string FIPSEP::DESTINATION_OUPUT::itemName = "DESTINATION_OUPUT";
const std::string FIPSEP::DESTINATION_STAGE::itemName = "DESTINATION_STAGE";
const std::string FIPSEP::K_VAL_TABLE_NUM::itemName = "K_VAL_TABLE_NUM";
const std::string FIPSEP::GAS_PLANT_TABLE_NUM::itemName = "GAS_PLANT_TABLE_NUM";
const std::string FIPSEP::SURF_EQ_STATE_NUM::itemName = "SURF_EQ_STATE_NUM";
const std::string FIPSEP::DENSITY_EVAL_GAS_TEMP::itemName = "DENSITY_EVAL_GAS_TEMP";
const std::string FIPSEP::DENSITY_EVAL_PRESSURE_TEMP::itemName = "DENSITY_EVAL_PRESSURE_TEMP";


FIP_PROBE::FIP_PROBE() : ParserKeyword("FIP_PROBE", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  setMatchRegex("FIP.+");
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
const std::string FIP_PROBE::keywordName = "FIP_PROBE";
const std::string FIP_PROBE::data::itemName = "data";


FLUXNUM::FLUXNUM() : ParserKeyword("FLUXNUM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FLUXNUM");
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
const std::string FLUXNUM::keywordName = "FLUXNUM";
const std::string FLUXNUM::data::itemName = "data";


FLUXREG::FLUXREG() : ParserKeyword("FLUXREG", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FLUXREG");
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
const std::string FLUXREG::keywordName = "FLUXREG";
const std::string FLUXREG::data::itemName = "data";


FLUXTYPE::FLUXTYPE() : ParserKeyword("FLUXTYPE", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FLUXTYPE");
  {
     ParserRecord record;
     {
        ParserItem item("BC_TYPE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FLUXTYPE::keywordName = "FLUXTYPE";
const std::string FLUXTYPE::BC_TYPE::itemName = "BC_TYPE";


FMTHMD::FMTHMD() : ParserKeyword("FMTHMD", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTHMD");
}
const std::string FMTHMD::keywordName = "FMTHMD";


FMTIN::FMTIN() : ParserKeyword("FMTIN", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTIN");
}
const std::string FMTIN::keywordName = "FMTIN";


FMTOUT::FMTOUT() : ParserKeyword("FMTOUT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTOUT");
}
const std::string FMTOUT::keywordName = "FMTOUT";


FMWSET::FMWSET() : ParserKeyword("FMWSET", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("FMWSET");
}
const std::string FMWSET::keywordName = "FMWSET";


FOAM::FOAM() : ParserKeyword("FOAM", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FOAM");
}
const std::string FOAM::keywordName = "FOAM";


FOAMADS::FOAMADS() : ParserKeyword("FOAMADS", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMADS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("FoamDensity");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMADS::keywordName = "FOAMADS";
const std::string FOAMADS::DATA::itemName = "DATA";


FOAMDCYO::FOAMDCYO() : ParserKeyword("FOAMDCYO", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMDCYO");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMDCYO::keywordName = "FOAMDCYO";
const std::string FOAMDCYO::DATA::itemName = "DATA";


FOAMDCYW::FOAMDCYW() : ParserKeyword("FOAMDCYW", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMDCYW");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMDCYW::keywordName = "FOAMDCYW";
const std::string FOAMDCYW::DATA::itemName = "DATA";


FOAMFCN::FOAMFCN() : ParserKeyword("FOAMFCN", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFCN");
  {
     ParserRecord record;
     {
        ParserItem item("CAPILLARY_NUMBER", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("EXP", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMFCN::keywordName = "FOAMFCN";
const std::string FOAMFCN::CAPILLARY_NUMBER::itemName = "CAPILLARY_NUMBER";
const std::string FOAMFCN::EXP::itemName = "EXP";


FOAMFRM::FOAMFRM() : ParserKeyword("FOAMFRM", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFRM");
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
const std::string FOAMFRM::keywordName = "FOAMFRM";
const std::string FOAMFRM::DATA::itemName = "DATA";


FOAMFSC::FOAMFSC() : ParserKeyword("FOAMFSC", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "FOAMROCK",
  });
  clearDeckNames();
  addDeckName("FOAMFSC");
  {
     ParserRecord record;
     {
        ParserItem item("REF_SURF_CONC", ParserItem::itype::DOUBLE);
        item.push_backDimension("FoamSurfactantConcentration");
        record.addItem(item);
     }
     {
        ParserItem item("EXPONENT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_SURF_CONC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-20) );
        item.push_backDimension("FoamSurfactantConcentration");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_WAT_SAT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMFSC::keywordName = "FOAMFSC";
const std::string FOAMFSC::REF_SURF_CONC::itemName = "REF_SURF_CONC";
const std::string FOAMFSC::EXPONENT::itemName = "EXPONENT";
const std::string FOAMFSC::MIN_SURF_CONC::itemName = "MIN_SURF_CONC";
const std::string FOAMFSC::MIN_WAT_SAT::itemName = "MIN_WAT_SAT";


FOAMFSO::FOAMFSO() : ParserKeyword("FOAMFSO", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFSO");
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
const std::string FOAMFSO::keywordName = "FOAMFSO";
const std::string FOAMFSO::DATA::itemName = "DATA";


FOAMFST::FOAMFST() : ParserKeyword("FOAMFST", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFST");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("FoamDensity");
        item.push_backDimension("SurfaceTension");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMFST::keywordName = "FOAMFST";
const std::string FOAMFST::DATA::itemName = "DATA";


FOAMFSW::FOAMFSW() : ParserKeyword("FOAMFSW", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFSW");
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
const std::string FOAMFSW::keywordName = "FOAMFSW";
const std::string FOAMFSW::DATA::itemName = "DATA";


FOAMMOB::FOAMMOB() : ParserKeyword("FOAMMOB", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMMOB");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("FoamDensity");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMMOB::keywordName = "FOAMMOB";
const std::string FOAMMOB::DATA::itemName = "DATA";


FOAMMOBP::FOAMMOBP() : ParserKeyword("FOAMMOBP", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMMOBP");
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
const std::string FOAMMOBP::keywordName = "FOAMMOBP";
const std::string FOAMMOBP::DATA::itemName = "DATA";


FOAMMOBS::FOAMMOBS() : ParserKeyword("FOAMMOBS", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMMOBS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length/Time");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMMOBS::keywordName = "FOAMMOBS";
const std::string FOAMMOBS::DATA::itemName = "DATA";


FOAMOPTS::FOAMOPTS() : ParserKeyword("FOAMOPTS", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("TRANSPORT_PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("GAS") );
        record.addItem(item);
     }
     {
        ParserItem item("MODEL", ParserItem::itype::STRING);
        item.setDefault( std::string("TAB") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMOPTS::keywordName = "FOAMOPTS";
const std::string FOAMOPTS::TRANSPORT_PHASE::itemName = "TRANSPORT_PHASE";
const std::string FOAMOPTS::TRANSPORT_PHASE::defaultValue = "GAS";
const std::string FOAMOPTS::MODEL::itemName = "MODEL";
const std::string FOAMOPTS::MODEL::defaultValue = "TAB";


FOAMROCK::FOAMROCK() : ParserKeyword("FOAMROCK", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMROCK");
  {
     ParserRecord record;
     {
        ParserItem item("ADSORPTION_INDEX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("ROCK_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMROCK::keywordName = "FOAMROCK";
const std::string FOAMROCK::ADSORPTION_INDEX::itemName = "ADSORPTION_INDEX";
const std::string FOAMROCK::ROCK_DENSITY::itemName = "ROCK_DENSITY";


FORMFEED::FORMFEED() : ParserKeyword("FORMFEED", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("FORMFEED");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FORMFEED::keywordName = "FORMFEED";
const std::string FORMFEED::VALUE::itemName = "VALUE";


FRICTION::FRICTION() : ParserKeyword("FRICTION", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FRICTION");
  {
     ParserRecord record;
     {
        ParserItem item("NWFRIC", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NWFRIB", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FRICTION::keywordName = "FRICTION";
const std::string FRICTION::NWFRIC::itemName = "NWFRIC";
const std::string FRICTION::NWFRIB::itemName = "NWFRIB";


FULLIMP::FULLIMP() : ParserKeyword("FULLIMP", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FULLIMP");
}
const std::string FULLIMP::keywordName = "FULLIMP";


}
}
