
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/S.hpp>
namespace Opm {
namespace ParserKeywords {
SALINITY::SALINITY() : ParserKeyword("SALINITY", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "SALTMF",
  });
  clearDeckNames();
  addDeckName("SALINITY");
  {
     ParserRecord record;
     {
        ParserItem item("MOLALITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALINITY::keywordName = "SALINITY";
const std::string SALINITY::MOLALITY::itemName = "MOLALITY";


SALT::SALT() : ParserKeyword("SALT", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SALT");
  {
     ParserRecord record;
     {
        ParserItem item("SALT_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALT::keywordName = "SALT";
const std::string SALT::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";


SALTMF::SALTMF() : ParserKeyword("SALTMF", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "SALINITY",
  });
  clearDeckNames();
  addDeckName("SALTMF");
  {
     ParserRecord record;
     {
        ParserItem item("MOLE_FRACTION", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTMF::keywordName = "SALTMF";
const std::string SALTMF::MOLE_FRACTION::itemName = "MOLE_FRACTION";


SALTNODE::SALTNODE() : ParserKeyword("SALTNODE", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SALTNODE");
  {
     ParserRecord record;
     {
        ParserItem item("SALT_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTNODE::keywordName = "SALTNODE";
const std::string SALTNODE::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";


SALTP::SALTP() : ParserKeyword("SALTP", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SALTP");
  {
     ParserRecord record;
     {
        ParserItem item("SALT_SATURATION", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTP::keywordName = "SALTP";
const std::string SALTP::SALT_SATURATION::itemName = "SALT_SATURATION";


SALTPVD::SALTPVD() : ParserKeyword("SALTPVD", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SALTPVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTPVD::keywordName = "SALTPVD";
const std::string SALTPVD::DATA::itemName = "DATA";


SALTREST::SALTREST() : ParserKeyword("SALTREST", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SALTREST");
  {
     ParserRecord record;
     {
        ParserItem item("SALT_CONCENTRATION", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTREST::keywordName = "SALTREST";
const std::string SALTREST::SALT_CONCENTRATION::itemName = "SALT_CONCENTRATION";


SALTSOL::SALTSOL() : ParserKeyword("SALTSOL", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SALTSOL");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTSOL::keywordName = "SALTSOL";
const std::string SALTSOL::DATA::itemName = "DATA";


SALTVD::SALTVD() : ParserKeyword("SALTVD", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SALTVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Salinity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SALTVD::keywordName = "SALTVD";
const std::string SALTVD::DATA::itemName = "DATA";


SAMG::SAMG() : ParserKeyword("SAMG", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SAMG");
  {
     ParserRecord record;
     {
        ParserItem item("EPS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("REUSE", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SAMG::keywordName = "SAMG";
const std::string SAMG::EPS::itemName = "EPS";
const std::string SAMG::REUSE::itemName = "REUSE";


SATNUM::SATNUM() : ParserKeyword("SATNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("SATNUM");
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
const std::string SATNUM::keywordName = "SATNUM";
const std::string SATNUM::data::itemName = "data";


SATOPTS::SATOPTS() : ParserKeyword("SATOPTS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SATOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("options", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SATOPTS::keywordName = "SATOPTS";
const std::string SATOPTS::options::itemName = "options";


SAVE::SAVE() : ParserKeyword("SAVE", KeywordSize(0, 1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SAVE");
  {
     ParserRecord record;
     {
        ParserItem item("FILE_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("UNFORMATTED") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SAVE::keywordName = "SAVE";
const std::string SAVE::FILE_TYPE::itemName = "FILE_TYPE";
const std::string SAVE::FILE_TYPE::defaultValue = "UNFORMATTED";


SBIOF::SBIOF() : ParserKeyword("SBIOF", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SBIOF");
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
const std::string SBIOF::keywordName = "SBIOF";
const std::string SBIOF::data::itemName = "data";


SCALC::SCALC() : ParserKeyword("SCALC", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SCALC");
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
const std::string SCALC::keywordName = "SCALC";
const std::string SCALC::data::itemName = "data";


SCALECRS::SCALECRS() : ParserKeyword("SCALECRS", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SCALECRS");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCALECRS::keywordName = "SCALECRS";
const std::string SCALECRS::VALUE::itemName = "VALUE";
const std::string SCALECRS::VALUE::defaultValue = "NO";


SCALELIM::SCALELIM() : ParserKeyword("SCALELIM", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SCALELIM");
  {
     ParserRecord record;
     {
        ParserItem item("SAT_LIMIT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCALELIM::keywordName = "SCALELIM";
const std::string SCALELIM::SAT_LIMIT::itemName = "SAT_LIMIT";


SCDATAB::SCDATAB() : ParserKeyword("SCDATAB", KeywordSize("SCDPDIMS", "NTSCDA", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SCDATAB");
  {
     ParserRecord record;
     {
        ParserItem item("SCALE_DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCDATAB::keywordName = "SCDATAB";
const std::string SCDATAB::SCALE_DATA::itemName = "SCALE_DATA";


SCDETAB::SCDETAB() : ParserKeyword("SCDETAB", KeywordSize("SCDPDIMS", "NTSCDE", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SCDETAB");
  {
     ParserRecord record;
     {
        ParserItem item("SCALE_DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCDETAB::keywordName = "SCDETAB";
const std::string SCDETAB::SCALE_DATA::itemName = "SCALE_DATA";


SCDPDIMS::SCDPDIMS() : ParserKeyword("SCDPDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SCDPDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTSCDP", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NPSCDP", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NTSCDA", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("PSCDA", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UNUSED1", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UNUSED2", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("NTSCDE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCDPDIMS::keywordName = "SCDPDIMS";
const std::string SCDPDIMS::NTSCDP::itemName = "NTSCDP";
const std::string SCDPDIMS::NPSCDP::itemName = "NPSCDP";
const std::string SCDPDIMS::NTSCDA::itemName = "NTSCDA";
const std::string SCDPDIMS::PSCDA::itemName = "PSCDA";
const std::string SCDPDIMS::UNUSED1::itemName = "UNUSED1";
const std::string SCDPDIMS::UNUSED2::itemName = "UNUSED2";
const std::string SCDPDIMS::NTSCDE::itemName = "NTSCDE";


SCDPTAB::SCDPTAB() : ParserKeyword("SCDPTAB", KeywordSize("SCDPDIMS", "NTSCDP", false, 0)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SCDPTAB");
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
const std::string SCDPTAB::keywordName = "SCDPTAB";
const std::string SCDPTAB::DATA::itemName = "DATA";


SCDPTRAC::SCDPTRAC() : ParserKeyword("SCDPTRAC", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SCDPTRAC");
  {
     ParserRecord record;
     {
        ParserItem item("TRACER", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCDPTRAC::keywordName = "SCDPTRAC";
const std::string SCDPTRAC::TRACER::itemName = "TRACER";


SCHEDULE::SCHEDULE() : ParserKeyword("SCHEDULE", KeywordSize(0, false)) {
  clearDeckNames();
  addDeckName("SCHEDULE");
}
const std::string SCHEDULE::keywordName = "SCHEDULE";


SCVD::SCVD() : ParserKeyword("SCVD", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SCVD::keywordName = "SCVD";
const std::string SCVD::DATA::itemName = "DATA";


SDENSITY::SDENSITY() : ParserKeyword("SDENSITY", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SDENSITY");
  {
     ParserRecord record;
     {
        ParserItem item("SOLVENT_DENSITY", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SDENSITY::keywordName = "SDENSITY";
const std::string SDENSITY::SOLVENT_DENSITY::itemName = "SOLVENT_DENSITY";


SEGMENT_PROBE::SEGMENT_PROBE() : ParserKeyword("SEGMENT_PROBE", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SMDEN");
  addDeckName("SDENM");
  addDeckName("SGDEN");
  addDeckName("SGOR");
  addDeckName("SGFR");
  addDeckName("SGFV");
  addDeckName("SGFRF");
  addDeckName("SGFRS");
  addDeckName("SGFT");
  addDeckName("SGHF");
  addDeckName("SGVIS");
  addDeckName("SODEN");
  addDeckName("SWGR");
  addDeckName("SOFR");
  addDeckName("SOFRF");
  addDeckName("SWVIS");
  addDeckName("SOFRS");
  addDeckName("SPRDH");
  addDeckName("SWCT");
  addDeckName("SOFT");
  addDeckName("SOFV");
  addDeckName("SOGR");
  addDeckName("SOHF");
  addDeckName("SOVIS");
  addDeckName("SPR");
  addDeckName("SPRD");
  addDeckName("SPRDA");
  addDeckName("SPRDF");
  addDeckName("STFC");
  addDeckName("STFR");
  addDeckName("SWDEN");
  addDeckName("SWFR");
  addDeckName("SWFT");
  addDeckName("SWFV");
  addDeckName("SWHF");
  setMatchRegex("SU.+|STFC.+|STFR.+");
  {
     ParserRecord record;
     {
        ParserItem item("Well", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("Segment", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SEGMENT_PROBE::keywordName = "SEGMENT_PROBE";
const std::string SEGMENT_PROBE::Well::itemName = "Well";
const std::string SEGMENT_PROBE::Segment::itemName = "Segment";


SEPARATE::SEPARATE() : ParserKeyword("SEPARATE", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("SEPARATE");
}
const std::string SEPARATE::keywordName = "SEPARATE";


SEPVALS::SEPVALS() : ParserKeyword("SEPVALS", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SEPVALS");
  {
     ParserRecord record;
     {
        ParserItem item("SEPARATOR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("BO", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("RS", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SEPVALS::keywordName = "SEPVALS";
const std::string SEPVALS::SEPARATOR::itemName = "SEPARATOR";
const std::string SEPVALS::BO::itemName = "BO";
const std::string SEPVALS::RS::itemName = "RS";


SFOAM::SFOAM() : ParserKeyword("SFOAM", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SFOAM");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/Length*Length*Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SFOAM::keywordName = "SFOAM";
const std::string SFOAM::data::itemName = "data";


SGAS::SGAS() : ParserKeyword("SGAS", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SGAS");
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
const std::string SGAS::keywordName = "SGAS";
const std::string SGAS::data::itemName = "data";


SGCR::SGCR() : ParserKeyword("SGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SGCR");
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
const std::string SGCR::keywordName = "SGCR";
const std::string SGCR::data::itemName = "data";


SGCWMIS::SGCWMIS() : ParserKeyword("SGCWMIS", KeywordSize("MISCIBLE", "NTMISC", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGCWMIS");
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
const std::string SGCWMIS::keywordName = "SGCWMIS";
const std::string SGCWMIS::DATA::itemName = "DATA";


SGF32D::SGF32D() : ParserKeyword("SGF32D", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGF32D");
  {
     ParserRecord record;
     {
        ParserItem item("SOIL", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("SWAT", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRG", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGF32D::keywordName = "SGF32D";
const std::string SGF32D::SOIL::itemName = "SOIL";
const std::string SGF32D::SWAT::itemName = "SWAT";
const std::string SGF32D::KRG::itemName = "KRG";


SGFN::SGFN() : ParserKeyword("SGFN", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "GSF",
  });
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SGFN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGFN::keywordName = "SGFN";
const std::string SGFN::DATA::itemName = "DATA";


SGL::SGL() : ParserKeyword("SGL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SGL");
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
const std::string SGL::keywordName = "SGL";
const std::string SGL::data::itemName = "data";


SGLPC::SGLPC() : ParserKeyword("SGLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SGLPC");
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
const std::string SGLPC::keywordName = "SGLPC";
const std::string SGLPC::data::itemName = "data";


SGOF::SGOF() : ParserKeyword("SGOF", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "SLGOF",
    "GSF",
  });
  setRequiredKeywords({
    "GAS",
    "OIL",
  });
  clearDeckNames();
  addDeckName("SGOF");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGOF::keywordName = "SGOF";
const std::string SGOF::DATA::itemName = "DATA";


SGOFLET::SGOFLET() : ParserKeyword("SGOFLET", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
    "OIL",
  });
  clearDeckNames();
  addDeckName("SGOFLET");
  {
     ParserRecord record;
     {
        ParserItem item("SG_0", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SG_CRITICAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRT_GAS", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SO_0", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SO_CRITICAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRT_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PCIR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PCT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGOFLET::keywordName = "SGOFLET";
const std::string SGOFLET::SG_0::itemName = "SG_0";
const std::string SGOFLET::SG_CRITICAL::itemName = "SG_CRITICAL";
const std::string SGOFLET::L_GAS::itemName = "L_GAS";
const std::string SGOFLET::E_GAS::itemName = "E_GAS";
const std::string SGOFLET::T_GAS::itemName = "T_GAS";
const std::string SGOFLET::KRT_GAS::itemName = "KRT_GAS";
const std::string SGOFLET::SO_0::itemName = "SO_0";
const std::string SGOFLET::SO_CRITICAL::itemName = "SO_CRITICAL";
const std::string SGOFLET::L_OIL::itemName = "L_OIL";
const std::string SGOFLET::E_OIL::itemName = "E_OIL";
const std::string SGOFLET::T_OIL::itemName = "T_OIL";
const std::string SGOFLET::KRT_OIL::itemName = "KRT_OIL";
const std::string SGOFLET::L_PC::itemName = "L_PC";
const std::string SGOFLET::E_PC::itemName = "E_PC";
const std::string SGOFLET::T_PC::itemName = "T_PC";
const std::string SGOFLET::PCIR::itemName = "PCIR";
const std::string SGOFLET::PCT::itemName = "PCT";


SGU::SGU() : ParserKeyword("SGU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SGU");
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
const std::string SGU::keywordName = "SGU";
const std::string SGU::data::itemName = "data";


SGWFN::SGWFN() : ParserKeyword("SGWFN", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SGWFN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SGWFN::keywordName = "SGWFN";
const std::string SGWFN::DATA::itemName = "DATA";


SHRATE::SHRATE() : ParserKeyword("SHRATE", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SHRATE");
  {
     ParserRecord record;
     {
        ParserItem item("SHEAR_RATE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SHRATE::keywordName = "SHRATE";
const std::string SHRATE::SHEAR_RATE::itemName = "SHEAR_RATE";


SIGMA::SIGMA() : ParserKeyword("SIGMA", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SIGMA");
  {
     ParserRecord record;
     {
        ParserItem item("COUPLING", ParserItem::itype::DOUBLE);
        item.push_backDimension("1/Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SIGMA::keywordName = "SIGMA";
const std::string SIGMA::COUPLING::itemName = "COUPLING";


SIGMAGDV::SIGMAGDV() : ParserKeyword("SIGMAGDV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SIGMAGDV");
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
const std::string SIGMAGDV::keywordName = "SIGMAGDV";
const std::string SIGMAGDV::data::itemName = "data";


SIGMATH::SIGMATH() : ParserKeyword("SIGMATH", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SIGMATH");
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
const std::string SIGMATH::keywordName = "SIGMATH";
const std::string SIGMATH::data::itemName = "data";


SIGMAV::SIGMAV() : ParserKeyword("SIGMAV", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SIGMAV");
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
const std::string SIGMAV::keywordName = "SIGMAV";
const std::string SIGMAV::data::itemName = "data";


SIMULATE::SIMULATE() : ParserKeyword("SIMULATE", KeywordSize(0, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SIMULATE");
}
const std::string SIMULATE::keywordName = "SIMULATE";


SKIP::SKIP() : ParserKeyword("SKIP", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SKIP");
}
const std::string SKIP::keywordName = "SKIP";


SKIP100::SKIP100() : ParserKeyword("SKIP100", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SKIP100");
}
const std::string SKIP100::keywordName = "SKIP100";


SKIP300::SKIP300() : ParserKeyword("SKIP300", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SKIP300");
}
const std::string SKIP300::keywordName = "SKIP300";


SKIPREST::SKIPREST() : ParserKeyword("SKIPREST", KeywordSize(0, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SKIPREST");
}
const std::string SKIPREST::keywordName = "SKIPREST";


SKPRPOLY::SKPRPOLY() : ParserKeyword("SKPRPOLY", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKPRPOLY");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("POLYMERCONCENTRATION", ParserItem::itype::DOUBLE);
        item.push_backDimension("PolymerDensity");
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
        ParserItem item("SKINPRESSURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SKPRPOLY::keywordName = "SKPRPOLY";
const std::string SKPRPOLY::TABLE_NUMBER::itemName = "TABLE_NUMBER";
const std::string SKPRPOLY::POLYMERCONCENTRATION::itemName = "POLYMERCONCENTRATION";
const std::string SKPRPOLY::THROUGHPUT::itemName = "THROUGHPUT";
const std::string SKPRPOLY::VELOCITY::itemName = "VELOCITY";
const std::string SKPRPOLY::SKINPRESSURE::itemName = "SKINPRESSURE";


SKPRWAT::SKPRWAT() : ParserKeyword("SKPRWAT", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKPRWAT");
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
        ParserItem item("SKINPRESSURE", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SKPRWAT::keywordName = "SKPRWAT";
const std::string SKPRWAT::TABLE_NUMBER::itemName = "TABLE_NUMBER";
const std::string SKPRWAT::THROUGHPUT::itemName = "THROUGHPUT";
const std::string SKPRWAT::VELOCITY::itemName = "VELOCITY";
const std::string SKPRWAT::SKINPRESSURE::itemName = "SKINPRESSURE";


SKRO::SKRO() : ParserKeyword("SKRO", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKRO");
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
const std::string SKRO::keywordName = "SKRO";
const std::string SKRO::data::itemName = "data";


SKRORG::SKRORG() : ParserKeyword("SKRORG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKRORG");
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
const std::string SKRORG::keywordName = "SKRORG";
const std::string SKRORG::data::itemName = "data";


SKRORW::SKRORW() : ParserKeyword("SKRORW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKRORW");
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
const std::string SKRORW::keywordName = "SKRORW";
const std::string SKRORW::data::itemName = "data";


SKRW::SKRW() : ParserKeyword("SKRW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKRW");
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
const std::string SKRW::keywordName = "SKRW";
const std::string SKRW::data::itemName = "data";


SKRWR::SKRWR() : ParserKeyword("SKRWR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SKRWR");
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
const std::string SKRWR::keywordName = "SKRWR";
const std::string SKRWR::data::itemName = "data";


SLAVES::SLAVES() : ParserKeyword("SLAVES", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SLAVES");
  {
     ParserRecord record;
     {
        ParserItem item("SLAVE_RESERVOIR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("SLAVE_ECLBASE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("HOST_NAME", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIRECTORY", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NUM_PE", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SLAVES::keywordName = "SLAVES";
const std::string SLAVES::SLAVE_RESERVOIR::itemName = "SLAVE_RESERVOIR";
const std::string SLAVES::SLAVE_ECLBASE::itemName = "SLAVE_ECLBASE";
const std::string SLAVES::HOST_NAME::itemName = "HOST_NAME";
const std::string SLAVES::DIRECTORY::itemName = "DIRECTORY";
const std::string SLAVES::NUM_PE::itemName = "NUM_PE";


SLGOF::SLGOF() : ParserKeyword("SLGOF", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "SGOF",
    "GSF",
  });
  setRequiredKeywords({
    "GAS",
  });
  clearDeckNames();
  addDeckName("SLGOF");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SLGOF::keywordName = "SLGOF";
const std::string SLGOF::DATA::itemName = "DATA";


SMICR::SMICR() : ParserKeyword("SMICR", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SMICR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SMICR::keywordName = "SMICR";
const std::string SMICR::data::itemName = "data";


SMRYDIMS::SMRYDIMS() : ParserKeyword("SMRYDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SMRYDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("DIMS", ParserItem::itype::INT);
        item.setDefault( 10000 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SMRYDIMS::keywordName = "SMRYDIMS";
const std::string SMRYDIMS::DIMS::itemName = "DIMS";


SMULTX::SMULTX() : ParserKeyword("SMULTX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SMULTX");
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
const std::string SMULTX::keywordName = "SMULTX";
const std::string SMULTX::data::itemName = "data";


SMULTY::SMULTY() : ParserKeyword("SMULTY", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SMULTY");
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
const std::string SMULTY::keywordName = "SMULTY";
const std::string SMULTY::data::itemName = "data";


SMULTZ::SMULTZ() : ParserKeyword("SMULTZ", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SMULTZ");
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
const std::string SMULTZ::keywordName = "SMULTZ";
const std::string SMULTZ::data::itemName = "data";


SOCRS::SOCRS() : ParserKeyword("SOCRS", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOCRS");
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
const std::string SOCRS::keywordName = "SOCRS";
const std::string SOCRS::data::itemName = "data";


SOF2::SOF2() : ParserKeyword("SOF2", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "OIL",
  });
  clearDeckNames();
  addDeckName("SOF2");
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
const std::string SOF2::keywordName = "SOF2";
const std::string SOF2::DATA::itemName = "DATA";


SOF3::SOF3() : ParserKeyword("SOF3", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "OIL",
    "GAS",
    "WATER",
  });
  clearDeckNames();
  addDeckName("SOF3");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOF3::keywordName = "SOF3";
const std::string SOF3::DATA::itemName = "DATA";


SOF32D::SOF32D() : ParserKeyword("SOF32D", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOF32D");
  {
     ParserRecord record;
     {
        ParserItem item("SWAT", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("SGAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRO", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOF32D::keywordName = "SOF32D";
const std::string SOF32D::SWAT::itemName = "SWAT";
const std::string SOF32D::SGAS::itemName = "SGAS";
const std::string SOF32D::KRO::itemName = "KRO";


SOGCR::SOGCR() : ParserKeyword("SOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "GAS",
    "OIL",
    "ENDSCALE",
  });
  clearDeckNames();
  addDeckName("SOGCR");
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
const std::string SOGCR::keywordName = "SOGCR";
const std::string SOGCR::data::itemName = "data";


SOIL::SOIL() : ParserKeyword("SOIL", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SOIL");
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
const std::string SOIL::keywordName = "SOIL";
const std::string SOIL::data::itemName = "data";


SOLID::SOLID() : ParserKeyword("SOLID", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SOLID");
}
const std::string SOLID::keywordName = "SOLID";


SOLUTION::SOLUTION() : ParserKeyword("SOLUTION", KeywordSize(0, false)) {
  clearDeckNames();
  addDeckName("SOLUTION");
}
const std::string SOLUTION::keywordName = "SOLUTION";


SOLVCONC::SOLVCONC() : ParserKeyword("SOLVCONC", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SOLVCONC");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SOLVCONC::keywordName = "SOLVCONC";
const std::string SOLVCONC::data::itemName = "data";


SOLVDIMS::SOLVDIMS() : ParserKeyword("SOLVDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SOLVDIMS");
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
const std::string SOLVDIMS::keywordName = "SOLVDIMS";
const std::string SOLVDIMS::data::itemName = "data";


SOLVDIRS::SOLVDIRS() : ParserKeyword("SOLVDIRS", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SOLVDIRS");
  {
     ParserRecord record;
     {
        ParserItem item("DIRECTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOLVDIRS::keywordName = "SOLVDIRS";
const std::string SOLVDIRS::DIRECTION::itemName = "DIRECTION";


SOLVENT::SOLVENT() : ParserKeyword("SOLVENT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SOLVENT");
}
const std::string SOLVENT::keywordName = "SOLVENT";


SOLVFRAC::SOLVFRAC() : ParserKeyword("SOLVFRAC", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SOLVFRAC");
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
const std::string SOLVFRAC::keywordName = "SOLVFRAC";
const std::string SOLVFRAC::data::itemName = "data";


SOLVNUM::SOLVNUM() : ParserKeyword("SOLVNUM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SOLVNUM");
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
const std::string SOLVNUM::keywordName = "SOLVNUM";
const std::string SOLVNUM::data::itemName = "data";


SOMGAS::SOMGAS() : ParserKeyword("SOMGAS", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOMGAS");
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
const std::string SOMGAS::keywordName = "SOMGAS";
const std::string SOMGAS::DATA::itemName = "DATA";


SOMWAT::SOMWAT() : ParserKeyword("SOMWAT", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SOMWAT");
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
const std::string SOMWAT::keywordName = "SOMWAT";
const std::string SOMWAT::DATA::itemName = "DATA";


SORWMIS::SORWMIS() : ParserKeyword("SORWMIS", KeywordSize("MISCIBLE", "NTMISC", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SORWMIS");
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
const std::string SORWMIS::keywordName = "SORWMIS";
const std::string SORWMIS::DATA::itemName = "DATA";


SOURCE::SOURCE() : ParserKeyword("SOURCE", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SOURCE");
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
     {
        ParserItem item("COMPONENT", ParserItem::itype::STRING);
        item.setDefault( std::string("NONE") );
        record.addItem(item);
     }
     {
        ParserItem item("RATE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Mass/Time");
        record.addItem(item);
     }
     {
        ParserItem item("HRATE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Energy/Time");
        record.addItem(item);
     }
     {
        ParserItem item("TEMP", ParserItem::itype::DOUBLE);
        item.push_backDimension("Temperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SOURCE::keywordName = "SOURCE";
const std::string SOURCE::I::itemName = "I";
const std::string SOURCE::J::itemName = "J";
const std::string SOURCE::K::itemName = "K";
const std::string SOURCE::COMPONENT::itemName = "COMPONENT";
const std::string SOURCE::COMPONENT::defaultValue = "NONE";
const std::string SOURCE::RATE::itemName = "RATE";
const std::string SOURCE::HRATE::itemName = "HRATE";
const std::string SOURCE::TEMP::itemName = "TEMP";


SOWCR::SOWCR() : ParserKeyword("SOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "OIL",
    "WATER",
    "ENDSCALE",
  });
  clearDeckNames();
  addDeckName("SOWCR");
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
const std::string SOWCR::keywordName = "SOWCR";
const std::string SOWCR::data::itemName = "data";


SOXYG::SOXYG() : ParserKeyword("SOXYG", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SOXYG");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SOXYG::keywordName = "SOXYG";
const std::string SOXYG::data::itemName = "data";


SPECGRID::SPECGRID() : ParserKeyword("SPECGRID", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("SPECGRID");
  {
     ParserRecord record;
     {
        ParserItem item("NX", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NY", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NZ", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NUMRES", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("COORD_TYPE", ParserItem::itype::STRING);
        item.setDefault( std::string("F") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECGRID::keywordName = "SPECGRID";
const std::string SPECGRID::NX::itemName = "NX";
const std::string SPECGRID::NY::itemName = "NY";
const std::string SPECGRID::NZ::itemName = "NZ";
const std::string SPECGRID::NUMRES::itemName = "NUMRES";
const std::string SPECGRID::COORD_TYPE::itemName = "COORD_TYPE";
const std::string SPECGRID::COORD_TYPE::defaultValue = "F";


SPECHEAT::SPECHEAT() : ParserKeyword("SPECHEAT", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SPECHEAT");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        item.push_backDimension("Energy/Mass*AbsoluteTemperature");
        item.push_backDimension("Energy/Mass*AbsoluteTemperature");
        item.push_backDimension("Energy/Mass*AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECHEAT::keywordName = "SPECHEAT";
const std::string SPECHEAT::DATA::itemName = "DATA";


SPECROCK::SPECROCK() : ParserKeyword("SPECROCK", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SPECROCK");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Temperature");
        item.push_backDimension("Energy/Length*Length*Length*AbsoluteTemperature");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SPECROCK::keywordName = "SPECROCK";
const std::string SPECROCK::DATA::itemName = "DATA";


SPIDER::SPIDER() : ParserKeyword("SPIDER", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SPIDER");
}
const std::string SPIDER::keywordName = "SPIDER";


SPOLY::SPOLY() : ParserKeyword("SPOLY", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  addValidSectionName("SPECIAL");
  clearDeckNames();
  addDeckName("SPOLY");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SPOLY::keywordName = "SPOLY";
const std::string SPOLY::data::itemName = "data";


SPOLYMW::SPOLYMW() : ParserKeyword("SPOLYMW", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SPOLYMW");
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
const std::string SPOLYMW::keywordName = "SPOLYMW";
const std::string SPOLYMW::data::itemName = "data";


SSFN::SSFN() : ParserKeyword("SSFN", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSFN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SSFN::keywordName = "SSFN";
const std::string SSFN::DATA::itemName = "DATA";


SSGCR::SSGCR() : ParserKeyword("SSGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSGCR");
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
const std::string SSGCR::keywordName = "SSGCR";
const std::string SSGCR::data::itemName = "data";


SSGL::SSGL() : ParserKeyword("SSGL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSGL");
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
const std::string SSGL::keywordName = "SSGL";
const std::string SSGL::data::itemName = "data";


SSOGCR::SSOGCR() : ParserKeyword("SSOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSOGCR");
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
const std::string SSOGCR::keywordName = "SSOGCR";
const std::string SSOGCR::data::itemName = "data";


SSOL::SSOL() : ParserKeyword("SSOL", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SSOL");
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
const std::string SSOL::keywordName = "SSOL";
const std::string SSOL::data::itemName = "data";


SSOWCR::SSOWCR() : ParserKeyword("SSOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSOWCR");
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
const std::string SSOWCR::keywordName = "SSOWCR";
const std::string SSOWCR::data::itemName = "data";


SSWCR::SSWCR() : ParserKeyword("SSWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSWCR");
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
const std::string SSWCR::keywordName = "SSWCR";
const std::string SSWCR::data::itemName = "data";


SSWL::SSWL() : ParserKeyword("SSWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSWL");
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
const std::string SSWL::keywordName = "SSWL";
const std::string SSWL::data::itemName = "data";


SSWU::SSWU() : ParserKeyword("SSWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SSWU");
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
const std::string SSWU::keywordName = "SSWU";
const std::string SSWU::data::itemName = "data";


START::START() : ParserKeyword("START", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("START");
  {
     ParserRecord record;
     {
        ParserItem item("DAY", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("MONTH", ParserItem::itype::STRING);
        item.setDefault( std::string("JAN") );
        record.addItem(item);
     }
     {
        ParserItem item("YEAR", ParserItem::itype::INT);
        item.setDefault( 1983 );
        record.addItem(item);
     }
     {
        ParserItem item("TIME", ParserItem::itype::STRING);
        item.setDefault( std::string("00:00:00.000") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string START::keywordName = "START";
const std::string START::DAY::itemName = "DAY";
const std::string START::MONTH::itemName = "MONTH";
const std::string START::MONTH::defaultValue = "JAN";
const std::string START::YEAR::itemName = "YEAR";
const std::string START::TIME::itemName = "TIME";
const std::string START::TIME::defaultValue = "00:00:00.000";


STCOND::STCOND() : ParserKeyword("STCOND", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STCOND");
  {
     ParserRecord record;
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
     addRecord( record );
  }
}
const std::string STCOND::keywordName = "STCOND";
const std::string STCOND::TEMPERATURE::itemName = "TEMPERATURE";
const std::string STCOND::PRESSURE::itemName = "PRESSURE";


STOG::STOG() : ParserKeyword("STOG", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STOG");
  setAlternatingKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("REF_OIL_PHASE_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
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
        item.push_backDimension("SurfaceTension");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string STOG::keywordName = "STOG";
const std::string STOG::REF_OIL_PHASE_PRESSURE::itemName = "REF_OIL_PHASE_PRESSURE";
const std::string STOG::table::itemName = "table";


STONE::STONE() : ParserKeyword("STONE", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE");
}
const std::string STONE::keywordName = "STONE";


STONE1::STONE1() : ParserKeyword("STONE1", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE1");
}
const std::string STONE1::keywordName = "STONE1";


STONE1EX::STONE1EX() : ParserKeyword("STONE1EX", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE1EX");
  {
     ParserRecord record;
     {
        ParserItem item("EXP_VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string STONE1EX::keywordName = "STONE1EX";
const std::string STONE1EX::EXP_VALUE::itemName = "EXP_VALUE";


STONE2::STONE2() : ParserKeyword("STONE2", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STONE2");
}
const std::string STONE2::keywordName = "STONE2";


STOW::STOW() : ParserKeyword("STOW", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STOW");
  setAlternatingKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("REF_OIL_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
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
        item.push_backDimension("SurfaceTension");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string STOW::keywordName = "STOW";
const std::string STOW::REF_OIL_PRESSURE::itemName = "REF_OIL_PRESSURE";
const std::string STOW::table::itemName = "table";


STREQUIL::STREQUIL() : ParserKeyword("STREQUIL", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("STREQUIL");
  {
     ParserRecord record;
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_POSX", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_POSY", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSXX", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSXXGRAD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure/Length");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSYY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSYYGRAD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure/Length");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSZZ", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("STRESSZZGRAD", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure/Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string STREQUIL::keywordName = "STREQUIL";
const std::string STREQUIL::DATUM_DEPTH::itemName = "DATUM_DEPTH";
const std::string STREQUIL::DATUM_POSX::itemName = "DATUM_POSX";
const std::string STREQUIL::DATUM_POSY::itemName = "DATUM_POSY";
const std::string STREQUIL::STRESSXX::itemName = "STRESSXX";
const std::string STREQUIL::STRESSXXGRAD::itemName = "STRESSXXGRAD";
const std::string STREQUIL::STRESSYY::itemName = "STRESSYY";
const std::string STREQUIL::STRESSYYGRAD::itemName = "STRESSYYGRAD";
const std::string STREQUIL::STRESSZZ::itemName = "STRESSZZ";
const std::string STREQUIL::STRESSZZGRAD::itemName = "STRESSZZGRAD";


STRESSEQUILNUM::STRESSEQUILNUM() : ParserKeyword("STRESSEQUILNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("STRESSEQUILNUM");
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
const std::string STRESSEQUILNUM::keywordName = "STRESSEQUILNUM";
const std::string STRESSEQUILNUM::data::itemName = "data";


STWG::STWG() : ParserKeyword("STWG", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("STWG");
  setAlternatingKeyword(true);
  {
     ParserRecord record;
     {
        ParserItem item("REF_OIL_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
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
        item.push_backDimension("SurfaceTension");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string STWG::keywordName = "STWG";
const std::string STWG::REF_OIL_PRESSURE::itemName = "REF_OIL_PRESSURE";
const std::string STWG::table::itemName = "table";


SUMMARY::SUMMARY() : ParserKeyword("SUMMARY", KeywordSize(0, false)) {
  clearDeckNames();
  addDeckName("SUMMARY");
}
const std::string SUMMARY::keywordName = "SUMMARY";


SUMTHIN::SUMTHIN() : ParserKeyword("SUMTHIN", KeywordSize(1, false)) {
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SUMTHIN");
  {
     ParserRecord record;
     {
        ParserItem item("TIME", ParserItem::itype::DOUBLE);
        item.push_backDimension("Time");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SUMTHIN::keywordName = "SUMTHIN";
const std::string SUMTHIN::TIME::itemName = "TIME";


SUREA::SUREA() : ParserKeyword("SUREA", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SUREA");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string SUREA::keywordName = "SUREA";
const std::string SUREA::data::itemName = "data";


SURF::SURF() : ParserKeyword("SURF", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SURF");
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
const std::string SURF::keywordName = "SURF";
const std::string SURF::data::itemName = "data";


SURFACT::SURFACT() : ParserKeyword("SURFACT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SURFACT");
}
const std::string SURFACT::keywordName = "SURFACT";


SURFACTW::SURFACTW() : ParserKeyword("SURFACTW", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("SURFACTW");
}
const std::string SURFACTW::keywordName = "SURFACTW";


SURFADDW::SURFADDW() : ParserKeyword("SURFADDW", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFADDW");
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
const std::string SURFADDW::keywordName = "SURFADDW";
const std::string SURFADDW::DATA::itemName = "DATA";


SURFADS::SURFADS() : ParserKeyword("SURFADS", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFADS");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/Length*Length*Length");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SURFADS::keywordName = "SURFADS";
const std::string SURFADS::DATA::itemName = "DATA";


SURFCAPD::SURFCAPD() : ParserKeyword("SURFCAPD", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFCAPD");
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
const std::string SURFCAPD::keywordName = "SURFCAPD";
const std::string SURFCAPD::DATA::itemName = "DATA";


SURFESAL::SURFESAL() : ParserKeyword("SURFESAL", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFESAL");
  {
     ParserRecord record;
     {
        ParserItem item("COEFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SURFESAL::keywordName = "SURFESAL";
const std::string SURFESAL::COEFF::itemName = "COEFF";


SURFNUM::SURFNUM() : ParserKeyword("SURFNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("SURFNUM");
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
const std::string SURFNUM::keywordName = "SURFNUM";
const std::string SURFNUM::data::itemName = "data";


SURFOPTS::SURFOPTS() : ParserKeyword("SURFOPTS", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("MIN_SWAT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     {
        ParserItem item("SMOOTHING", ParserItem::itype::DOUBLE);
        item.setDefault( double(1e-06) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SURFOPTS::keywordName = "SURFOPTS";
const std::string SURFOPTS::MIN_SWAT::itemName = "MIN_SWAT";
const std::string SURFOPTS::SMOOTHING::itemName = "SMOOTHING";


SURFROCK::SURFROCK() : ParserKeyword("SURFROCK", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFROCK");
  {
     ParserRecord record;
     {
        ParserItem item("INDEX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("MASS_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Mass/ReservoirVolume");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SURFROCK::keywordName = "SURFROCK";
const std::string SURFROCK::INDEX::itemName = "INDEX";
const std::string SURFROCK::MASS_DENSITY::itemName = "MASS_DENSITY";


SURFST::SURFST() : ParserKeyword("SURFST", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFST");
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
const std::string SURFST::keywordName = "SURFST";
const std::string SURFST::DATA::itemName = "DATA";


SURFSTES::SURFSTES() : ParserKeyword("SURFSTES", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFSTES");
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
const std::string SURFSTES::keywordName = "SURFSTES";
const std::string SURFSTES::DATA::itemName = "DATA";


SURFVISC::SURFVISC() : ParserKeyword("SURFVISC", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SURFVISC");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Mass/ReservoirVolume");
        item.push_backDimension("Viscosity");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SURFVISC::keywordName = "SURFVISC";
const std::string SURFVISC::DATA::itemName = "DATA";


SURFWNUM::SURFWNUM() : ParserKeyword("SURFWNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("SURFWNUM");
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
const std::string SURFWNUM::keywordName = "SURFWNUM";
const std::string SURFWNUM::data::itemName = "data";


SWAT::SWAT() : ParserKeyword("SWAT", KeywordSize(1, false)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("SWAT");
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
const std::string SWAT::keywordName = "SWAT";
const std::string SWAT::data::itemName = "data";


SWATINIT::SWATINIT() : ParserKeyword("SWATINIT", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWATINIT");
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
const std::string SWATINIT::keywordName = "SWATINIT";
const std::string SWATINIT::data::itemName = "data";


SWCR::SWCR() : ParserKeyword("SWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWCR");
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
const std::string SWCR::keywordName = "SWCR";
const std::string SWCR::data::itemName = "data";


SWF32D::SWF32D() : ParserKeyword("SWF32D", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWF32D");
  {
     ParserRecord record;
     {
        ParserItem item("SOIL", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
  {
     ParserRecord record;
     {
        ParserItem item("SGAS", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRW", ParserItem::itype::DOUBLE);
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWF32D::keywordName = "SWF32D";
const std::string SWF32D::SOIL::itemName = "SOIL";
const std::string SWF32D::SGAS::itemName = "SGAS";
const std::string SWF32D::KRW::itemName = "KRW";


SWFN::SWFN() : ParserKeyword("SWFN", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "WSF",
  });
  setRequiredKeywords({
    "WATER",
  });
  clearDeckNames();
  addDeckName("SWFN");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWFN::keywordName = "SWFN";
const std::string SWFN::DATA::itemName = "DATA";


SWINGFAC::SWINGFAC() : ParserKeyword("SWINGFAC", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("SWINGFAC");
  {
     ParserRecord record;
     {
        ParserItem item("SWING_FACTOR1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR3", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR4", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR5", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR6", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR7", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR8", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR9", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR10", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR11", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SWING_FACTOR12", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR1", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR2", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR3", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR4", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR5", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR6", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR7", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR8", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR9", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR10", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR11", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("PROFILE_FACTOR12", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWINGFAC::keywordName = "SWINGFAC";
const std::string SWINGFAC::SWING_FACTOR1::itemName = "SWING_FACTOR1";
const std::string SWINGFAC::SWING_FACTOR2::itemName = "SWING_FACTOR2";
const std::string SWINGFAC::SWING_FACTOR3::itemName = "SWING_FACTOR3";
const std::string SWINGFAC::SWING_FACTOR4::itemName = "SWING_FACTOR4";
const std::string SWINGFAC::SWING_FACTOR5::itemName = "SWING_FACTOR5";
const std::string SWINGFAC::SWING_FACTOR6::itemName = "SWING_FACTOR6";
const std::string SWINGFAC::SWING_FACTOR7::itemName = "SWING_FACTOR7";
const std::string SWINGFAC::SWING_FACTOR8::itemName = "SWING_FACTOR8";
const std::string SWINGFAC::SWING_FACTOR9::itemName = "SWING_FACTOR9";
const std::string SWINGFAC::SWING_FACTOR10::itemName = "SWING_FACTOR10";
const std::string SWINGFAC::SWING_FACTOR11::itemName = "SWING_FACTOR11";
const std::string SWINGFAC::SWING_FACTOR12::itemName = "SWING_FACTOR12";
const std::string SWINGFAC::PROFILE_FACTOR1::itemName = "PROFILE_FACTOR1";
const std::string SWINGFAC::PROFILE_FACTOR2::itemName = "PROFILE_FACTOR2";
const std::string SWINGFAC::PROFILE_FACTOR3::itemName = "PROFILE_FACTOR3";
const std::string SWINGFAC::PROFILE_FACTOR4::itemName = "PROFILE_FACTOR4";
const std::string SWINGFAC::PROFILE_FACTOR5::itemName = "PROFILE_FACTOR5";
const std::string SWINGFAC::PROFILE_FACTOR6::itemName = "PROFILE_FACTOR6";
const std::string SWINGFAC::PROFILE_FACTOR7::itemName = "PROFILE_FACTOR7";
const std::string SWINGFAC::PROFILE_FACTOR8::itemName = "PROFILE_FACTOR8";
const std::string SWINGFAC::PROFILE_FACTOR9::itemName = "PROFILE_FACTOR9";
const std::string SWINGFAC::PROFILE_FACTOR10::itemName = "PROFILE_FACTOR10";
const std::string SWINGFAC::PROFILE_FACTOR11::itemName = "PROFILE_FACTOR11";
const std::string SWINGFAC::PROFILE_FACTOR12::itemName = "PROFILE_FACTOR12";


SWL::SWL() : ParserKeyword("SWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWL");
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
const std::string SWL::keywordName = "SWL";
const std::string SWL::data::itemName = "data";


SWLPC::SWLPC() : ParserKeyword("SWLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWLPC");
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
const std::string SWLPC::keywordName = "SWLPC";
const std::string SWLPC::data::itemName = "data";


SWOF::SWOF() : ParserKeyword("SWOF", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setProhibitedKeywords({
    "WSF",
  });
  setRequiredKeywords({
    "OIL",
    "WATER",
  });
  clearDeckNames();
  addDeckName("SWOF");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWOF::keywordName = "SWOF";
const std::string SWOF::DATA::itemName = "DATA";


SWOFLET::SWOFLET() : ParserKeyword("SWOFLET", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "OIL",
    "WATER",
  });
  clearDeckNames();
  addDeckName("SWOFLET");
  {
     ParserRecord record;
     {
        ParserItem item("SW_RESIDUAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SW_CRITICAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_WATER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_WATER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_WATER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRT_WATER", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SO_RESIDUAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SO_CRITICAL", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("KRT_OIL", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("L_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("E_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("T_PC", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("PCIR", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("PCT", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string SWOFLET::keywordName = "SWOFLET";
const std::string SWOFLET::SW_RESIDUAL::itemName = "SW_RESIDUAL";
const std::string SWOFLET::SW_CRITICAL::itemName = "SW_CRITICAL";
const std::string SWOFLET::L_WATER::itemName = "L_WATER";
const std::string SWOFLET::E_WATER::itemName = "E_WATER";
const std::string SWOFLET::T_WATER::itemName = "T_WATER";
const std::string SWOFLET::KRT_WATER::itemName = "KRT_WATER";
const std::string SWOFLET::SO_RESIDUAL::itemName = "SO_RESIDUAL";
const std::string SWOFLET::SO_CRITICAL::itemName = "SO_CRITICAL";
const std::string SWOFLET::L_OIL::itemName = "L_OIL";
const std::string SWOFLET::E_OIL::itemName = "E_OIL";
const std::string SWOFLET::T_OIL::itemName = "T_OIL";
const std::string SWOFLET::KRT_OIL::itemName = "KRT_OIL";
const std::string SWOFLET::L_PC::itemName = "L_PC";
const std::string SWOFLET::E_PC::itemName = "E_PC";
const std::string SWOFLET::T_PC::itemName = "T_PC";
const std::string SWOFLET::PCIR::itemName = "PCIR";
const std::string SWOFLET::PCT::itemName = "PCT";


SWU::SWU() : ParserKeyword("SWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("SWU");
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
const std::string SWU::keywordName = "SWU";
const std::string SWU::data::itemName = "data";


}
}
