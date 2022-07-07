
#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Parser/ParserItem.hpp>
#include <opm/input/eclipse/Parser/ParserRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>





#include <opm/input/eclipse/Parser/ParserKeywords/E.hpp>
namespace Opm {
namespace ParserKeywords {
ECHO::ECHO() : ParserKeyword("ECHO", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ECHO");
}
const std::string ECHO::keywordName = "ECHO";


ECLMC::ECLMC() : ParserKeyword("ECLMC", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ECLMC");
}
const std::string ECLMC::keywordName = "ECLMC";


EDIT::EDIT() : ParserKeyword("EDIT", KeywordSize(0, false)) {
  clearDeckNames();
  addDeckName("EDIT");
}
const std::string EDIT::keywordName = "EDIT";


EDITNNC::EDITNNC() : ParserKeyword("EDITNNC", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("EDITNNC");
  {
     ParserRecord record;
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRAN_MULT", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE12", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE21", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PRESS_TABLE12", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PRESS_TABLE21", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_FLOW12", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_FLOW21", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIFFM", ParserItem::itype::DOUBLE);
        record.addItem(item);
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


EDITNNCR::EDITNNCR() : ParserKeyword("EDITNNCR", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("EDIT");
  clearDeckNames();
  addDeckName("EDITNNCR");
  {
     ParserRecord record;
     {
        ParserItem item("I1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K1", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("I2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("J2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("K2", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("TRANS", ParserItem::itype::DOUBLE);
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE12", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("SAT_TABLE21", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PRESS_TABLE12", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("PRESS_TABLE21", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_FLOW12", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("FACE_FLOW21", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("DIFF", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EDITNNCR::keywordName = "EDITNNCR";
const std::string EDITNNCR::I1::itemName = "I1";
const std::string EDITNNCR::J1::itemName = "J1";
const std::string EDITNNCR::K1::itemName = "K1";
const std::string EDITNNCR::I2::itemName = "I2";
const std::string EDITNNCR::J2::itemName = "J2";
const std::string EDITNNCR::K2::itemName = "K2";
const std::string EDITNNCR::TRANS::itemName = "TRANS";
const std::string EDITNNCR::SAT_TABLE12::itemName = "SAT_TABLE12";
const std::string EDITNNCR::SAT_TABLE21::itemName = "SAT_TABLE21";
const std::string EDITNNCR::PRESS_TABLE12::itemName = "PRESS_TABLE12";
const std::string EDITNNCR::PRESS_TABLE21::itemName = "PRESS_TABLE21";
const std::string EDITNNCR::FACE_FLOW12::itemName = "FACE_FLOW12";
const std::string EDITNNCR::FACE_FLOW21::itemName = "FACE_FLOW21";
const std::string EDITNNCR::DIFF::itemName = "DIFF";


EHYSTR::EHYSTR() : ParserKeyword("EHYSTR", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("EHYSTR");
  {
     ParserRecord record;
     {
        ParserItem item("curvature_caplillary_pressure_hyst", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("relative_perm_hyst", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("curvature_param_killough_wetting", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("mod_param_trapped", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("limiting_hyst_flag", ParserItem::itype::STRING);
        item.setDefault( std::string("BOTH") );
        record.addItem(item);
     }
     {
        ParserItem item("shape_cap_press_flag", ParserItem::itype::STRING);
        item.setDefault( std::string("RETR") );
        record.addItem(item);
     }
     {
        ParserItem item("init_fluid_mob_flag", ParserItem::itype::STRING);
        item.setDefault( std::string("DRAIN") );
        record.addItem(item);
     }
     {
        ParserItem item("wetting_phase_flag", ParserItem::itype::STRING);
        item.setDefault( std::string("OIL") );
        record.addItem(item);
     }
     {
        ParserItem item("baker_flag_oil", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("baker_flag_gas", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("baker_flag_water", ParserItem::itype::STRING);
        item.setDefault( std::string("NO") );
        record.addItem(item);
     }
     {
        ParserItem item("threshold_saturation", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        record.addItem(item);
     }
     {
        ParserItem item("FLAG_SOMETHING", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EHYSTR::keywordName = "EHYSTR";
const std::string EHYSTR::curvature_caplillary_pressure_hyst::itemName = "curvature_caplillary_pressure_hyst";
const double EHYSTR::curvature_caplillary_pressure_hyst::defaultValue = 0.100000;
const std::string EHYSTR::relative_perm_hyst::itemName = "relative_perm_hyst";
const int EHYSTR::relative_perm_hyst::defaultValue = 0;
const std::string EHYSTR::curvature_param_killough_wetting::itemName = "curvature_param_killough_wetting";
const double EHYSTR::curvature_param_killough_wetting::defaultValue = 1.000000;
const std::string EHYSTR::mod_param_trapped::itemName = "mod_param_trapped";
const double EHYSTR::mod_param_trapped::defaultValue = 0.100000;
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
const std::string EHYSTR::FLAG_SOMETHING::itemName = "FLAG_SOMETHING";
const int EHYSTR::FLAG_SOMETHING::defaultValue = 0;


EHYSTRR::EHYSTRR() : ParserKeyword("EHYSTRR", KeywordSize("TABDIMS", "NTSFUN", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("EHYSTRR");
  {
     ParserRecord record;
     {
        ParserItem item("curvature_caplillary_pressure_hyst", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     {
        ParserItem item("curvature_parameter_wetting_phase_hyst", ParserItem::itype::DOUBLE);
        item.setDefault( double(1.000000) );
        record.addItem(item);
     }
     {
        ParserItem item("mod_param_non_wet_phase_sat", ParserItem::itype::DOUBLE);
        item.setDefault( double(0.100000) );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EHYSTRR::keywordName = "EHYSTRR";
const std::string EHYSTRR::curvature_caplillary_pressure_hyst::itemName = "curvature_caplillary_pressure_hyst";
const double EHYSTRR::curvature_caplillary_pressure_hyst::defaultValue = 0.100000;
const std::string EHYSTRR::curvature_parameter_wetting_phase_hyst::itemName = "curvature_parameter_wetting_phase_hyst";
const double EHYSTRR::curvature_parameter_wetting_phase_hyst::defaultValue = 1.000000;
const std::string EHYSTRR::mod_param_non_wet_phase_sat::itemName = "mod_param_non_wet_phase_sat";
const double EHYSTRR::mod_param_non_wet_phase_sat::defaultValue = 0.100000;


END::END() : ParserKeyword("END", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("END");
}
const std::string END::keywordName = "END";


ENDACTIO::ENDACTIO() : ParserKeyword("ENDACTIO", KeywordSize(0, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDACTIO");
}
const std::string ENDACTIO::keywordName = "ENDACTIO";


ENDBOX::ENDBOX() : ParserKeyword("ENDBOX", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDBOX");
}
const std::string ENDBOX::keywordName = "ENDBOX";


ENDDYN::ENDDYN() : ParserKeyword("ENDDYN", KeywordSize(0, false)) {
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDDYN");
}
const std::string ENDDYN::keywordName = "ENDDYN";


ENDFIN::ENDFIN() : ParserKeyword("ENDFIN", KeywordSize(0, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDFIN");
}
const std::string ENDFIN::keywordName = "ENDFIN";


ENDINC::ENDINC() : ParserKeyword("ENDINC", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDINC");
}
const std::string ENDINC::keywordName = "ENDINC";


ENDNUM::ENDNUM() : ParserKeyword("ENDNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("ENDNUM");
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
const std::string ENDNUM::keywordName = "ENDNUM";
const std::string ENDNUM::data::itemName = "data";


ENDPOINT_SPECIFIERS::ENDPOINT_SPECIFIERS() : ParserKeyword("ENDPOINT_SPECIFIERS", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("KRORGY");
  addDeckName("KRGR");
  addDeckName("IKRGRZ-");
  addDeckName("KRGZ");
  addDeckName("IKRGX-");
  addDeckName("ISOWCRX-");
  addDeckName("KRG");
  addDeckName("IKRORWY-");
  addDeckName("SWCRZ-");
  addDeckName("KRGX");
  addDeckName("KRGY");
  addDeckName("IKRO");
  addDeckName("KRORG");
  addDeckName("KRGX-");
  addDeckName("KRGY-");
  addDeckName("SWLX");
  addDeckName("KRGZ-");
  addDeckName("KRGRX");
  addDeckName("KRGRY");
  addDeckName("IKROX-");
  addDeckName("KRO");
  addDeckName("KRORGX-");
  addDeckName("KRGRZ");
  addDeckName("KRGRX-");
  addDeckName("KRGRY-");
  addDeckName("IKRORGY");
  addDeckName("SOWCRY");
  addDeckName("KRGRZ-");
  addDeckName("IKRGRY-");
  addDeckName("IKRG");
  addDeckName("ISOWCR");
  addDeckName("IKRORWX-");
  addDeckName("IKRGX");
  addDeckName("ISOWCRX");
  addDeckName("KRORWX-");
  addDeckName("IKRGY");
  addDeckName("ISOWCRY");
  addDeckName("IKRGZ");
  addDeckName("ISOWCRZ");
  addDeckName("IKRGY-");
  addDeckName("ISOWCRY-");
  addDeckName("IKRGZ-");
  addDeckName("ISWUX");
  addDeckName("ISOWCRZ-");
  addDeckName("KRORWZ");
  addDeckName("IKRGR");
  addDeckName("IKRORWZ-");
  addDeckName("SWCRY-");
  addDeckName("IKRGRX");
  addDeckName("IKRGRY");
  addDeckName("ISGUY-");
  addDeckName("IKRGRZ");
  addDeckName("IKRORGX");
  addDeckName("SOWCRX");
  addDeckName("IKRGRX-");
  addDeckName("KROX");
  addDeckName("KROY");
  addDeckName("KROZ");
  addDeckName("KROX-");
  addDeckName("ISOGCR");
  addDeckName("IKRORWX");
  addDeckName("KROY-");
  addDeckName("KROZ-");
  addDeckName("KRORW");
  addDeckName("KRORWX");
  addDeckName("KRORWY");
  addDeckName("KRORWY-");
  addDeckName("KRORWZ-");
  addDeckName("KRORGX");
  addDeckName("KRORGZ");
  addDeckName("SOGCRZ-");
  addDeckName("IKROY-");
  addDeckName("KRORGY-");
  addDeckName("IKROZ-");
  addDeckName("KRORGZ-");
  addDeckName("IKRORW");
  addDeckName("IKRORWY");
  addDeckName("KRWZ-");
  addDeckName("SWCRZ");
  addDeckName("IKRORWZ");
  addDeckName("SOGCR");
  addDeckName("SWCRY");
  addDeckName("IKRORG");
  addDeckName("SOWCR");
  addDeckName("IKRORGZ");
  addDeckName("SOWCRZ");
  addDeckName("IKRORGX-");
  addDeckName("SOWCRX-");
  addDeckName("IKRORGY-");
  addDeckName("SOWCRY-");
  addDeckName("IKRORGZ-");
  addDeckName("SOWCRZ-");
  addDeckName("KRW");
  addDeckName("KRWX");
  addDeckName("KRWY");
  addDeckName("KRWZ");
  addDeckName("KRWX-");
  addDeckName("KRWY-");
  addDeckName("KRWR");
  addDeckName("KRWRX");
  addDeckName("KRWRY");
  addDeckName("KRWRZ");
  addDeckName("KRWRX-");
  addDeckName("KRWRY-");
  addDeckName("KRWRZ-");
  addDeckName("IKRW");
  addDeckName("ISGCRZ-");
  addDeckName("IKRWX");
  addDeckName("IKRWY");
  addDeckName("IKRWZ");
  addDeckName("IKRWX-");
  addDeckName("IKRWY-");
  addDeckName("IKRWZ-");
  addDeckName("IKRWR");
  addDeckName("IKRWRX");
  addDeckName("IKRWRY");
  addDeckName("IKRWRZ");
  addDeckName("IKRWRX-");
  addDeckName("IKRWRY-");
  addDeckName("IKRWRZ-");
  addDeckName("SWL");
  addDeckName("SWLZ");
  addDeckName("ISWL");
  addDeckName("SWLX-");
  addDeckName("ISWLX");
  addDeckName("ISWLX-");
  addDeckName("SWLY");
  addDeckName("SWLY-");
  addDeckName("ISWCRZ");
  addDeckName("ISWLY");
  addDeckName("ISWLY-");
  addDeckName("SWLZ-");
  addDeckName("ISWLZ");
  addDeckName("ISWLZ-");
  addDeckName("SGU");
  addDeckName("ISGU");
  addDeckName("SGUX");
  addDeckName("SGUX-");
  addDeckName("ISGUX");
  addDeckName("ISGUX-");
  addDeckName("SGUY");
  addDeckName("SGUY-");
  addDeckName("SGCRZ");
  addDeckName("ISGUY");
  addDeckName("SGUZ");
  addDeckName("ISWUZ-");
  addDeckName("SGUZ-");
  addDeckName("ISGUZ");
  addDeckName("ISGUZ-");
  addDeckName("SWU");
  addDeckName("ISWU");
  addDeckName("SWUX");
  addDeckName("SWUX-");
  addDeckName("ISWUX-");
  addDeckName("SWUY");
  addDeckName("SWUY-");
  addDeckName("ISWUY");
  addDeckName("ISWUY-");
  addDeckName("SWUZ");
  addDeckName("SWUZ-");
  addDeckName("ISWUZ");
  addDeckName("SGCR");
  addDeckName("ISGCR");
  addDeckName("SGCRX");
  addDeckName("SGCRX-");
  addDeckName("ISGCRX");
  addDeckName("ISGCRX-");
  addDeckName("SGCRY");
  addDeckName("SGCRY-");
  addDeckName("ISGCRY");
  addDeckName("ISGCRY-");
  addDeckName("SGCRZ-");
  addDeckName("ISGCRZ");
  addDeckName("SOGCRX");
  addDeckName("SOGCRX-");
  addDeckName("ISOGCRX");
  addDeckName("ISOGCRX-");
  addDeckName("SOGCRY");
  addDeckName("SOGCRY-");
  addDeckName("ISOGCRY");
  addDeckName("ISOGCRY-");
  addDeckName("SOGCRZ");
  addDeckName("ISOGCRZ");
  addDeckName("ISOGCRZ-");
  addDeckName("SWCR");
  addDeckName("ISWCR");
  addDeckName("SWCRX");
  addDeckName("SWCRX-");
  addDeckName("ISWCRX");
  addDeckName("ISWCRX-");
  addDeckName("ISWCRZ-");
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
const std::string ENDPOINT_SPECIFIERS::keywordName = "ENDPOINT_SPECIFIERS";
const std::string ENDPOINT_SPECIFIERS::data::itemName = "data";


ENDSCALE::ENDSCALE() : ParserKeyword("ENDSCALE", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("ENDSCALE");
  {
     ParserRecord record;
     {
        ParserItem item("DIRECT", ParserItem::itype::STRING);
        item.setDefault( std::string("NODIR") );
        record.addItem(item);
     }
     {
        ParserItem item("IRREVERS", ParserItem::itype::STRING);
        item.setDefault( std::string("REVERS") );
        record.addItem(item);
     }
     {
        ParserItem item("NTENDP", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NSENDP", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("COMB_MODE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENDSCALE::keywordName = "ENDSCALE";
const std::string ENDSCALE::DIRECT::itemName = "DIRECT";
const std::string ENDSCALE::DIRECT::defaultValue = "NODIR";
const std::string ENDSCALE::IRREVERS::itemName = "IRREVERS";
const std::string ENDSCALE::IRREVERS::defaultValue = "REVERS";
const std::string ENDSCALE::NTENDP::itemName = "NTENDP";
const int ENDSCALE::NTENDP::defaultValue = 1;
const std::string ENDSCALE::NSENDP::itemName = "NSENDP";
const int ENDSCALE::NSENDP::defaultValue = 20;
const std::string ENDSCALE::COMB_MODE::itemName = "COMB_MODE";
const int ENDSCALE::COMB_MODE::defaultValue = 0;


ENDSKIP::ENDSKIP() : ParserKeyword("ENDSKIP", KeywordSize(0, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("ENDSKIP");
}
const std::string ENDSKIP::keywordName = "ENDSKIP";


ENKRVD::ENKRVD() : ParserKeyword("ENKRVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENKRVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(-1.000000) );
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
const std::string ENKRVD::keywordName = "ENKRVD";
const std::string ENKRVD::DATA::itemName = "DATA";
const double ENKRVD::DATA::defaultValue = -1.000000;


ENPCVD::ENPCVD() : ParserKeyword("ENPCVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENPCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Length");
        item.push_backDimension("Pressure");
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENPCVD::keywordName = "ENPCVD";
const std::string ENPCVD::DATA::itemName = "DATA";
const double ENPCVD::DATA::defaultValue = -1.000000;


ENPTVD::ENPTVD() : ParserKeyword("ENPTVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENPTVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(-1.000000) );
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
const std::string ENPTVD::keywordName = "ENPTVD";
const std::string ENPTVD::DATA::itemName = "DATA";
const double ENPTVD::DATA::defaultValue = -1.000000;


ENSPCVD::ENSPCVD() : ParserKeyword("ENSPCVD", KeywordSize("ENDSCALE", "NTENDP", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ENSPCVD");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.setDefault( double(-1.000000) );
        item.push_backDimension("Length");
        item.push_backDimension("1");
        item.push_backDimension("1");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ENSPCVD::keywordName = "ENSPCVD";
const std::string ENSPCVD::DATA::itemName = "DATA";
const double ENSPCVD::DATA::defaultValue = -1.000000;


EPSDBGS::EPSDBGS() : ParserKeyword("EPSDBGS", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("EPSDBGS");
  {
     ParserRecord record;
     {
        ParserItem item("TABLE_OUTPUT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("CHECK_DRAIN_HYST", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
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
        ParserItem item("GRID_NAME", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EPSDBGS::keywordName = "EPSDBGS";
const std::string EPSDBGS::TABLE_OUTPUT::itemName = "TABLE_OUTPUT";
const int EPSDBGS::TABLE_OUTPUT::defaultValue = 0;
const std::string EPSDBGS::CHECK_DRAIN_HYST::itemName = "CHECK_DRAIN_HYST";
const int EPSDBGS::CHECK_DRAIN_HYST::defaultValue = 0;
const std::string EPSDBGS::IX1::itemName = "IX1";
const std::string EPSDBGS::IX2::itemName = "IX2";
const std::string EPSDBGS::JY1::itemName = "JY1";
const std::string EPSDBGS::JY2::itemName = "JY2";
const std::string EPSDBGS::KZ1::itemName = "KZ1";
const std::string EPSDBGS::KZ2::itemName = "KZ2";
const std::string EPSDBGS::GRID_NAME::itemName = "GRID_NAME";
const std::string EPSDBGS::GRID_NAME::defaultValue = "";


EPSDEBUG::EPSDEBUG() : ParserKeyword("EPSDEBUG", KeywordSize(1, false)) {
  addValidSectionName("PROPS");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("EPSDEBUG");
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
        ParserItem item("TABLE_OUTPUT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("GRID_NAME", ParserItem::itype::STRING);
        item.setDefault( std::string("") );
        record.addItem(item);
     }
     {
        ParserItem item("CHECK_DRAIN_HYST", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EPSDEBUG::keywordName = "EPSDEBUG";
const std::string EPSDEBUG::IX1::itemName = "IX1";
const std::string EPSDEBUG::IX2::itemName = "IX2";
const std::string EPSDEBUG::JY1::itemName = "JY1";
const std::string EPSDEBUG::JY2::itemName = "JY2";
const std::string EPSDEBUG::KZ1::itemName = "KZ1";
const std::string EPSDEBUG::KZ2::itemName = "KZ2";
const std::string EPSDEBUG::TABLE_OUTPUT::itemName = "TABLE_OUTPUT";
const int EPSDEBUG::TABLE_OUTPUT::defaultValue = 0;
const std::string EPSDEBUG::GRID_NAME::itemName = "GRID_NAME";
const std::string EPSDEBUG::GRID_NAME::defaultValue = "";
const std::string EPSDEBUG::CHECK_DRAIN_HYST::itemName = "CHECK_DRAIN_HYST";
const int EPSDEBUG::CHECK_DRAIN_HYST::defaultValue = 0;


EQLDIMS::EQLDIMS() : ParserKeyword("EQLDIMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("EQLDIMS");
  {
     ParserRecord record;
     {
        ParserItem item("NTEQUL", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_NODES_P", ParserItem::itype::INT);
        item.setDefault( 100 );
        record.addItem(item);
     }
     {
        ParserItem item("DEPTH_NODES_TAB", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
     }
     {
        ParserItem item("NTTRVD", ParserItem::itype::INT);
        item.setDefault( 1 );
        record.addItem(item);
     }
     {
        ParserItem item("NSTRVD", ParserItem::itype::INT);
        item.setDefault( 20 );
        record.addItem(item);
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


EQLNUM::EQLNUM() : ParserKeyword("EQLNUM", KeywordSize(1, false)) {
  addValidSectionName("REGIONS");
  clearDeckNames();
  addDeckName("EQLNUM");
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
const std::string EQLNUM::keywordName = "EQLNUM";
const std::string EQLNUM::data::itemName = "data";


EQLOPTS::EQLOPTS() : ParserKeyword("EQLOPTS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  clearDeckNames();
  addDeckName("EQLOPTS");
  {
     ParserRecord record;
     {
        ParserItem item("OPTION1", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPTION2", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPTION3", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("OPTION4", ParserItem::itype::STRING);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQLOPTS::keywordName = "EQLOPTS";
const std::string EQLOPTS::OPTION1::itemName = "OPTION1";
const std::string EQLOPTS::OPTION2::itemName = "OPTION2";
const std::string EQLOPTS::OPTION3::itemName = "OPTION3";
const std::string EQLOPTS::OPTION4::itemName = "OPTION4";


EQLZCORN::EQLZCORN() : ParserKeyword("EQLZCORN", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("EQLZCORN");
  {
     ParserRecord record;
     {
        ParserItem item("VALUE_ZCORN_ARRAY", ParserItem::itype::DOUBLE);
        item.push_backDimension("Length");
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
        ParserItem item("IX1A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("IX2A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY1A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("JY2A", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("ACTION_REQ", ParserItem::itype::STRING);
        item.setDefault( std::string("TOP") );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EQLZCORN::keywordName = "EQLZCORN";
const std::string EQLZCORN::VALUE_ZCORN_ARRAY::itemName = "VALUE_ZCORN_ARRAY";
const std::string EQLZCORN::IX1::itemName = "IX1";
const std::string EQLZCORN::IX2::itemName = "IX2";
const std::string EQLZCORN::JY1::itemName = "JY1";
const std::string EQLZCORN::JY2::itemName = "JY2";
const std::string EQLZCORN::KZ1::itemName = "KZ1";
const std::string EQLZCORN::KZ2::itemName = "KZ2";
const std::string EQLZCORN::IX1A::itemName = "IX1A";
const std::string EQLZCORN::IX2A::itemName = "IX2A";
const std::string EQLZCORN::JY1A::itemName = "JY1A";
const std::string EQLZCORN::JY2A::itemName = "JY2A";
const std::string EQLZCORN::ACTION_REQ::itemName = "ACTION_REQ";
const std::string EQLZCORN::ACTION_REQ::defaultValue = "TOP";


EQUALREG::EQUALREG() : ParserKeyword("EQUALREG", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("GRID");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUALREG");
  {
     ParserRecord record;
     {
        ParserItem item("ARRAY", ParserItem::itype::STRING);
        item.setDescription("The 3D array we will update");
        record.addItem(item);
     }
     {
        ParserItem item("VALUE", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.setDescription("The value we will assign");
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NUMBER", ParserItem::itype::INT);
        item.setDescription("The region number we are interested in");
        record.addItem(item);
     }
     {
        ParserItem item("REGION_NAME", ParserItem::itype::STRING);
        item.setDefault( std::string("M") );
        item.setDescription("The name of the region we are interested in");
        record.addItem(item);
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


EQUALS::EQUALS() : ParserKeyword("EQUALS", KeywordSize(SLASH_TERMINATED)) {
  addValidSectionName("GRID");
  addValidSectionName("EDIT");
  addValidSectionName("PROPS");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUALS");
  {
     ParserRecord record;
     {
        ParserItem item("field", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("value", ParserItem::itype::DOUBLE);
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


EQUIL::EQUIL() : ParserKeyword("EQUIL", KeywordSize("EQLDIMS", "NTEQUL", false, 0)) {
  addValidSectionName("SOLUTION");
  clearDeckNames();
  addDeckName("EQUIL");
  {
     ParserRecord record;
     {
        ParserItem item("DATUM_DEPTH", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("DATUM_PRESSURE", ParserItem::itype::DOUBLE);
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("OWC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        item.setDescription("The OWC item is depth of the OIL Water contact. This should ...");
        record.addItem(item);
     }
     {
        ParserItem item("PC_OWC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("GOC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Length");
        record.addItem(item);
     }
     {
        ParserItem item("PC_GOC", ParserItem::itype::DOUBLE);
        item.setDefault( double(0) );
        item.push_backDimension("Pressure");
        record.addItem(item);
     }
     {
        ParserItem item("BLACK_OIL_INIT", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("BLACK_OIL_INIT_WG", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     {
        ParserItem item("OIP_INIT", ParserItem::itype::INT);
        item.setDefault( -5 );
        record.addItem(item);
     }
     {
        ParserItem item("EQLOPT04", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("EQLOPT5", ParserItem::itype::INT);
        record.addItem(item);
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
const std::string EQUIL::EQLOPT04::itemName = "EQLOPT04";
const std::string EQUIL::EQLOPT5::itemName = "EQLOPT5";


ESSNODE::ESSNODE() : ParserKeyword("ESSNODE", KeywordSize("TABDIMS", "NTPVT", false, 0)) {
  addValidSectionName("PROPS");
  clearDeckNames();
  addDeckName("ESSNODE");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::DOUBLE);
        item.setSizeType(ParserItem::item_size::ALL);
        item.push_backDimension("Density");
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string ESSNODE::keywordName = "ESSNODE";
const std::string ESSNODE::DATA::itemName = "DATA";


EXCAVATE::EXCAVATE() : ParserKeyword("EXCAVATE", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("EXCAVATE");
  {
     ParserRecord record;
     {
        ParserItem item("DATA", ParserItem::itype::INT);
        item.setSizeType(ParserItem::item_size::ALL);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EXCAVATE::keywordName = "EXCAVATE";
const std::string EXCAVATE::DATA::itemName = "DATA";


EXCEL::EXCEL() : ParserKeyword("EXCEL", KeywordSize(0, false)) {
  addValidSectionName("SUMMARY");
  clearDeckNames();
  addDeckName("EXCEL");
}
const std::string EXCEL::keywordName = "EXCEL";


EXIT::EXIT() : ParserKeyword("EXIT", KeywordSize(1, false)) {
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("EXIT");
  {
     ParserRecord record;
     {
        ParserItem item("STATUS_CODE", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EXIT::keywordName = "EXIT";
const std::string EXIT::STATUS_CODE::itemName = "STATUS_CODE";
const int EXIT::STATUS_CODE::defaultValue = 0;


EXTFIN::EXTFIN() : ParserKeyword("EXTFIN", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("EXTFIN");
  {
     ParserRecord record;
     {
        ParserItem item("LOCAL_GRID_REF", ParserItem::itype::STRING);
        record.addItem(item);
     }
     {
        ParserItem item("NX", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NY", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NZ", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NREPG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NHALO", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NFLOG", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NUMINT", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NUMCON", ParserItem::itype::INT);
        record.addItem(item);
     }
     {
        ParserItem item("NWMAX", ParserItem::itype::INT);
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EXTFIN::keywordName = "EXTFIN";
const std::string EXTFIN::LOCAL_GRID_REF::itemName = "LOCAL_GRID_REF";
const std::string EXTFIN::NX::itemName = "NX";
const std::string EXTFIN::NY::itemName = "NY";
const std::string EXTFIN::NZ::itemName = "NZ";
const std::string EXTFIN::NREPG::itemName = "NREPG";
const std::string EXTFIN::NHALO::itemName = "NHALO";
const std::string EXTFIN::NFLOG::itemName = "NFLOG";
const std::string EXTFIN::NUMINT::itemName = "NUMINT";
const std::string EXTFIN::NUMCON::itemName = "NUMCON";
const std::string EXTFIN::NWMAX::itemName = "NWMAX";


EXTHOST::EXTHOST() : ParserKeyword("EXTHOST", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("EXTHOST");
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
const std::string EXTHOST::keywordName = "EXTHOST";
const std::string EXTHOST::data::itemName = "data";


EXTRAPMS::EXTRAPMS() : ParserKeyword("EXTRAPMS", KeywordSize(1, false)) {
  addValidSectionName("RUNSPEC");
  addValidSectionName("GRID");
  addValidSectionName("PROPS");
  addValidSectionName("EDIT");
  addValidSectionName("REGIONS");
  addValidSectionName("SOLUTION");
  addValidSectionName("SUMMARY");
  addValidSectionName("SCHEDULE");
  clearDeckNames();
  addDeckName("EXTRAPMS");
  {
     ParserRecord record;
     {
        ParserItem item("LEVEL", ParserItem::itype::INT);
        item.setDefault( 0 );
        record.addItem(item);
     }
     addRecord( record );
  }
}
const std::string EXTRAPMS::keywordName = "EXTRAPMS";
const std::string EXTRAPMS::LEVEL::itemName = "LEVEL";
const int EXTRAPMS::LEVEL::defaultValue = 0;


EXTREPGL::EXTREPGL() : ParserKeyword("EXTREPGL", KeywordSize(1, false)) {
  addValidSectionName("GRID");
  clearDeckNames();
  addDeckName("EXTREPGL");
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
const std::string EXTREPGL::keywordName = "EXTREPGL";
const std::string EXTREPGL::data::itemName = "data";


}
}
