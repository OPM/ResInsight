
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/I.hpp>
namespace Opm {
namespace ParserKeywords {
IHOST::IHOST() : ParserKeyword("IHOST", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("IHOST");
  {
     ParserRecord record;
     {
        ParserItem item("LGR", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("PROCESS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IHOST::keywordName = "IHOST";
const std::string IHOST::LGR::itemName = "LGR";
const std::string IHOST::PROCESS::itemName = "PROCESS";


IMBNUM::IMBNUM() : ParserKeyword("IMBNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("IMBNUM");
  addDeckName("IMBNUMZ");
  addDeckName("IMBNUMX");
  addDeckName("IMBNUMY");
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
const std::string IMBNUM::keywordName = "IMBNUM";
const std::string IMBNUM::data::itemName = "data";


IMBNUMMF::IMBNUMMF() : ParserKeyword("IMBNUMMF", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("IMBNUMMF");
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
const std::string IMBNUMMF::keywordName = "IMBNUMMF";
const std::string IMBNUMMF::data::itemName = "data";


IMKRVD::IMKRVD() : ParserKeyword("IMKRVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMKRVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMKRVD::keywordName = "IMKRVD";
const std::string IMKRVD::DATA::itemName = "DATA";


IMPCVD::IMPCVD() : ParserKeyword("IMPCVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMPCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Pressure");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMPCVD::keywordName = "IMPCVD";
const std::string IMPCVD::DATA::itemName = "DATA";


IMPES::IMPES() : ParserKeyword("IMPES", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("IMPES");
}
const std::string IMPES::keywordName = "IMPES";


IMPLICIT::IMPLICIT() : ParserKeyword("IMPLICIT", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("IMPLICIT");
}
const std::string IMPLICIT::keywordName = "IMPLICIT";


IMPORT::IMPORT() : ParserKeyword("IMPORT", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("IMPORT");
  {
     ParserRecord record;
     {
        ParserItem item("FILE", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FORMATTED", ParserItem::itype::STRING);
        item.setDefault( std::string("UNFORMATTED") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMPORT::keywordName = "IMPORT";
const std::string IMPORT::FILE::itemName = "FILE";
const std::string IMPORT::FORMATTED::itemName = "FORMATTED";
const std::string IMPORT::FORMATTED::defaultValue = "UNFORMATTED";


IMPTVD::IMPTVD() : ParserKeyword("IMPTVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMPTVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMPTVD::keywordName = "IMPTVD";
const std::string IMPTVD::DATA::itemName = "DATA";


IMSPCVD::IMSPCVD() : ParserKeyword("IMSPCVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IMSPCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IMSPCVD::keywordName = "IMSPCVD";
const std::string IMSPCVD::DATA::itemName = "DATA";


INCLUDE::INCLUDE() : ParserKeyword("INCLUDE", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("INCLUDE");
  {
     ParserRecord record;
     {
        ParserItem item("IncludeFile", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string INCLUDE::keywordName = "INCLUDE";
const std::string INCLUDE::IncludeFile::itemName = "IncludeFile";


INIT::INIT() : ParserKeyword("INIT", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("INIT");
}
const std::string INIT::keywordName = "INIT";


INRAD::INRAD() : ParserKeyword("INRAD", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("INRAD");
  {
     ParserRecord record;
     {
        ParserItem item("RADIUS", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string INRAD::keywordName = "INRAD";
const std::string INRAD::RADIUS::itemName = "RADIUS";


INSPEC::INSPEC() : ParserKeyword("INSPEC", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("INSPEC");
}
const std::string INSPEC::keywordName = "INSPEC";


INTPC::INTPC() : ParserKeyword("INTPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("INTPC");
  {
     ParserRecord record;
     {
        ParserItem item("PHASE", ParserItem::itype::STRING);
        item.setDefault( std::string("BOTH") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string INTPC::keywordName = "INTPC";
const std::string INTPC::PHASE::itemName = "PHASE";
const std::string INTPC::PHASE::defaultValue = "BOTH";


IONROCK::IONROCK() : ParserKeyword("IONROCK", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("IONROCK");
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
const std::string IONROCK::keywordName = "IONROCK";
const std::string IONROCK::data::itemName = "data";


IONXROCK::IONXROCK() : ParserKeyword("IONXROCK", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IONXROCK");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IONXROCK::keywordName = "IONXROCK";
const std::string IONXROCK::VALUE::itemName = "VALUE";


IONXSURF::IONXSURF() : ParserKeyword("IONXSURF", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("IONXSURF");
  {
     ParserRecord record;
     {
        ParserItem item("MOLECULAR_WEIGHT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("ION_EXCH_CONST", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string IONXSURF::keywordName = "IONXSURF";
const std::string IONXSURF::MOLECULAR_WEIGHT::itemName = "MOLECULAR_WEIGHT";
const std::string IONXSURF::ION_EXCH_CONST::itemName = "ION_EXCH_CONST";


IPCG::IPCG() : ParserKeyword("IPCG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "ENDSCALE",
  });
  clearDeckNames();
  addDeckName("IPCG");
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
const std::string IPCG::keywordName = "IPCG";
const std::string IPCG::data::itemName = "data";


IPCW::IPCW() : ParserKeyword("IPCW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  setRequiredKeywords({
    "ENDSCALE",
  });
  clearDeckNames();
  addDeckName("IPCW");
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
const std::string IPCW::keywordName = "IPCW";
const std::string IPCW::data::itemName = "data";


ISGCR::ISGCR() : ParserKeyword("ISGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGCR");
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
const std::string ISGCR::keywordName = "ISGCR";
const std::string ISGCR::data::itemName = "data";


ISGL::ISGL() : ParserKeyword("ISGL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGL");
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
const std::string ISGL::keywordName = "ISGL";
const std::string ISGL::data::itemName = "data";


ISGLPC::ISGLPC() : ParserKeyword("ISGLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGLPC");
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
const std::string ISGLPC::keywordName = "ISGLPC";
const std::string ISGLPC::data::itemName = "data";


ISGU::ISGU() : ParserKeyword("ISGU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISGU");
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
const std::string ISGU::keywordName = "ISGU";
const std::string ISGU::data::itemName = "data";


ISOGCR::ISOGCR() : ParserKeyword("ISOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISOGCR");
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
const std::string ISOGCR::keywordName = "ISOGCR";
const std::string ISOGCR::data::itemName = "data";


ISOLNUM::ISOLNUM() : ParserKeyword("ISOLNUM", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("ISOLNUM");
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
const std::string ISOLNUM::keywordName = "ISOLNUM";
const std::string ISOLNUM::data::itemName = "data";


ISOWCR::ISOWCR() : ParserKeyword("ISOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISOWCR");
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
const std::string ISOWCR::keywordName = "ISOWCR";
const std::string ISOWCR::data::itemName = "data";


ISWCR::ISWCR() : ParserKeyword("ISWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWCR");
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
const std::string ISWCR::keywordName = "ISWCR";
const std::string ISWCR::data::itemName = "data";


ISWL::ISWL() : ParserKeyword("ISWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWL");
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
const std::string ISWL::keywordName = "ISWL";
const std::string ISWL::data::itemName = "data";


ISWLPC::ISWLPC() : ParserKeyword("ISWLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWLPC");
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
const std::string ISWLPC::keywordName = "ISWLPC";
const std::string ISWLPC::data::itemName = "data";


ISWU::ISWU() : ParserKeyword("ISWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ISWU");
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
const std::string ISWU::keywordName = "ISWU";
const std::string ISWU::data::itemName = "data";


}
}
