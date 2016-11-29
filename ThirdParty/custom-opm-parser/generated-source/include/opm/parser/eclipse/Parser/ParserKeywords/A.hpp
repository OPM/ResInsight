#ifndef PARSER_KEYWORDS_A_HPP
#define PARSER_KEYWORDS_A_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class ACTDIMS : public ParserKeyword {
   public:
       ACTDIMS();
       static const std::string keywordName;

       class MAX_ACTION {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ACTION_LINES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ACTION_LINE_CHARACTERS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ACTION_COND {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class ACTNUM : public ParserKeyword {
   public:
       ACTNUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class ADD : public ParserKeyword {
   public:
       ADD();
       static const std::string keywordName;

       class field {
       public:
           static const std::string itemName;
       };

       class shift {
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



   class ADDREG : public ParserKeyword {
   public:
       ADDREG();
       static const std::string keywordName;

       class ARRAY {
       public:
           static const std::string itemName;
       };

       class SHIFT {
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



   class ADSALNOD : public ParserKeyword {
   public:
       ADSALNOD();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class ALL : public ParserKeyword {
   public:
       ALL();
       static const std::string keywordName;
   };



   class API : public ParserKeyword {
   public:
       API();
       static const std::string keywordName;
   };



   class AQUANCON : public ParserKeyword {
   public:
       AQUANCON();
       static const std::string keywordName;

       class AQUIFER_ID {
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

       class FACE {
       public:
           static const std::string itemName;
       };

       class INFLUX_COEFF {
       public:
           static const std::string itemName;
       };

       class INFLUX_MULT {
       public:
           static const std::string itemName;
       };

       class CONNECT_ADJOINING_ACTIVE_CELL {
       public:
           static const std::string itemName;
       };
   };



   class AQUCON : public ParserKeyword {
   public:
       AQUCON();
       static const std::string keywordName;

       class ID {
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

       class CONNECT_FACE {
       public:
           static const std::string itemName;
       };

       class TRANS_MULT {
       public:
           static const std::string itemName;
       };

       class TRANS_OPTION {
       public:
           static const std::string itemName;
       };

       class ALLOW_INTERNAL_CELLS {
       public:
           static const std::string itemName;
       };

       class VEFRAC {
       public:
           static const std::string itemName;
       };

       class VEFRACP {
       public:
           static const std::string itemName;
       };
   };



   class AQUDIMS : public ParserKeyword {
   public:
       AQUDIMS();
       static const std::string keywordName;

       class MXNAQN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXNAQC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NIFTBL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRIFTB {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NANAQU {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NCAMAX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXNALI {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MXAAQL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class AQUFETP : public ParserKeyword {
   public:
       AQUFETP();
       static const std::string keywordName;

       class ID {
       public:
           static const std::string itemName;
       };

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
       };

       class P0 {
       public:
           static const std::string itemName;
       };

       class V0 {
       public:
           static const std::string itemName;
       };

       class COMPRESSIBILITY {
       public:
           static const std::string itemName;
       };

       class PI {
       public:
           static const std::string itemName;
       };

       class WATER_TABLE {
       public:
           static const std::string itemName;
       };

       class SALINITY {
       public:
           static const std::string itemName;
       };

       class TEMP {
       public:
           static const std::string itemName;
       };
   };



   class AQUIFER_PROBE_ANALYTIC : public ParserKeyword {
   public:
       AQUIFER_PROBE_ANALYTIC();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class AQUNUM : public ParserKeyword {
   public:
       AQUNUM();
       static const std::string keywordName;

       class AQUIFER_ID {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
       };

       class J {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };

       class CROSS_SECTION {
       public:
           static const std::string itemName;
       };

       class LENGTH {
       public:
           static const std::string itemName;
       };

       class PORO {
       public:
           static const std::string itemName;
       };

       class PERM {
       public:
           static const std::string itemName;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };

       class INITIAL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class PVT_TABLE_NUM {
       public:
           static const std::string itemName;
       };

       class SAT_TABLE_NUM {
       public:
           static const std::string itemName;
       };
   };



}
}
#endif
