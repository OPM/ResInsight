#ifndef PARSER_KEYWORDS_E_HPP
#define PARSER_KEYWORDS_E_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class ECHO : public ParserKeyword {
   public:
       ECHO();
       static const std::string keywordName;
   };



   class ECLMC : public ParserKeyword {
   public:
       ECLMC();
       static const std::string keywordName;
   };



   class EDIT : public ParserKeyword {
   public:
       EDIT();
       static const std::string keywordName;
   };



   class EDITNNC : public ParserKeyword {
   public:
       EDITNNC();
       static const std::string keywordName;

       class I1 {
       public:
           static const std::string itemName;
       };

       class J1 {
       public:
           static const std::string itemName;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class I2 {
       public:
           static const std::string itemName;
       };

       class J2 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class TRAN_MULT {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE12 {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE21 {
       public:
           static const std::string itemName;
       };

       class PRESS_TABLE12 {
       public:
           static const std::string itemName;
       };

       class PRESS_TABLE21 {
       public:
           static const std::string itemName;
       };

       class FACE_FLOW12 {
       public:
           static const std::string itemName;
       };

       class FACE_FLOW21 {
       public:
           static const std::string itemName;
       };

       class DIFFM {
       public:
           static const std::string itemName;
       };
   };



   class EDITNNCR : public ParserKeyword {
   public:
       EDITNNCR();
       static const std::string keywordName;

       class I1 {
       public:
           static const std::string itemName;
       };

       class J1 {
       public:
           static const std::string itemName;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class I2 {
       public:
           static const std::string itemName;
       };

       class J2 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };

       class TRANS {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE12 {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE21 {
       public:
           static const std::string itemName;
       };

       class PRESS_TABLE12 {
       public:
           static const std::string itemName;
       };

       class PRESS_TABLE21 {
       public:
           static const std::string itemName;
       };

       class FACE_FLOW12 {
       public:
           static const std::string itemName;
       };

       class FACE_FLOW21 {
       public:
           static const std::string itemName;
       };

       class DIFF {
       public:
           static const std::string itemName;
       };
   };



   class EHYSTR : public ParserKeyword {
   public:
       EHYSTR();
       static const std::string keywordName;

       class curvature_caplillary_pressure_hyst {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class relative_perm_hyst {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class curvature_param_killough_wetting {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class mod_param_trapped {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class limiting_hyst_flag {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class shape_cap_press_flag {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class init_fluid_mob_flag {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class wetting_phase_flag {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class baker_flag_oil {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class baker_flag_gas {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class baker_flag_water {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class threshold_saturation {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class EHYSTRR : public ParserKeyword {
   public:
       EHYSTRR();
       static const std::string keywordName;

       class curvature_caplillary_pressure_hyst {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class curvature_parameter_wetting_phase_hyst {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class mod_param_non_wet_phase_sat {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class END : public ParserKeyword {
   public:
       END();
       static const std::string keywordName;
   };



   class ENDACTIO : public ParserKeyword {
   public:
       ENDACTIO();
       static const std::string keywordName;
   };



   class ENDBOX : public ParserKeyword {
   public:
       ENDBOX();
       static const std::string keywordName;
   };



   class ENDDYN : public ParserKeyword {
   public:
       ENDDYN();
       static const std::string keywordName;
   };



   class ENDFIN : public ParserKeyword {
   public:
       ENDFIN();
       static const std::string keywordName;
   };



   class ENDINC : public ParserKeyword {
   public:
       ENDINC();
       static const std::string keywordName;
   };



   class ENDNUM : public ParserKeyword {
   public:
       ENDNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ENDPOINT_SPECIFIERS : public ParserKeyword {
   public:
       ENDPOINT_SPECIFIERS();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ENDSCALE : public ParserKeyword {
   public:
       ENDSCALE();
       static const std::string keywordName;

       class DIRECT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class IRREVERS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NUM_TABLES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NUM_NODES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class COMB_MODE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class ENDSKIP : public ParserKeyword {
   public:
       ENDSKIP();
       static const std::string keywordName;
   };



   class ENKRVD : public ParserKeyword {
   public:
       ENKRVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ENPCVD : public ParserKeyword {
   public:
       ENPCVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ENPTVD : public ParserKeyword {
   public:
       ENPTVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class ENSPCVD : public ParserKeyword {
   public:
       ENSPCVD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class EPSDBGS : public ParserKeyword {
   public:
       EPSDBGS();
       static const std::string keywordName;

       class TABLE_OUTPUT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class CHECK_DRAIN_HYST {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class IX1 {
       public:
           static const std::string itemName;
       };

       class IX2 {
       public:
           static const std::string itemName;
       };

       class JY1 {
       public:
           static const std::string itemName;
       };

       class JY2 {
       public:
           static const std::string itemName;
       };

       class KZ1 {
       public:
           static const std::string itemName;
       };

       class KZ2 {
       public:
           static const std::string itemName;
       };

       class GRID_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class EPSDEBUG : public ParserKeyword {
   public:
       EPSDEBUG();
       static const std::string keywordName;

       class IX1 {
       public:
           static const std::string itemName;
       };

       class IX2 {
       public:
           static const std::string itemName;
       };

       class JY1 {
       public:
           static const std::string itemName;
       };

       class JY2 {
       public:
           static const std::string itemName;
       };

       class KZ1 {
       public:
           static const std::string itemName;
       };

       class KZ2 {
       public:
           static const std::string itemName;
       };

       class TABLE_OUTPUT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class GRID_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHECK_DRAIN_HYST {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class EQLDIMS : public ParserKeyword {
   public:
       EQLDIMS();
       static const std::string keywordName;

       class NTEQUL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DEPTH_NODES_P {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DEPTH_NODES_TAB {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NTTRVD {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NSTRVD {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class EQLNUM : public ParserKeyword {
   public:
       EQLNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class EQLOPTS : public ParserKeyword {
   public:
       EQLOPTS();
       static const std::string keywordName;

       class OPTION1 {
       public:
           static const std::string itemName;
       };

       class OPTION2 {
       public:
           static const std::string itemName;
       };

       class OPTION3 {
       public:
           static const std::string itemName;
       };

       class OPTION4 {
       public:
           static const std::string itemName;
       };
   };



   class EQLZCORN : public ParserKeyword {
   public:
       EQLZCORN();
       static const std::string keywordName;

       class VALUE_ZCORN_ARRAY {
       public:
           static const std::string itemName;
       };

       class IX1 {
       public:
           static const std::string itemName;
       };

       class IX2 {
       public:
           static const std::string itemName;
       };

       class JY1 {
       public:
           static const std::string itemName;
       };

       class JY2 {
       public:
           static const std::string itemName;
       };

       class KZ1 {
       public:
           static const std::string itemName;
       };

       class KZ2 {
       public:
           static const std::string itemName;
       };

       class IX1A {
       public:
           static const std::string itemName;
       };

       class IX2A {
       public:
           static const std::string itemName;
       };

       class JY1A {
       public:
           static const std::string itemName;
       };

       class JY2A {
       public:
           static const std::string itemName;
       };

       class ACTION_REQ {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class EQUALREG : public ParserKeyword {
   public:
       EQUALREG();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class REGION_NUMBER {
       public:
           static const std::string itemName;
       };

       class REGION_NAME {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class EQUALS : public ParserKeyword {
   public:
       EQUALS();
       static const std::string keywordName;

       class field {
       public:
           static const std::string itemName;
       };

       class value {
       public:
           static const std::string itemName;
       };

       class I1 {
       public:
           static const std::string itemName;
       };

       class I2 {
       public:
           static const std::string itemName;
       };

       class J1 {
       public:
           static const std::string itemName;
       };

       class J2 {
       public:
           static const std::string itemName;
       };

       class K1 {
       public:
           static const std::string itemName;
       };

       class K2 {
       public:
           static const std::string itemName;
       };
   };



   class EQUIL : public ParserKeyword {
   public:
       EQUIL();
       static const std::string keywordName;

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DATUM_PRESSURE {
       public:
           static const std::string itemName;
       };

       class OWC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PC_OWC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GOC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PC_GOC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class BLACK_OIL_INIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class BLACK_OIL_INIT_WG {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class OIP_INIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class ESSNODE : public ParserKeyword {
   public:
       ESSNODE();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class EXCAVATE : public ParserKeyword {
   public:
       EXCAVATE();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class EXCEL : public ParserKeyword {
   public:
       EXCEL();
       static const std::string keywordName;
   };



   class EXIT : public ParserKeyword {
   public:
       EXIT();
       static const std::string keywordName;

       class STATUS_CODE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class EXTFIN : public ParserKeyword {
   public:
       EXTFIN();
       static const std::string keywordName;

       class LOCAL_GRID_REF {
       public:
           static const std::string itemName;
       };

       class NX {
       public:
           static const std::string itemName;
       };

       class NY {
       public:
           static const std::string itemName;
       };

       class NZ {
       public:
           static const std::string itemName;
       };

       class NREPG {
       public:
           static const std::string itemName;
       };

       class NHALO {
       public:
           static const std::string itemName;
       };

       class NFLOG {
       public:
           static const std::string itemName;
       };

       class NUMINT {
       public:
           static const std::string itemName;
       };

       class NUMCON {
       public:
           static const std::string itemName;
       };

       class NWMAX {
       public:
           static const std::string itemName;
       };
   };



   class EXTHOST : public ParserKeyword {
   public:
       EXTHOST();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class EXTRAPMS : public ParserKeyword {
   public:
       EXTRAPMS();
       static const std::string keywordName;

       class LEVEL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class EXTREPGL : public ParserKeyword {
   public:
       EXTREPGL();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
