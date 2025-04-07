
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/H.hpp>
namespace Opm {
namespace ParserKeywords {
H2SOL::H2SOL() : ParserKeyword("H2SOL", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("H2SOL");
}
const std::string H2SOL::keywordName = "H2SOL";


H2STORE::H2STORE() : ParserKeyword("H2STORE", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("H2STORE");
}
const std::string H2STORE::keywordName = "H2STORE";


HALFTRAN::HALFTRAN() : ParserKeyword("HALFTRAN", KeywordSize(0, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HALFTRAN");
}
const std::string HALFTRAN::keywordName = "HALFTRAN";


HAxxxxxx::HAxxxxxx() : ParserKeyword("HAxxxxxx", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("HASOGCR");
  addDeckName("HASOWCR");
  addDeckName("HASWL");
  addDeckName("HASGLPC");
  addDeckName("HASWLPC");
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
const std::string HAxxxxxx::keywordName = "HAxxxxxx";
const std::string HAxxxxxx::data::itemName = "data";


HBNUM::HBNUM() : ParserKeyword("HBNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("HBNUM");
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
const std::string HBNUM::keywordName = "HBNUM";
const std::string HBNUM::data::itemName = "data";


HDISP::HDISP() : ParserKeyword("HDISP", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HDISP");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Length");
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HDISP::keywordName = "HDISP";
const std::string HDISP::DATA::itemName = "DATA";


HEATCR::HEATCR() : ParserKeyword("HEATCR", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HEATCR");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/Length*Length*Length*AbsoluteTemperature");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string HEATCR::keywordName = "HEATCR";
const std::string HEATCR::data::itemName = "data";


HEATCRT::HEATCRT() : ParserKeyword("HEATCRT", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HEATCRT");
  {
     ParserRecord record;
     {
        ParserItem item("data", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Energy/Length*Length*Length*AbsoluteTemperature*AbsoluteTemperature");
        record.addDataItem(item);
     }
     addDataRecord( record );
  }
}
const std::string HEATCRT::keywordName = "HEATCRT";
const std::string HEATCRT::data::itemName = "data";


HMAQUCT::HMAQUCT() : ParserKeyword("HMAQUCT", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMAQUCT");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_PERM_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_OPEN_ANGLE_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_AQUIFER_DEPTH", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMAQUCT::keywordName = "HMAQUCT";
const std::string HMAQUCT::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMAQUCT::DERIVATIES_RESP_PERM_MULT::itemName = "DERIVATIES_RESP_PERM_MULT";
const std::string HMAQUCT::DERIVATIES_RESP_PERM_MULT::defaultValue = "NO";
const std::string HMAQUCT::DERIVATIES_RESP_OPEN_ANGLE_MULT::itemName = "DERIVATIES_RESP_OPEN_ANGLE_MULT";
const std::string HMAQUCT::DERIVATIES_RESP_OPEN_ANGLE_MULT::defaultValue = "NO";
const std::string HMAQUCT::DERIVATIES_RESP_AQUIFER_DEPTH::itemName = "DERIVATIES_RESP_AQUIFER_DEPTH";
const std::string HMAQUCT::DERIVATIES_RESP_AQUIFER_DEPTH::defaultValue = "NO";


HMAQUFET::HMAQUFET() : ParserKeyword("HMAQUFET", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMAQUFET");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_WAT_VOL_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_AQUIFER_PROD_INDEX_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_AQUIFER_DEPTH", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMAQUFET::keywordName = "HMAQUFET";
const std::string HMAQUFET::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMAQUFET::DERIVATIES_RESP_WAT_VOL_MULT::itemName = "DERIVATIES_RESP_WAT_VOL_MULT";
const std::string HMAQUFET::DERIVATIES_RESP_WAT_VOL_MULT::defaultValue = "NO";
const std::string HMAQUFET::DERIVATIES_RESP_AQUIFER_PROD_INDEX_MULT::itemName = "DERIVATIES_RESP_AQUIFER_PROD_INDEX_MULT";
const std::string HMAQUFET::DERIVATIES_RESP_AQUIFER_PROD_INDEX_MULT::defaultValue = "NO";
const std::string HMAQUFET::DERIVATIES_RESP_AQUIFER_DEPTH::itemName = "DERIVATIES_RESP_AQUIFER_DEPTH";
const std::string HMAQUFET::DERIVATIES_RESP_AQUIFER_DEPTH::defaultValue = "NO";


HMAQUNUM::HMAQUNUM() : ParserKeyword("HMAQUNUM", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HMAQUNUM");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_PORE_VOL_MULT", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_AQUIFER_PERM_MULT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DERIVATIES_RESP_AQUIFER_GRID_CON_TRANS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMAQUNUM::keywordName = "HMAQUNUM";
const std::string HMAQUNUM::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMAQUNUM::DERIVATIES_RESP_PORE_VOL_MULT::itemName = "DERIVATIES_RESP_PORE_VOL_MULT";
const std::string HMAQUNUM::DERIVATIES_RESP_PORE_VOL_MULT::defaultValue = "NO";
const std::string HMAQUNUM::DERIVATIES_RESP_AQUIFER_PERM_MULT::itemName = "DERIVATIES_RESP_AQUIFER_PERM_MULT";
const std::string HMAQUNUM::DERIVATIES_RESP_AQUIFER_GRID_CON_TRANS::itemName = "DERIVATIES_RESP_AQUIFER_GRID_CON_TRANS";
const std::string HMAQUNUM::DERIVATIES_RESP_AQUIFER_GRID_CON_TRANS::defaultValue = "NO";


HMDIMS::HMDIMS() : ParserKeyword("HMDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("HMDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("MAX_GRAD_REGIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_SUB_REGIONS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_GRADS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_FAULTS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_AQUIFER_PARAMS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WELL_PARAMS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("UNUSED", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_ROCK_GRAD_PARAMS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("MAX_WELL_CONN_PARAMS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMDIMS::keywordName = "HMDIMS";
const std::string HMDIMS::MAX_GRAD_REGIONS::itemName = "MAX_GRAD_REGIONS";
const std::string HMDIMS::MAX_SUB_REGIONS::itemName = "MAX_SUB_REGIONS";
const std::string HMDIMS::MAX_GRADS::itemName = "MAX_GRADS";
const std::string HMDIMS::MAX_FAULTS::itemName = "MAX_FAULTS";
const std::string HMDIMS::MAX_AQUIFER_PARAMS::itemName = "MAX_AQUIFER_PARAMS";
const std::string HMDIMS::MAX_WELL_PARAMS::itemName = "MAX_WELL_PARAMS";
const std::string HMDIMS::UNUSED::itemName = "UNUSED";
const std::string HMDIMS::MAX_ROCK_GRAD_PARAMS::itemName = "MAX_ROCK_GRAD_PARAMS";
const std::string HMDIMS::MAX_WELL_CONN_PARAMS::itemName = "MAX_WELL_CONN_PARAMS";


HMFAULTS::HMFAULTS() : ParserKeyword("HMFAULTS", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HMFAULTS");
  {
     ParserRecord record;
     {
        ParserItem item("FAULT_SEGMENT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMFAULTS::keywordName = "HMFAULTS";
const std::string HMFAULTS::FAULT_SEGMENT::itemName = "FAULT_SEGMENT";


HMMLAQUN::HMMLAQUN() : ParserKeyword("HMMLAQUN", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HMMLAQUN");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PORE_VOL_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PORE_PERM_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_GRID_CONN_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMMLAQUN::keywordName = "HMMLAQUN";
const std::string HMMLAQUN::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMMLAQUN::AQUIFER_PORE_VOL_MULT::itemName = "AQUIFER_PORE_VOL_MULT";
const std::string HMMLAQUN::AQUIFER_PORE_PERM_MULT::itemName = "AQUIFER_PORE_PERM_MULT";
const std::string HMMLAQUN::AQUIFER_GRID_CONN_MULT::itemName = "AQUIFER_GRID_CONN_MULT";


HMMLCTAQ::HMMLCTAQ() : ParserKeyword("HMMLCTAQ", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMMLCTAQ");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PERM_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_ANGLE_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_DEPTH_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMMLCTAQ::keywordName = "HMMLCTAQ";
const std::string HMMLCTAQ::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMMLCTAQ::AQUIFER_PERM_MULT::itemName = "AQUIFER_PERM_MULT";
const std::string HMMLCTAQ::AQUIFER_ANGLE_MULT::itemName = "AQUIFER_ANGLE_MULT";
const std::string HMMLCTAQ::AQUIFER_DEPTH_MULT::itemName = "AQUIFER_DEPTH_MULT";


HMMLFTAQ::HMMLFTAQ() : ParserKeyword("HMMLFTAQ", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMMLFTAQ");
  {
     ParserRecord record;
     {
        ParserItem item("AQUIFER_ID", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_WAT_VOL_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_PROD_INDEX_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("AQUIFER_DEPTH_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMMLFTAQ::keywordName = "HMMLFTAQ";
const std::string HMMLFTAQ::AQUIFER_ID::itemName = "AQUIFER_ID";
const std::string HMMLFTAQ::AQUIFER_WAT_VOL_MULT::itemName = "AQUIFER_WAT_VOL_MULT";
const std::string HMMLFTAQ::AQUIFER_PROD_INDEX_MULT::itemName = "AQUIFER_PROD_INDEX_MULT";
const std::string HMMLFTAQ::AQUIFER_DEPTH_MULT::itemName = "AQUIFER_DEPTH_MULT";


HMMLTWCN::HMMLTWCN() : ParserKeyword("HMMLTWCN", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMMLTWCN");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
        item.setDefault( std::string("FIELD") );
        record.addItem(item);
     }
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
        ParserItem item("CTF", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("SKIN", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMMLTWCN::keywordName = "HMMLTWCN";
const std::string HMMLTWCN::WELL::itemName = "WELL";
const std::string HMMLTWCN::GRID::itemName = "GRID";
const std::string HMMLTWCN::GRID::defaultValue = "FIELD";
const std::string HMMLTWCN::I::itemName = "I";
const std::string HMMLTWCN::J::itemName = "J";
const std::string HMMLTWCN::K::itemName = "K";
const std::string HMMLTWCN::CTF::itemName = "CTF";
const std::string HMMLTWCN::SKIN::itemName = "SKIN";


HMMULTFT::HMMULTFT() : ParserKeyword("HMMULTFT", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HMMULTFT");
  {
     ParserRecord record;
     {
        ParserItem item("FAULT", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("TRANS_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     {
        ParserItem item("DIFF_MULT", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMMULTFT::keywordName = "HMMULTFT";
const std::string HMMULTFT::FAULT::itemName = "FAULT";
const std::string HMMULTFT::TRANS_MULT::itemName = "TRANS_MULT";
const std::string HMMULTFT::DIFF_MULT::itemName = "DIFF_MULT";


HMMULTSG::HMMULTSG() : ParserKeyword("HMMULTSG", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HMMULTSG");
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
const std::string HMMULTSG::keywordName = "HMMULTSG";
const std::string HMMULTSG::data::itemName = "data";


HMMULTxx::HMMULTxx() : ParserKeyword("HMMULTxx", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("HMMULTX");
  addDeckName("HMMULTY");
  addDeckName("HMMULTZ");
  addDeckName("HMMULTR");
  addDeckName("HMMULTPV");
  addDeckName("HMMULTTH");
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
const std::string HMMULTxx::keywordName = "HMMULTxx";
const std::string HMMULTxx::data::itemName = "data";


HMPROPS::HMPROPS() : ParserKeyword("HMPROPS", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("HMPROPS");
}
const std::string HMPROPS::keywordName = "HMPROPS";


HMROCK::HMROCK() : ParserKeyword("HMROCK", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HMROCK");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CALCULATE_GRADIENTS", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMROCK::keywordName = "HMROCK";
const std::string HMROCK::TABLE_NUMBER::itemName = "TABLE_NUMBER";
const std::string HMROCK::CALCULATE_GRADIENTS::itemName = "CALCULATE_GRADIENTS";


HMROCKT::HMROCKT() : ParserKeyword("HMROCKT", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HMROCKT");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_NUMBER", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CALCULATE_GRADIENTS_1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("CALCULATE_GRADIENTS_2", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMROCKT::keywordName = "HMROCKT";
const std::string HMROCKT::TABLE_NUMBER::itemName = "TABLE_NUMBER";
const std::string HMROCKT::CALCULATE_GRADIENTS_1::itemName = "CALCULATE_GRADIENTS_1";
const std::string HMROCKT::CALCULATE_GRADIENTS_2::itemName = "CALCULATE_GRADIENTS_2";


HMRREF::HMRREF() : ParserKeyword("HMRREF", KeywordSize("ROCKCOMP", "NTROCC", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HMRREF");
  {
     ParserRecord record;
     {
        ParserItem item("P_REF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("P_DIM", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMRREF::keywordName = "HMRREF";
const std::string HMRREF::P_REF::itemName = "P_REF";
const std::string HMRREF::P_DIM::itemName = "P_DIM";


HMWELCON::HMWELCON() : ParserKeyword("HMWELCON", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("HMWELCON");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("GRID", ParserItem::itype::STRING);
        item.setDefault( std::string("FIELD") );
        record.addItem(item);
     }
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
        ParserItem item("REQ_TRANS_FACTOR_GRAD", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("REQ_SKIN_FACTOR_GRAD", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMWELCON::keywordName = "HMWELCON";
const std::string HMWELCON::WELL::itemName = "WELL";
const std::string HMWELCON::GRID::itemName = "GRID";
const std::string HMWELCON::GRID::defaultValue = "FIELD";
const std::string HMWELCON::I::itemName = "I";
const std::string HMWELCON::J::itemName = "J";
const std::string HMWELCON::K::itemName = "K";
const std::string HMWELCON::REQ_TRANS_FACTOR_GRAD::itemName = "REQ_TRANS_FACTOR_GRAD";
const std::string HMWELCON::REQ_TRANS_FACTOR_GRAD::defaultValue = "NO";
const std::string HMWELCON::REQ_SKIN_FACTOR_GRAD::itemName = "REQ_SKIN_FACTOR_GRAD";
const std::string HMWELCON::REQ_SKIN_FACTOR_GRAD::defaultValue = "NO";


HMWPIMLT::HMWPIMLT() : ParserKeyword("HMWPIMLT", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("HMWPIMLT");
  {
     ParserRecord record;
     {
        ParserItem item("WELL", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HMWPIMLT::keywordName = "HMWPIMLT";
const std::string HMWPIMLT::WELL::itemName = "WELL";


HMxxxxxx::HMxxxxxx() : ParserKeyword("HMxxxxxx", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("HMTRNXY");
  addDeckName("HMSWCR");
  addDeckName("HMTRANX");
  addDeckName("HMTRANY");
  addDeckName("HMPERMY");
  addDeckName("HMTRANZ");
  addDeckName("HMPORVM");
  addDeckName("HMSIGMA");
  addDeckName("HMPERMX");
  addDeckName("HMSOGCR");
  addDeckName("HMKRO");
  addDeckName("HMPRMXY");
  addDeckName("HMPCW");
  addDeckName("HMPERMZ");
  addDeckName("HMSGCR");
  addDeckName("HMSOWCR");
  addDeckName("HMSWL");
  addDeckName("HMKRW");
  addDeckName("HMKRG");
  addDeckName("HMKRWR");
  addDeckName("HMKRGR");
  addDeckName("HMKRORW");
  addDeckName("HMKRORG");
  addDeckName("HMPCG");
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
const std::string HMxxxxxx::keywordName = "HMxxxxxx";
const std::string HMxxxxxx::data::itemName = "data";


HRFIN::HRFIN() : ParserKeyword("HRFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HRFIN");
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
const std::string HRFIN::keywordName = "HRFIN";
const std::string HRFIN::data::itemName = "data";


HWELLS::HWELLS() : ParserKeyword("HWELLS", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("HWELLS");
}
const std::string HWELLS::keywordName = "HWELLS";


HWKRO::HWKRO() : ParserKeyword("HWKRO", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWKRO");
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
const std::string HWKRO::keywordName = "HWKRO";
const std::string HWKRO::data::itemName = "data";


HWKRORG::HWKRORG() : ParserKeyword("HWKRORG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWKRORG");
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
const std::string HWKRORG::keywordName = "HWKRORG";
const std::string HWKRORG::data::itemName = "data";


HWKRORW::HWKRORW() : ParserKeyword("HWKRORW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWKRORW");
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
const std::string HWKRORW::keywordName = "HWKRORW";
const std::string HWKRORW::data::itemName = "data";


HWKRW::HWKRW() : ParserKeyword("HWKRW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWKRW");
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
const std::string HWKRW::keywordName = "HWKRW";
const std::string HWKRW::data::itemName = "data";


HWKRWR::HWKRWR() : ParserKeyword("HWKRWR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWKRWR");
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
const std::string HWKRWR::keywordName = "HWKRWR";
const std::string HWKRWR::data::itemName = "data";


HWPCW::HWPCW() : ParserKeyword("HWPCW", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWPCW");
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
const std::string HWPCW::keywordName = "HWPCW";
const std::string HWPCW::data::itemName = "data";


HWSNUM::HWSNUM() : ParserKeyword("HWSNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("HWSNUM");
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
const std::string HWSNUM::keywordName = "HWSNUM";
const std::string HWSNUM::data::itemName = "data";


HWSOGCR::HWSOGCR() : ParserKeyword("HWSOGCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSOGCR");
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
const std::string HWSOGCR::keywordName = "HWSOGCR";
const std::string HWSOGCR::data::itemName = "data";


HWSOWCR::HWSOWCR() : ParserKeyword("HWSOWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSOWCR");
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
const std::string HWSOWCR::keywordName = "HWSOWCR";
const std::string HWSOWCR::data::itemName = "data";


HWSWCR::HWSWCR() : ParserKeyword("HWSWCR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSWCR");
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
const std::string HWSWCR::keywordName = "HWSWCR";
const std::string HWSWCR::data::itemName = "data";


HWSWL::HWSWL() : ParserKeyword("HWSWL", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSWL");
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
const std::string HWSWL::keywordName = "HWSWL";
const std::string HWSWL::data::itemName = "data";


HWSWLPC::HWSWLPC() : ParserKeyword("HWSWLPC", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSWLPC");
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
const std::string HWSWLPC::keywordName = "HWSWLPC";
const std::string HWSWLPC::data::itemName = "data";


HWSWU::HWSWU() : ParserKeyword("HWSWU", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HWSWU");
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
const std::string HWSWU::keywordName = "HWSWU";
const std::string HWSWU::data::itemName = "data";


HXFIN::HXFIN() : ParserKeyword("HXFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HXFIN");
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
const std::string HXFIN::keywordName = "HXFIN";
const std::string HXFIN::data::itemName = "data";


HYDRHEAD::HYDRHEAD() : ParserKeyword("HYDRHEAD", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HYDRHEAD");
  {
     ParserRecord record;
     {
        ParserItem item("REF_DEPTH", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("FRESHWATER_DENSITY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     {
        ParserItem item("REMOVE_DEPTH_TERMS", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string HYDRHEAD::keywordName = "HYDRHEAD";
const std::string HYDRHEAD::REF_DEPTH::itemName = "REF_DEPTH";
const std::string HYDRHEAD::FRESHWATER_DENSITY::itemName = "FRESHWATER_DENSITY";
const std::string HYDRHEAD::REMOVE_DEPTH_TERMS::itemName = "REMOVE_DEPTH_TERMS";
const std::string HYDRHEAD::REMOVE_DEPTH_TERMS::defaultValue = "NO";


HYFIN::HYFIN() : ParserKeyword("HYFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HYFIN");
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
const std::string HYFIN::keywordName = "HYFIN";
const std::string HYFIN::data::itemName = "data";


HYMOBGDR::HYMOBGDR() : ParserKeyword("HYMOBGDR", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HYMOBGDR");
}
const std::string HYMOBGDR::keywordName = "HYMOBGDR";


HYST::HYST() : ParserKeyword("HYST", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("HYST");
}
const std::string HYST::keywordName = "HYST";


HYSTCHCK::HYSTCHCK() : ParserKeyword("HYSTCHCK", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("HYSTCHCK");
  {
     ParserRecord record;
     addRecord( record );
  }
}
const std::string HYSTCHCK::keywordName = "HYSTCHCK";


HZFIN::HZFIN() : ParserKeyword("HZFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("HZFIN");
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
const std::string HZFIN::keywordName = "HZFIN";
const std::string HZFIN::data::itemName = "data";


}
}
