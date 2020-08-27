#ifndef PARSER_KEYWORDS_M_HPP
#define PARSER_KEYWORDS_M_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class MAPAXES : public ParserKeyword {
   public:
       MAPAXES();
       static const std::string keywordName;

       class X1 {
       public:
           static const std::string itemName;
       };

       class Y1 {
       public:
           static const std::string itemName;
       };

       class X2 {
       public:
           static const std::string itemName;
       };

       class Y2 {
       public:
           static const std::string itemName;
       };

       class X3 {
       public:
           static const std::string itemName;
       };

       class Y3 {
       public:
           static const std::string itemName;
       };
   };



   class MAPUNITS : public ParserKeyword {
   public:
       MAPUNITS();
       static const std::string keywordName;

       class UNIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MASSFLOW : public ParserKeyword {
   public:
       MASSFLOW();
       static const std::string keywordName;

       class WORD {
       public:
           static const std::string itemName;
       };
   };



   class MATCORR : public ParserKeyword {
   public:
       MATCORR();
       static const std::string keywordName;

       class NEWTON_IT_NUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NON_LIN_CONV_ERR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MATERIAL_BALANCE_ERR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MAXVALUE : public ParserKeyword {
   public:
       MAXVALUE();
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



   class MEMORY : public ParserKeyword {
   public:
       MEMORY();
       static const std::string keywordName;

       class UNUSED {
       public:
           static const std::string itemName;
       };

       class THOUSANDS_CHAR8 {
       public:
           static const std::string itemName;
       };
   };



   class MESSAGE : public ParserKeyword {
   public:
       MESSAGE();
       static const std::string keywordName;

       class MessageText {
       public:
           static const std::string itemName;
       };
   };



   class MESSAGES : public ParserKeyword {
   public:
       MESSAGES();
       static const std::string keywordName;

       class MESSAGE_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class COMMENT_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class WARNING_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class PROBLEM_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ERROR_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class BUG_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MESSAGE_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class COMMENT_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class WARNING_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class PROBLEM_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ERROR_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class BUG_STOP_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class GROUP_PRINT_LIMIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class MESSOPTS : public ParserKeyword {
   public:
       MESSOPTS();
       static const std::string keywordName;

       class MNEMONIC {
       public:
           static const std::string itemName;
       };

       class SEVERITY {
       public:
           static const std::string itemName;
       };
   };



   class MESSSRVC : public ParserKeyword {
   public:
       MESSSRVC();
       static const std::string keywordName;

       class PRODUCE_MESSAGE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class METRIC : public ParserKeyword {
   public:
       METRIC();
       static const std::string keywordName;
   };



   class MINNNCT : public ParserKeyword {
   public:
       MINNNCT();
       static const std::string keywordName;

       class CUTOFF_TRANSMISSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DIFFUSIVITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CUTOFF_THERMAL_TRANSMISSIBILITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MINPORV : public ParserKeyword {
   public:
       MINPORV();
       static const std::string keywordName;

       class MIN_PORE_VOL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MINPV : public ParserKeyword {
   public:
       MINPV();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MINPVFIL : public ParserKeyword {
   public:
       MINPVFIL();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MINPVV : public ParserKeyword {
   public:
       MINPVV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class MINVALUE : public ParserKeyword {
   public:
       MINVALUE();
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



   class MISC : public ParserKeyword {
   public:
       MISC();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class MISCIBLE : public ParserKeyword {
   public:
       MISCIBLE();
       static const std::string keywordName;

       class NTMISC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NSMISC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class TWOPOINT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MISCNUM : public ParserKeyword {
   public:
       MISCNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MLANG : public ParserKeyword {
   public:
       MLANG();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MLANGSLV : public ParserKeyword {
   public:
       MLANGSLV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MONITOR : public ParserKeyword {
   public:
       MONITOR();
       static const std::string keywordName;
   };



   class MPFANUM : public ParserKeyword {
   public:
       MPFANUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MPFNNC : public ParserKeyword {
   public:
       MPFNNC();
       static const std::string keywordName;

       class IX {
       public:
           static const std::string itemName;
       };

       class IY {
       public:
           static const std::string itemName;
       };

       class IZ {
       public:
           static const std::string itemName;
       };

       class JX {
       public:
           static const std::string itemName;
       };

       class JY {
       public:
           static const std::string itemName;
       };

       class JZ {
       public:
           static const std::string itemName;
       };

       class TRANP {
       public:
           static const std::string itemName;
       };

       class KX {
       public:
           static const std::string itemName;
       };

       class KY {
       public:
           static const std::string itemName;
       };

       class KZ {
       public:
           static const std::string itemName;
       };

       class TRANS {
       public:
           static const std::string itemName;
       };
   };



   class MSFN : public ParserKeyword {
   public:
       MSFN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class MSGFILE : public ParserKeyword {
   public:
       MSGFILE();
       static const std::string keywordName;

       class ENABLE_FLAG {
       public:
           static const std::string itemName;
       };
   };



   class MULSGGD : public ParserKeyword {
   public:
       MULSGGD();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MULSGGDV : public ParserKeyword {
   public:
       MULSGGDV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MULTFLT : public ParserKeyword {
   public:
       MULTFLT();
       static const std::string keywordName;

       class fault {
       public:
           static const std::string itemName;
       };

       class factor {
       public:
           static const std::string itemName;
       };
   };



   class MULTIN : public ParserKeyword {
   public:
       MULTIN();
       static const std::string keywordName;
   };



   class MULTIPLY : public ParserKeyword {
   public:
       MULTIPLY();
       static const std::string keywordName;

       class field {
       public:
           static const std::string itemName;
       };

       class factor {
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



   class MULTIREG : public ParserKeyword {
   public:
       MULTIREG();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class FACTOR {
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



   class MULTNUM : public ParserKeyword {
   public:
       MULTNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MULTOUT : public ParserKeyword {
   public:
       MULTOUT();
       static const std::string keywordName;
   };



   class MULTOUTS : public ParserKeyword {
   public:
       MULTOUTS();
       static const std::string keywordName;
   };



   class MULTPV : public ParserKeyword {
   public:
       MULTPV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MULTREAL : public ParserKeyword {
   public:
       MULTREAL();
       static const std::string keywordName;

       class SESSION_SPEC {
       public:
           static const std::string itemName;
       };

       class STANDARD_LICENCE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MULTREGD : public ParserKeyword {
   public:
       MULTREGD();
       static const std::string keywordName;

       class FROM_REGION {
       public:
           static const std::string itemName;
       };

       class TO_REGION {
       public:
           static const std::string itemName;
       };

       class MULTIPLIER {
       public:
           static const std::string itemName;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHOICE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MULTREGH : public ParserKeyword {
   public:
       MULTREGH();
       static const std::string keywordName;

       class FROM_REGION {
       public:
           static const std::string itemName;
       };

       class TO_REGION {
       public:
           static const std::string itemName;
       };

       class MULTIPLIER {
       public:
           static const std::string itemName;
       };

       class DIRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CHOICE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MULTREGP : public ParserKeyword {
   public:
       MULTREGP();
       static const std::string keywordName;

       class REGION {
       public:
           static const std::string itemName;
       };

       class MULTIPLIER {
       public:
           static const std::string itemName;
       };

       class REGION_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MULTREGT : public ParserKeyword {
   public:
       MULTREGT();
       static const std::string keywordName;

       class SRC_REGION {
       public:
           static const std::string itemName;
       };

       class TARGET_REGION {
       public:
           static const std::string itemName;
       };

       class TRAN_MULT {
       public:
           static const std::string itemName;
       };

       class DIRECTIONS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NNC_MULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REGION_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class MULTSIG : public ParserKeyword {
   public:
       MULTSIG();
       static const std::string keywordName;

       class VALUE {
       public:
           static const std::string itemName;
       };
   };



   class MULTSIGV : public ParserKeyword {
   public:
       MULTSIGV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MULT_XYZ : public ParserKeyword {
   public:
       MULT_XYZ();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class MW : public ParserKeyword {
   public:
       MW();
       static const std::string keywordName;

       class MOLAR_WEIGHT {
       public:
           static const std::string itemName;
       };
   };



   class MWS : public ParserKeyword {
   public:
       MWS();
       static const std::string keywordName;

       class MOLAR_WEIGHT {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
