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
   };



   class METRIC : public ParserKeyword {
   public:
       METRIC();
       static const std::string keywordName;
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



   class MONITOR : public ParserKeyword {
   public:
       MONITOR();
       static const std::string keywordName;
   };



   class MSFN : public ParserKeyword {
   public:
       MSFN();
       static const std::string keywordName;

       class table {
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



   class MULTPV : public ParserKeyword {
   public:
       MULTPV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
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
