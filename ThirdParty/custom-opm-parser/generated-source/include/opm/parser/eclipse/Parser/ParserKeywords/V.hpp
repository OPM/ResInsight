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



}
}
#endif
