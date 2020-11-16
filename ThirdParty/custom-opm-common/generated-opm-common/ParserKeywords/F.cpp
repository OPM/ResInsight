#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/Parser/ParserItem.hpp>
#include <opm/parser/eclipse/Parser/ParserRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>





#include <opm/parser/eclipse/Parser/ParserKeywords/F.hpp>
namespace Opm {
namespace ParserKeywords {
FAULTDIM::FAULTDIM( ) : ParserKeyword("FAULTDIM")
{
  setFixedSize( (size_t) 1);
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
const int FAULTDIM::MFSEGS::defaultValue = 0;


FAULTS::FAULTS( ) : ParserKeyword("FAULTS")
{
  setSizeType(SLASH_TERMINATED);
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


FBHPDEF::FBHPDEF( ) : ParserKeyword("FBHPDEF")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("FBHPDEF");
  {
     ParserRecord record;
     {
        ParserItem item("TARGET_BHP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("LIMIT_BHP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FBHPDEF::keywordName = "FBHPDEF";
const std::string FBHPDEF::TARGET_BHP::itemName = "TARGET_BHP";
const std::string FBHPDEF::LIMIT_BHP::itemName = "LIMIT_BHP";


FHERCHBL::FHERCHBL( ) : ParserKeyword("FHERCHBL")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("NNEWTF","NTHRBL",0);
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


FIELD::FIELD( ) : ParserKeyword("FIELD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FIELD");
}
const std::string FIELD::keywordName = "FIELD";


FIELD_PROBE::FIELD_PROBE( ) : ParserKeyword("FIELD_PROBE")
{
  setFixedSize( (size_t) 0);
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


FILEUNIT::FILEUNIT( ) : ParserKeyword("FILEUNIT")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
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


FILLEPS::FILLEPS( ) : ParserKeyword("FILLEPS")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FILLEPS");
}
const std::string FILLEPS::keywordName = "FILLEPS";


FIPNUM::FIPNUM( ) : ParserKeyword("FIPNUM")
{
  setFixedSize( (size_t) 1);
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


FIPOWG::FIPOWG( ) : ParserKeyword("FIPOWG")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("FIPOWG");
}
const std::string FIPOWG::keywordName = "FIPOWG";


FIPSEP::FIPSEP( ) : ParserKeyword("FIPSEP")
{
  setSizeType(SLASH_TERMINATED);
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
const double FIPSEP::STAGE_TEMPERATURE::defaultValue = 15.560000;
const std::string FIPSEP::STAGE_PRESSURE::itemName = "STAGE_PRESSURE";
const double FIPSEP::STAGE_PRESSURE::defaultValue = 1.013250;
const std::string FIPSEP::DESTINATION_OUPUT::itemName = "DESTINATION_OUPUT";
const int FIPSEP::DESTINATION_OUPUT::defaultValue = 0;
const std::string FIPSEP::DESTINATION_STAGE::itemName = "DESTINATION_STAGE";
const int FIPSEP::DESTINATION_STAGE::defaultValue = 0;
const std::string FIPSEP::K_VAL_TABLE_NUM::itemName = "K_VAL_TABLE_NUM";
const int FIPSEP::K_VAL_TABLE_NUM::defaultValue = 0;
const std::string FIPSEP::GAS_PLANT_TABLE_NUM::itemName = "GAS_PLANT_TABLE_NUM";
const int FIPSEP::GAS_PLANT_TABLE_NUM::defaultValue = 0;
const std::string FIPSEP::SURF_EQ_STATE_NUM::itemName = "SURF_EQ_STATE_NUM";
const std::string FIPSEP::DENSITY_EVAL_GAS_TEMP::itemName = "DENSITY_EVAL_GAS_TEMP";
const std::string FIPSEP::DENSITY_EVAL_PRESSURE_TEMP::itemName = "DENSITY_EVAL_PRESSURE_TEMP";


FIP_PROBE::FIP_PROBE( ) : ParserKeyword("FIP_PROBE")
{
  setFixedSize( (size_t) 1);
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


FLUXNUM::FLUXNUM( ) : ParserKeyword("FLUXNUM")
{
  setFixedSize( (size_t) 1);
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


FLUXREG::FLUXREG( ) : ParserKeyword("FLUXREG")
{
  setFixedSize( (size_t) 1);
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


FLUXTYPE::FLUXTYPE( ) : ParserKeyword("FLUXTYPE")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("FLUXTYPE");
}
const std::string FLUXTYPE::keywordName = "FLUXTYPE";


FMTHMD::FMTHMD( ) : ParserKeyword("FMTHMD")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTHMD");
}
const std::string FMTHMD::keywordName = "FMTHMD";


FMTIN::FMTIN( ) : ParserKeyword("FMTIN")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTIN");
}
const std::string FMTIN::keywordName = "FMTIN";


FMTOUT::FMTOUT( ) : ParserKeyword("FMTOUT")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FMTOUT");
}
const std::string FMTOUT::keywordName = "FMTOUT";


FMWSET::FMWSET( ) : ParserKeyword("FMWSET")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("FMWSET");
}
const std::string FMWSET::keywordName = "FMWSET";


FOAM::FOAM( ) : ParserKeyword("FOAM")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FOAM");
}
const std::string FOAM::keywordName = "FOAM";


FOAMADS::FOAMADS( ) : ParserKeyword("FOAMADS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMDCYO::FOAMDCYO( ) : ParserKeyword("FOAMDCYO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMDCYW::FOAMDCYW( ) : ParserKeyword("FOAMDCYW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMFCN::FOAMFCN( ) : ParserKeyword("FOAMFCN")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("FOAMFCN");
  {
     ParserRecord record;
     {
        ParserItem item("CAPILLARY_NUMBER", ParserItem::itype::INT);
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
const double FOAMFCN::EXP::defaultValue = 1.000000;


FOAMFRM::FOAMFRM( ) : ParserKeyword("FOAMFRM")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMFSC::FOAMFSC( ) : ParserKeyword("FOAMFSC")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
  addValidSectionName("PROPS");
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
     addRecord( record );
  }
}
const std::string FOAMFSC::keywordName = "FOAMFSC";
const std::string FOAMFSC::REF_SURF_CONC::itemName = "REF_SURF_CONC";
const std::string FOAMFSC::EXPONENT::itemName = "EXPONENT";
const double FOAMFSC::EXPONENT::defaultValue = 1.000000;
const std::string FOAMFSC::MIN_SURF_CONC::itemName = "MIN_SURF_CONC";
const double FOAMFSC::MIN_SURF_CONC::defaultValue = 0.000000;


FOAMFSO::FOAMFSO( ) : ParserKeyword("FOAMFSO")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMFST::FOAMFST( ) : ParserKeyword("FOAMFST")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMFSW::FOAMFSW( ) : ParserKeyword("FOAMFSW")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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


FOAMMOB::FOAMMOB( ) : ParserKeyword("FOAMMOB")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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


FOAMMOBP::FOAMMOBP( ) : ParserKeyword("FOAMMOBP")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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


FOAMMOBS::FOAMMOBS( ) : ParserKeyword("FOAMMOBS")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTPVT",0);
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


FOAMOPTS::FOAMOPTS( ) : ParserKeyword("FOAMOPTS")
{
  setFixedSize( (size_t) 1);
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
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string FOAMOPTS::keywordName = "FOAMOPTS";
const std::string FOAMOPTS::TRANSPORT_PHASE::itemName = "TRANSPORT_PHASE";
const std::string FOAMOPTS::TRANSPORT_PHASE::defaultValue = "GAS";
const std::string FOAMOPTS::MODEL::itemName = "MODEL";


FOAMROCK::FOAMROCK( ) : ParserKeyword("FOAMROCK")
{
  setSizeType(OTHER_KEYWORD_IN_DECK);
  initSizeKeyword("TABDIMS","NTSFUN",0);
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
const int FOAMROCK::ADSORPTION_INDEX::defaultValue = 1;
const std::string FOAMROCK::ROCK_DENSITY::itemName = "ROCK_DENSITY";


FORMFEED::FORMFEED( ) : ParserKeyword("FORMFEED")
{
  setFixedSize( (size_t) 1);
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
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
const int FORMFEED::VALUE::defaultValue = 1;


FRICTION::FRICTION( ) : ParserKeyword("FRICTION")
{
  setFixedSize( (size_t) 1);
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
const int FRICTION::NWFRIC::defaultValue = 0;
const std::string FRICTION::NWFRIB::itemName = "NWFRIB";
const int FRICTION::NWFRIB::defaultValue = 1;


FULLIMP::FULLIMP( ) : ParserKeyword("FULLIMP")
{
  setFixedSize( (size_t) 0);
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("FULLIMP");
}
const std::string FULLIMP::keywordName = "FULLIMP";


}
}
