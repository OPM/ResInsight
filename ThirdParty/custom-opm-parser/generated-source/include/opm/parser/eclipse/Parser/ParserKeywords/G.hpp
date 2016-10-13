#ifndef PARSER_KEYWORDS_G_HPP
#define PARSER_KEYWORDS_G_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class GAS : public ParserKeyword {
   public:
       GAS();
       static const std::string keywordName;
   };



   class GASVISCT : public ParserKeyword {
   public:
       GASVISCT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class GCOMPIDX : public ParserKeyword {
   public:
       GCOMPIDX();
       static const std::string keywordName;

       class GAS_COMPONENT_INDEX {
       public:
           static const std::string itemName;
       };
   };



   class GCONINJE : public ParserKeyword {
   public:
       GCONINJE();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class CONTROL_MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SURFACE_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class RESV_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class REINJ_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class VOIDAGE_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class FREE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GUIDE_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GUIDE_DEF {
       public:
           static const std::string itemName;
       };

       class REINJECT_GROUP {
       public:
           static const std::string itemName;
       };

       class VOIDAGE_GROUP {
       public:
           static const std::string itemName;
       };

       class WETGAS_TARGET {
       public:
           static const std::string itemName;
       };
   };



   class GCONPROD : public ParserKeyword {
   public:
       GCONPROD();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class CONTROL_MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OIL_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class LIQUID_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXCEED_PROC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RESPOND_TO_PARENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GUIDE_RATE {
       public:
           static const std::string itemName;
       };

       class GUIDE_RATE_DEF {
       public:
           static const std::string itemName;
       };

       class WATER_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
       };

       class GAS_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
       };

       class LIQUID_EXCEED_PROCEDURE {
       public:
           static const std::string itemName;
       };

       class RESERVOIR_FLUID_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class RESERVOIR_VOLUME_BALANCE {
       public:
           static const std::string itemName;
       };
   };



   class GDORIENT : public ParserKeyword {
   public:
       GDORIENT();
       static const std::string keywordName;

       class I {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class K {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class Z {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class HAND {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GECON : public ParserKeyword {
   public:
       GECON();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class MIN_OIL_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_GAS_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_WCT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_GOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_WATER_GAS_RATIO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WORKOVER {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class END_RUN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_OPEN_WELLS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class GEFAC : public ParserKeyword {
   public:
       GEFAC();
       static const std::string keywordName;

       class GROUP {
       public:
           static const std::string itemName;
       };

       class EFFICIENCY_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRANSFER_EXT_NET {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRAVITY : public ParserKeyword {
   public:
       GRAVITY();
       static const std::string keywordName;

       class API_GRAVITY {
       public:
           static const std::string itemName;
       };

       class WATER_SP_GRAVITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_SP_GRAVITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class GRID : public ParserKeyword {
   public:
       GRID();
       static const std::string keywordName;
   };



   class GRIDFILE : public ParserKeyword {
   public:
       GRIDFILE();
       static const std::string keywordName;

       class GRID {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class EGRID {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class GRIDOPTS : public ParserKeyword {
   public:
       GRIDOPTS();
       static const std::string keywordName;

       class TRANMULT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class NRMULT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NRPINC {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class GRIDUNIT : public ParserKeyword {
   public:
       GRIDUNIT();
       static const std::string keywordName;

       class LengthUnit {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAP {
       public:
           static const std::string itemName;
       };
   };



   class GROUP_PROBE : public ParserKeyword {
   public:
       GROUP_PROBE();
       static const std::string keywordName;

       class GROUPS {
       public:
           static const std::string itemName;
       };
   };



   class GRUPNET : public ParserKeyword {
   public:
       GRUPNET();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class TERMINAL_PRESSURE {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class ALQ {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SUB_SEA_MANIFOLD {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIFT_GAS_FLOW_THROUGH {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class ALQ_SURFACE_EQV {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class GRUPTREE : public ParserKeyword {
   public:
       GRUPTREE();
       static const std::string keywordName;

       class CHILD_GROUP {
       public:
           static const std::string itemName;
       };

       class PARENT_GROUP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



}
}
#endif
