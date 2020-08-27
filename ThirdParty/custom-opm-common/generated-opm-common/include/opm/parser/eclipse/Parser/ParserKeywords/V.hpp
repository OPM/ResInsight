#ifndef PARSER_KEYWORDS_V_HPP
#define PARSER_KEYWORDS_V_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class VAPOIL : public ParserKeyword {
   public:
       VAPOIL();
       static const std::string keywordName;
   };



   class VAPPARS : public ParserKeyword {
   public:
       VAPPARS();
       static const std::string keywordName;

       class OIL_VAP_PROPENSITY {
       public:
           static const std::string itemName;
       };

       class OIL_DENSITY_PROPENSITY {
       public:
           static const std::string itemName;
       };
   };



   class VAPWAT : public ParserKeyword {
   public:
       VAPWAT();
       static const std::string keywordName;
   };



   class VDFLOW : public ParserKeyword {
   public:
       VDFLOW();
       static const std::string keywordName;

       class BETA {
       public:
           static const std::string itemName;
       };
   };



   class VDFLOWR : public ParserKeyword {
   public:
       VDFLOWR();
       static const std::string keywordName;

       class BETA {
       public:
           static const std::string itemName;
       };
   };



   class VE : public ParserKeyword {
   public:
       VE();
       static const std::string keywordName;

       class MODEL_TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class VEDEBUG : public ParserKeyword {
   public:
       VEDEBUG();
       static const std::string keywordName;

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

       class DEBUG_LEVEL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LGR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class VEFIN : public ParserKeyword {
   public:
       VEFIN();
       static const std::string keywordName;

       class VE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NVEPT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class VEFRAC : public ParserKeyword {
   public:
       VEFRAC();
       static const std::string keywordName;

       class FRAC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class VEFRACP : public ParserKeyword {
   public:
       VEFRACP();
       static const std::string keywordName;

       class FRAC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class VEFRACPV : public ParserKeyword {
   public:
       VEFRACPV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class VEFRACV : public ParserKeyword {
   public:
       VEFRACV();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class VFPCHK : public ParserKeyword {
   public:
       VFPCHK();
       static const std::string keywordName;

       class BHP_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class VFPIDIMS : public ParserKeyword {
   public:
       VFPIDIMS();
       static const std::string keywordName;

       class MAX_FLOW_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_THP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_INJ_VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class VFPINJ : public ParserKeyword {
   public:
       VFPINJ();
       static const std::string keywordName;

       class TABLE {
       public:
           static const std::string itemName;
       };

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
       };

       class RATE_TYPE {
       public:
           static const std::string itemName;
       };

       class PRESSURE_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class UNITS {
       public:
           static const std::string itemName;
       };

       class BODY_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLOW_VALUES {
       public:
           static const std::string itemName;
       };

       class THP_VALUES {
       public:
           static const std::string itemName;
       };

       class THP_INDEX {
       public:
           static const std::string itemName;
       };

       class VALUES {
       public:
           static const std::string itemName;
       };
   };



   class VFPPDIMS : public ParserKeyword {
   public:
       VFPPDIMS();
       static const std::string keywordName;

       class MAX_FLOW_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_THP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WCT_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_GCT_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ALQ_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_PROD_VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class VFPPROD : public ParserKeyword {
   public:
       VFPPROD();
       static const std::string keywordName;

       class TABLE {
       public:
           static const std::string itemName;
       };

       class DATUM_DEPTH {
       public:
           static const std::string itemName;
       };

       class RATE_TYPE {
       public:
           static const std::string itemName;
       };

       class WFR {
       public:
           static const std::string itemName;
       };

       class GFR {
       public:
           static const std::string itemName;
       };

       class PRESSURE_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ALQ_DEF {
       public:
           static const std::string itemName;
       };

       class UNITS {
       public:
           static const std::string itemName;
       };

       class BODY_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLOW_VALUES {
       public:
           static const std::string itemName;
       };

       class THP_VALUES {
       public:
           static const std::string itemName;
       };

       class WFR_VALUES {
       public:
           static const std::string itemName;
       };

       class GFR_VALUES {
       public:
           static const std::string itemName;
       };

       class ALQ_VALUES {
       public:
           static const std::string itemName;
       };

       class THP_INDEX {
       public:
           static const std::string itemName;
       };

       class WFR_INDEX {
       public:
           static const std::string itemName;
       };

       class GFR_INDEX {
       public:
           static const std::string itemName;
       };

       class ALQ_INDEX {
       public:
           static const std::string itemName;
       };

       class VALUES {
       public:
           static const std::string itemName;
       };
   };



   class VFPTABL : public ParserKeyword {
   public:
       VFPTABL();
       static const std::string keywordName;

       class METHOD {
       public:
           static const std::string itemName;
       };
   };



   class VISAGE : public ParserKeyword {
   public:
       VISAGE();
       static const std::string keywordName;
   };



   class VISCD : public ParserKeyword {
   public:
       VISCD();
       static const std::string keywordName;
   };



   class VISCREF : public ParserKeyword {
   public:
       VISCREF();
       static const std::string keywordName;

       class REFERENCE_PRESSURE {
       public:
           static const std::string itemName;
       };

       class REFERENCE_RS {
       public:
           static const std::string itemName;
       };
   };



   class VISDATES : public ParserKeyword {
   public:
       VISDATES();
       static const std::string keywordName;

       class DAY {
       public:
           static const std::string itemName;
       };

       class MONTH {
       public:
           static const std::string itemName;
       };

       class YEAR {
       public:
           static const std::string itemName;
       };

       class TIMESTAMP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class VISOPTS : public ParserKeyword {
   public:
       VISOPTS();
       static const std::string keywordName;

       class INIT_RUN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class EXIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ACTIVE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class REL_TOL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class UNUSED {
       public:
           static const std::string itemName;
       };

       class RETAIN_RESTART_FREQUENCY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RETAIN_RESTART_CONTENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ERROR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



}
}
#endif
