
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/L.hpp>
namespace Opm {
namespace ParserKeywords {
LAB::LAB() : ParserKeyword("LAB", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LAB");
}
const std::string LAB::keywordName = "LAB";


LANGMPL::LANGMPL() : ParserKeyword("LANGMPL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LANGMPL");
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
const std::string LANGMPL::keywordName = "LANGMPL";
const std::string LANGMPL::data::itemName = "data";


LANGMUIR::LANGMUIR() : ParserKeyword("LANGMUIR", KeywordSize("REGDIMS", "NTCREG", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LANGMUIR");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LANGMUIR::keywordName = "LANGMUIR";
const std::string LANGMUIR::DATA::itemName = "DATA";


LANGSOLV::LANGSOLV() : ParserKeyword("LANGSOLV", KeywordSize("REGDIMS", "NTCREG", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LANGSOLV");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("GasSurfaceVolume/Length*Length*Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LANGSOLV::keywordName = "LANGSOLV";
const std::string LANGSOLV::DATA::itemName = "DATA";


LCUNIT::LCUNIT() : ParserKeyword("LCUNIT", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LCUNIT");
  {
     ParserRecord record;
     {
        ParserItem item("UNIT", ParserItem::itype::STRING);
        item.setDefault( std::string("UNIT") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LCUNIT::keywordName = "LCUNIT";
const std::string LCUNIT::UNIT::itemName = "UNIT";
const std::string LCUNIT::UNIT::defaultValue = "UNIT";


LGR::LGR() : ParserKeyword("LGR", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LGR");
  {
     ParserRecord record;
     {
        ParserItem item("MAXLGR", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAXCLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MCOARS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAMALG", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MXLALG", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("LSTACK", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("INTERP", ParserItem::itype::STRING);
        item.setDefault( std::string("INTERP") );
        record.addItem(item);
     }
     {
        ParserItem item("NCHCOR", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LGR::keywordName = "LGR";
const std::string LGR::MAXLGR::itemName = "MAXLGR";
const int LGR::MAXLGR::defaultValue = 0;
const std::string LGR::MAXCLS::itemName = "MAXCLS";
const int LGR::MAXCLS::defaultValue = 0;
const std::string LGR::MCOARS::itemName = "MCOARS";
const int LGR::MCOARS::defaultValue = 0;
const std::string LGR::MAMALG::itemName = "MAMALG";
const int LGR::MAMALG::defaultValue = 0;
const std::string LGR::MXLALG::itemName = "MXLALG";
const int LGR::MXLALG::defaultValue = 0;
const std::string LGR::LSTACK::itemName = "LSTACK";
const int LGR::LSTACK::defaultValue = 0;
const std::string LGR::INTERP::itemName = "INTERP";
const std::string LGR::INTERP::defaultValue = "INTERP";
const std::string LGR::NCHCOR::itemName = "NCHCOR";
const int LGR::NCHCOR::defaultValue = 0;


LGRCOPY::LGRCOPY() : ParserKeyword("LGRCOPY", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LGRCOPY");
}
const std::string LGRCOPY::keywordName = "LGRCOPY";


LGRFREE::LGRFREE() : ParserKeyword("LGRFREE", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LGRFREE");
  {
     ParserRecord record;
     {
        ParserItem item("LOCAL_GRID_REFINMENT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LGRFREE::keywordName = "LGRFREE";
const std::string LGRFREE::LOCAL_GRID_REFINMENT::itemName = "LOCAL_GRID_REFINMENT";


LGRLOCK::LGRLOCK() : ParserKeyword("LGRLOCK", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LGRLOCK");
  {
     ParserRecord record;
     {
        ParserItem item("LOCAL_GRID_REFINMENT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LGRLOCK::keywordName = "LGRLOCK";
const std::string LGRLOCK::LOCAL_GRID_REFINMENT::itemName = "LOCAL_GRID_REFINMENT";


LGROFF::LGROFF() : ParserKeyword("LGROFF", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LGROFF");
  {
     ParserRecord record;
     {
        ParserItem item("LOCAL_GRID_REFINMENT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ACTIVE_WELLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LGROFF::keywordName = "LGROFF";
const std::string LGROFF::LOCAL_GRID_REFINMENT::itemName = "LOCAL_GRID_REFINMENT";
const std::string LGROFF::ACTIVE_WELLS::itemName = "ACTIVE_WELLS";
const int LGROFF::ACTIVE_WELLS::defaultValue = 0;


LGRON::LGRON() : ParserKeyword("LGRON", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LGRON");
  {
     ParserRecord record;
     {
        ParserItem item("LOCAL_GRID_REFINMENT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("ACTIVE_WELLS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LGRON::keywordName = "LGRON";
const std::string LGRON::LOCAL_GRID_REFINMENT::itemName = "LOCAL_GRID_REFINMENT";
const std::string LGRON::ACTIVE_WELLS::itemName = "ACTIVE_WELLS";
const int LGRON::ACTIVE_WELLS::defaultValue = 0;


LICENSES::LICENSES() : ParserKeyword("LICENSES", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LICENSES");
  {
     ParserRecord record;
     {
        ParserItem item("FEATURE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LICENSES::keywordName = "LICENSES";
const std::string LICENSES::FEATURE::itemName = "FEATURE";


LIFTOPT::LIFTOPT() : ParserKeyword("LIFTOPT", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LIFTOPT");
  {
     ParserRecord record;
     {
        ParserItem item("INCREMENT_SIZE", ParserItem::itype::DOUBLE);
        item.push_backDimension("GasSurfaceVolume/Time");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_ECONOMIC_GRADIENT", ParserItem::itype::DOUBLE);
        item.push_backDimension("LiquidSurfaceVolume/GasSurfaceVolume");
        record.addItem(item);
     }
     {
        ParserItem item("MIN_INTERVAL_BETWEEN_GAS_LIFT_OPTIMIZATIONS", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Time");
        record.addItem(item);
     }
     {
        ParserItem item("OPTIMISE_ALL_ITERATIONS", ParserItem::itype::STRING);
        item.setDefault( std::string("YES") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LIFTOPT::keywordName = "LIFTOPT";
const std::string LIFTOPT::INCREMENT_SIZE::itemName = "INCREMENT_SIZE";
const std::string LIFTOPT::MIN_ECONOMIC_GRADIENT::itemName = "MIN_ECONOMIC_GRADIENT";
const std::string LIFTOPT::MIN_INTERVAL_BETWEEN_GAS_LIFT_OPTIMIZATIONS::itemName = "MIN_INTERVAL_BETWEEN_GAS_LIFT_OPTIMIZATIONS";
const double LIFTOPT::MIN_INTERVAL_BETWEEN_GAS_LIFT_OPTIMIZATIONS::defaultValue = 0;
const std::string LIFTOPT::OPTIMISE_ALL_ITERATIONS::itemName = "OPTIMISE_ALL_ITERATIONS";
const std::string LIFTOPT::OPTIMISE_ALL_ITERATIONS::defaultValue = "YES";


LINCOM::LINCOM() : ParserKeyword("LINCOM", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("LINCOM");
  {
     ParserRecord record;
     {
        ParserItem item("ALPHA", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("BETA", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     {
        ParserItem item("GAMMA", ParserItem::itype::UDA);
        item.setDefault( UDAValue(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LINCOM::keywordName = "LINCOM";
const std::string LINCOM::ALPHA::itemName = "ALPHA";
const UDAValue LINCOM::ALPHA::defaultValue = UDAValue(0);
const std::string LINCOM::BETA::itemName = "BETA";
const UDAValue LINCOM::BETA::defaultValue = UDAValue(0);
const std::string LINCOM::GAMMA::itemName = "GAMMA";
const UDAValue LINCOM::GAMMA::defaultValue = UDAValue(0);


LINKPERM::LINKPERM() : ParserKeyword("LINKPERM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LINKPERM");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::STRING);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string LINKPERM::keywordName = "LINKPERM";
const std::string LINKPERM::data::itemName = "data";


LIVEOIL::LIVEOIL() : ParserKeyword("LIVEOIL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LIVEOIL");
}
const std::string LIVEOIL::keywordName = "LIVEOIL";


LKRO::LKRO() : ParserKeyword("LKRO", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LKRO");
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
const std::string LKRO::keywordName = "LKRO";
const std::string LKRO::data::itemName = "data";


LKRORG::LKRORG() : ParserKeyword("LKRORG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LKRORG");
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
const std::string LKRORG::keywordName = "LKRORG";
const std::string LKRORG::data::itemName = "data";


LKRORW::LKRORW() : ParserKeyword("LKRORW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LKRORW");
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
const std::string LKRORW::keywordName = "LKRORW";
const std::string LKRORW::data::itemName = "data";


LKRW::LKRW() : ParserKeyword("LKRW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LKRW");
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
const std::string LKRW::keywordName = "LKRW";
const std::string LKRW::data::itemName = "data";


LKRWR::LKRWR() : ParserKeyword("LKRWR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LKRWR");
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
const std::string LKRWR::keywordName = "LKRWR";
const std::string LKRWR::data::itemName = "data";


LOAD::LOAD() : ParserKeyword("LOAD", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LOAD");
  {
     ParserRecord record;
     {
        ParserItem item("FILE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("REPORT_STEP", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NOSIM", ParserItem::itype::STRING);
        item.setDefault( std::string("SIM") );
        record.addItem(item);
     }
     {
        ParserItem item("FORMATTED", ParserItem::itype::STRING);
        item.setDefault( std::string("UNFORMATTED") );
        record.addItem(item);
     }
     {
        ParserItem item("REQUEST_SAVE_OUTPUT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LOAD::keywordName = "LOAD";
const std::string LOAD::FILE::itemName = "FILE";
const std::string LOAD::REPORT_STEP::itemName = "REPORT_STEP";
const std::string LOAD::NOSIM::itemName = "NOSIM";
const std::string LOAD::NOSIM::defaultValue = "SIM";
const std::string LOAD::FORMATTED::itemName = "FORMATTED";
const std::string LOAD::FORMATTED::defaultValue = "UNFORMATTED";
const std::string LOAD::REQUEST_SAVE_OUTPUT::itemName = "REQUEST_SAVE_OUTPUT";
const std::string LOAD::REQUEST_SAVE_OUTPUT::defaultValue = "NO";


LOWSALT::LOWSALT() : ParserKeyword("LOWSALT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("LOWSALT");
}
const std::string LOWSALT::keywordName = "LOWSALT";


LPCW::LPCW() : ParserKeyword("LPCW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LPCW");
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
const std::string LPCW::keywordName = "LPCW";
const std::string LPCW::data::itemName = "data";


LSALTFNC::LSALTFNC() : ParserKeyword("LSALTFNC", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSALTFNC");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Pressure");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LSALTFNC::keywordName = "LSALTFNC";
const std::string LSALTFNC::DATA::itemName = "DATA";


LSLTWNUM::LSLTWNUM() : ParserKeyword("LSLTWNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("LSLTWNUM");
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
const std::string LSLTWNUM::keywordName = "LSLTWNUM";
const std::string LSLTWNUM::data::itemName = "data";


LSNUM::LSNUM() : ParserKeyword("LSNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("LSNUM");
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
const std::string LSNUM::keywordName = "LSNUM";
const std::string LSNUM::data::itemName = "data";


LSOGCR::LSOGCR() : ParserKeyword("LSOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSOGCR");
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
const std::string LSOGCR::keywordName = "LSOGCR";
const std::string LSOGCR::data::itemName = "data";


LSOWCR::LSOWCR() : ParserKeyword("LSOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSOWCR");
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
const std::string LSOWCR::keywordName = "LSOWCR";
const std::string LSOWCR::data::itemName = "data";


LSWCR::LSWCR() : ParserKeyword("LSWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSWCR");
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
const std::string LSWCR::keywordName = "LSWCR";
const std::string LSWCR::data::itemName = "data";


LSWL::LSWL() : ParserKeyword("LSWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSWL");
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
const std::string LSWL::keywordName = "LSWL";
const std::string LSWL::data::itemName = "data";


LSWLPC::LSWLPC() : ParserKeyword("LSWLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSWLPC");
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
const std::string LSWLPC::keywordName = "LSWLPC";
const std::string LSWLPC::data::itemName = "data";


LSWU::LSWU() : ParserKeyword("LSWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LSWU");
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
const std::string LSWU::keywordName = "LSWU";
const std::string LSWU::data::itemName = "data";


LTOSIGMA::LTOSIGMA() : ParserKeyword("LTOSIGMA", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LTOSIGMA");
  {
     ParserRecord record;
     {
        ParserItem item("FX", ParserItem::itype::DOUBLE);
        item.setDefault( double(4.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("FY", ParserItem::itype::DOUBLE);
        item.setDefault( double(4.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("FZ", ParserItem::itype::DOUBLE);
        item.setDefault( double(4.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("FGD", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("OPTION", ParserItem::itype::STRING);
        item.setDefault( std::string("XONLY") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LTOSIGMA::keywordName = "LTOSIGMA";
const std::string LTOSIGMA::FX::itemName = "FX";
const double LTOSIGMA::FX::defaultValue = 4.000000;
const std::string LTOSIGMA::FY::itemName = "FY";
const double LTOSIGMA::FY::defaultValue = 4.000000;
const std::string LTOSIGMA::FZ::itemName = "FZ";
const double LTOSIGMA::FZ::defaultValue = 4.000000;
const std::string LTOSIGMA::FGD::itemName = "FGD";
const double LTOSIGMA::FGD::defaultValue = 0;
const std::string LTOSIGMA::OPTION::itemName = "OPTION";
const std::string LTOSIGMA::OPTION::defaultValue = "XONLY";


LWKRO::LWKRO() : ParserKeyword("LWKRO", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWKRO");
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
const std::string LWKRO::keywordName = "LWKRO";
const std::string LWKRO::data::itemName = "data";


LWKRORG::LWKRORG() : ParserKeyword("LWKRORG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWKRORG");
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
const std::string LWKRORG::keywordName = "LWKRORG";
const std::string LWKRORG::data::itemName = "data";


LWKRORW::LWKRORW() : ParserKeyword("LWKRORW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWKRORW");
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
const std::string LWKRORW::keywordName = "LWKRORW";
const std::string LWKRORW::data::itemName = "data";


LWKRW::LWKRW() : ParserKeyword("LWKRW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWKRW");
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
const std::string LWKRW::keywordName = "LWKRW";
const std::string LWKRW::data::itemName = "data";


LWKRWR::LWKRWR() : ParserKeyword("LWKRWR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWKRWR");
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
const std::string LWKRWR::keywordName = "LWKRWR";
const std::string LWKRWR::data::itemName = "data";


LWPCW::LWPCW() : ParserKeyword("LWPCW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWPCW");
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
const std::string LWPCW::keywordName = "LWPCW";
const std::string LWPCW::data::itemName = "data";


LWSLTNUM::LWSLTNUM() : ParserKeyword("LWSLTNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("LWSLTNUM");
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
const std::string LWSLTNUM::keywordName = "LWSLTNUM";
const std::string LWSLTNUM::data::itemName = "data";


LWSNUM::LWSNUM() : ParserKeyword("LWSNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("LWSNUM");
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
const std::string LWSNUM::keywordName = "LWSNUM";
const std::string LWSNUM::data::itemName = "data";


LWSOGCR::LWSOGCR() : ParserKeyword("LWSOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSOGCR");
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
const std::string LWSOGCR::keywordName = "LWSOGCR";
const std::string LWSOGCR::data::itemName = "data";


LWSOWCR::LWSOWCR() : ParserKeyword("LWSOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSOWCR");
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
const std::string LWSOWCR::keywordName = "LWSOWCR";
const std::string LWSOWCR::data::itemName = "data";


LWSWCR::LWSWCR() : ParserKeyword("LWSWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSWCR");
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
const std::string LWSWCR::keywordName = "LWSWCR";
const std::string LWSWCR::data::itemName = "data";


LWSWL::LWSWL() : ParserKeyword("LWSWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSWL");
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
const std::string LWSWL::keywordName = "LWSWL";
const std::string LWSWL::data::itemName = "data";


LWSWLPC::LWSWLPC() : ParserKeyword("LWSWLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSWLPC");
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
const std::string LWSWLPC::keywordName = "LWSWLPC";
const std::string LWSWLPC::data::itemName = "data";


LWSWU::LWSWU() : ParserKeyword("LWSWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("LWSWU");
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
const std::string LWSWU::keywordName = "LWSWU";
const std::string LWSWU::data::itemName = "data";


LX::LX() : ParserKeyword("LX", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LX");
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
const std::string LX::keywordName = "LX";
const std::string LX::data::itemName = "data";


LXFIN::LXFIN() : ParserKeyword("LXFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LXFIN");
  {
     ParserRecord record;
     {
        ParserItem item("CELL_THICKNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SIZE_OPTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LXFIN::keywordName = "LXFIN";
const std::string LXFIN::CELL_THICKNESS::itemName = "CELL_THICKNESS";
const std::string LXFIN::SIZE_OPTION::itemName = "SIZE_OPTION";


LY::LY() : ParserKeyword("LY", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LY");
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
const std::string LY::keywordName = "LY";
const std::string LY::data::itemName = "data";


LYFIN::LYFIN() : ParserKeyword("LYFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LYFIN");
  {
     ParserRecord record;
     {
        ParserItem item("CELL_THICKNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SIZE_OPTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LYFIN::keywordName = "LYFIN";
const std::string LYFIN::CELL_THICKNESS::itemName = "CELL_THICKNESS";
const std::string LYFIN::SIZE_OPTION::itemName = "SIZE_OPTION";


LZ::LZ() : ParserKeyword("LZ", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LZ");
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
const std::string LZ::keywordName = "LZ";
const std::string LZ::data::itemName = "data";


LZFIN::LZFIN() : ParserKeyword("LZFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("LZFIN");
  {
     ParserRecord record;
     {
        ParserItem item("CELL_THICKNESS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("SIZE_OPTION", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string LZFIN::keywordName = "LZFIN";
const std::string LZFIN::CELL_THICKNESS::itemName = "CELL_THICKNESS";
const std::string LZFIN::SIZE_OPTION::itemName = "SIZE_OPTION";


}
}
