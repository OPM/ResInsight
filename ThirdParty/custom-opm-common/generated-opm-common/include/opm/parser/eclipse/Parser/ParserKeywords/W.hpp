#ifndef PARSER_KEYWORDS_W_HPP
#define PARSER_KEYWORDS_W_HPP
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
namespace Opm {
namespace ParserKeywords {

   class WAGHYSTR : public ParserKeyword {
   public:
       WAGHYSTR();
       static const std::string keywordName;

       class LANDS_PARAMETER {
       public:
           static const std::string itemName;
       };

       class SECONDARY_DRAINAGE_REDUCTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RES_OIL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WATER_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class IMB_LINEAR_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THREEPHASE_SAT_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class RES_OIL_MOD_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WAITBAL : public ParserKeyword {
   public:
       WAITBAL();
       static const std::string keywordName;

       class WAIT_NETWORK_BALANCE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WALKALIN : public ParserKeyword {
   public:
       WALKALIN();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class ALKALINE_CONCENTRAION {
       public:
           static const std::string itemName;
       };
   };



   class WALQCALC : public ParserKeyword {
   public:
       WALQCALC();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class ALQ_DEF {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WAPI : public ParserKeyword {
   public:
       WAPI();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class API {
       public:
           static const std::string itemName;
       };
   };



   class WARN : public ParserKeyword {
   public:
       WARN();
       static const std::string keywordName;
   };



   class WATDENT : public ParserKeyword {
   public:
       WATDENT();
       static const std::string keywordName;

       class REFERENCE_TEMPERATURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXPANSION_COEFF_LINEAR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXPANSION_COEFF_QUADRATIC {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WATER : public ParserKeyword {
   public:
       WATER();
       static const std::string keywordName;
   };



   class WATVISCT : public ParserKeyword {
   public:
       WATVISCT();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class WBHGLR : public ParserKeyword {
   public:
       WBHGLR();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MAX_GLR_CUTBACK {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_GLR_CUTBACK_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class RATE_CUTBACK_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_GLR_ELIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WORKOVER_ACTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WORKOVER_REMOVE_CUTBACKS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WBOREVOL : public ParserKeyword {
   public:
       WBOREVOL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WELLBORE_VOL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class START_BHP {
       public:
           static const std::string itemName;
       };
   };



   class WCALCVAL : public ParserKeyword {
   public:
       WCALCVAL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CALORIFIC_GAS_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class WCONHIST : public ParserKeyword {
   public:
       WCONHIST();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CMODE {
       public:
           static const std::string itemName;
       };

       class ORAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class WRAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class GRAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class LIFT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THP {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class BHP {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class NGLRAT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WCONINJ : public ParserKeyword {
   public:
       WCONINJ();
       static const std::string keywordName;
   };



   class WCONINJE : public ParserKeyword {
   public:
       WCONINJE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TYPE {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CMODE {
       public:
           static const std::string itemName;
       };

       class RATE {
       public:
           static const std::string itemName;
       };

       class RESV {
       public:
           static const std::string itemName;
       };

       class BHP {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class THP {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VAPOIL_C {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_STEAM_RATIO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_OIL_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_WATER_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_GAS_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OIL_STEAM_RATIO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WCONINJH : public ParserKeyword {
   public:
       WCONINJH();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TYPE {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class RATE {
       public:
           static const std::string itemName;
       };

       class BHP {
       public:
           static const std::string itemName;
       };

       class THP {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VAPOIL_C {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_OIL_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_WATER_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SURFACE_GAS_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CMODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WCONINJP : public ParserKeyword {
   public:
       WCONINJP();
       static const std::string keywordName;

       class PATTERN_WELL {
       public:
           static const std::string itemName;
       };

       class INJECTOR_TYPE {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class BHP_MAX {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class THP_MAX {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class VOIDAGE_TARGET_MULTIPLIER {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OIL_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WELL {
       public:
           static const std::string itemName;
       };

       class PROD_FRACTION {
       public:
           static const std::string itemName;
       };

       class FIPNUM_VALUE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WCONPROD : public ParserKeyword {
   public:
       WCONPROD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CMODE {
       public:
           static const std::string itemName;
       };

       class ORAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class WRAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class GRAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class LRAT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class RESV {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class BHP {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class THP {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
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

       class E300_ITEM13 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM14 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM15 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM16 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM17 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM18 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM19 {
       public:
           static const std::string itemName;
       };

       class E300_ITEM20 {
       public:
           static const std::string itemName;
       };
   };



   class WCUTBACK : public ParserKeyword {
   public:
       WCUTBACK();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WCT_LIMIT {
       public:
           static const std::string itemName;
       };

       class GOR_LIMIT {
       public:
           static const std::string itemName;
       };

       class WGR_LIMIT {
       public:
           static const std::string itemName;
       };

       class RATE_CUTBACK {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class PRESSURE_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRESSURE_LIMIT_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WCT_LIMIT_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GOR_LIMIT_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GLR_LIMIT_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WGR_LIMIT_REVERSE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WORKOVER_REMOVE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WCUTBACT : public ParserKeyword {
   public:
       WCUTBACT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class RATE_CUTBACK {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class WORKOVER_REMOVE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WCYCLE : public ParserKeyword {
   public:
       WCYCLE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class ON_TIME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OFF_TIME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class START_TIME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_TIMESTEP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CONTROLLED_TIMESTEP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WDFAC : public ParserKeyword {
   public:
       WDFAC();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DFACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WDFACCOR : public ParserKeyword {
   public:
       WDFACCOR();
       static const std::string keywordName;

       class WELLNAME {
       public:
           static const std::string itemName;
       };

       class A {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class B {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class C {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WDRILPRI : public ParserKeyword {
   public:
       WDRILPRI();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class PRIORITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DRILLING_UNIT {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WDRILRES : public ParserKeyword {
   public:
       WDRILRES();
       static const std::string keywordName;
   };



   class WDRILTIM : public ParserKeyword {
   public:
       WDRILTIM();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DRILL_TIME {
       public:
           static const std::string itemName;
       };

       class WORKOVER_CLOSE {
       public:
           static const std::string itemName;
       };

       class COMPARTMENT {
       public:
           static const std::string itemName;
       };
   };



   class WECON : public ParserKeyword {
   public:
       WECON();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MIN_OIL_PRODUCTION {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MIN_GAS_PRODUCTION {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_WATER_CUT {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_GAS_OIL_RATIO {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class MAX_WATER_GAS_RATIO {
       public:
           static const std::string itemName;
           static const UDAValue defaultValue;
       };

       class WORKOVER_RATIO_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class END_RUN_FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FOLLOW_ON_WELL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class LIMITED_QUANTITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class SECOND_MAX_WATER_CUT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WORKOVER_SECOND_WATER_CUT_LIMIT {
       public:
           static const std::string itemName;
       };

       class MAX_GAS_LIQUID_RATIO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_LIQUID_PRODCUTION_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_TEMP {
       public:
           static const std::string itemName;
       };

       class MIN_RES_FLUID_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WECONINJ : public ParserKeyword {
   public:
       WECONINJ();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MIN_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WECONT : public ParserKeyword {
   public:
       WECONT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WORKOVER {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class END_RUN_FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FOLLOW_ON_WELL {
       public:
           static const std::string itemName;
       };
   };



   class WEFAC : public ParserKeyword {
   public:
       WEFAC();
       static const std::string keywordName;

       class WELLNAME {
       public:
           static const std::string itemName;
       };

       class EFFICIENCY_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class EXTENDED_NETWORK_OPT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WELCNTL : public ParserKeyword {
   public:
       WELCNTL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CMODE {
       public:
           static const std::string itemName;
       };

       class NEW_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class WELDEBUG : public ParserKeyword {
   public:
       WELDEBUG();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DEBUG_FLAG {
       public:
           static const std::string itemName;
       };
   };



   class WELDRAW : public ParserKeyword {
   public:
       WELDRAW();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MAX_DRAW {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class USE_LIMIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GRID_BLOCKS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WELEVNT : public ParserKeyword {
   public:
       WELEVNT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WPWEM_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class WELLDIMS : public ParserKeyword {
   public:
       WELLDIMS();
       static const std::string keywordName;

       class MAXWELLS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAXCONN {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAXGROUPS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_GROUPSIZE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_STAGES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_STREAMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_MIXTURES {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_SEPARATORS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_MIXTURE_ITEMS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_COMPLETION_X {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_WELLIST_PR_WELL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_DYNAMIC_WELLIST {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_SECONDARY_WELLS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WELL_COMPLETION_PROBE : public ParserKeyword {
   public:
       WELL_COMPLETION_PROBE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class COMPLETION {
       public:
           static const std::string itemName;
       };
   };



   class WELL_PROBE : public ParserKeyword {
   public:
       WELL_PROBE();
       static const std::string keywordName;

       class WELLS {
       public:
           static const std::string itemName;
       };
   };



   class WELMOVEL : public ParserKeyword {
   public:
       WELMOVEL();
       static const std::string keywordName;

       class WELLNAME {
       public:
           static const std::string itemName;
       };

       class LGRNAME {
       public:
           static const std::string itemName;
       };

       class WELLHEAD_I {
       public:
           static const std::string itemName;
       };

       class WELLHEAD_J {
       public:
           static const std::string itemName;
       };
   };



   class WELOPEN : public ParserKeyword {
   public:
       WELOPEN();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
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

       class C1 {
       public:
           static const std::string itemName;
       };

       class C2 {
       public:
           static const std::string itemName;
       };
   };



   class WELOPENL : public ParserKeyword {
   public:
       WELOPENL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GRID {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
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

       class C1 {
       public:
           static const std::string itemName;
       };

       class C2 {
       public:
           static const std::string itemName;
       };
   };



   class WELPI : public ParserKeyword {
   public:
       WELPI();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class STEADY_STATE_PRODUCTIVITY_OR_INJECTIVITY_INDEX_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class WELPRI : public ParserKeyword {
   public:
       WELPRI();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class PRI1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class SCALING1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRI2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class SCALING2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WELSEGS : public ParserKeyword {
   public:
       WELSEGS();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };

       class LENGTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WELLBORE_VOLUME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class INFO_TYPE {
       public:
           static const std::string itemName;
       };

       class PRESSURE_COMPONENTS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLOW_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TOP_X {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TOP_Y {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class BRANCH {
       public:
           static const std::string itemName;
       };

       class JOIN_SEGMENT {
       public:
           static const std::string itemName;
       };

       class SEGMENT_LENGTH {
       public:
           static const std::string itemName;
       };

       class DEPTH_CHANGE {
       public:
           static const std::string itemName;
       };

       class DIAMETER {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class AREA {
       public:
           static const std::string itemName;
       };

       class VOLUME {
       public:
           static const std::string itemName;
       };

       class LENGTH_X {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class LENGTH_Y {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WELSOMIN : public ParserKeyword {
   public:
       WELSOMIN();
       static const std::string keywordName;

       class SOMIN {
       public:
           static const std::string itemName;
       };
   };



   class WELSPECL : public ParserKeyword {
   public:
       WELSPECL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GROUP {
       public:
           static const std::string itemName;
       };

       class LGR {
       public:
           static const std::string itemName;
       };

       class HEAD_I {
       public:
           static const std::string itemName;
       };

       class HEAD_J {
       public:
           static const std::string itemName;
       };

       class REF_DEPTH {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class D_RADIUS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class INFLOW_EQ {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class AUTO_SHUTIN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CROSSFLOW {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class P_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DENSITY_CALC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FIP_REGION {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class FRONTSIM1 {
       public:
           static const std::string itemName;
       };

       class FRONTSIM2 {
       public:
           static const std::string itemName;
       };

       class well_model {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class POLYMER_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WELSPECS : public ParserKeyword {
   public:
       WELSPECS();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GROUP {
       public:
           static const std::string itemName;
       };

       class HEAD_I {
       public:
           static const std::string itemName;
       };

       class HEAD_J {
       public:
           static const std::string itemName;
       };

       class REF_DEPTH {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class D_RADIUS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class INFLOW_EQ {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class AUTO_SHUTIN {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CROSSFLOW {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class P_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DENSITY_CALC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FIP_REGION {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class FRONTSIM1 {
       public:
           static const std::string itemName;
       };

       class FRONTSIM2 {
       public:
           static const std::string itemName;
       };

       class well_model {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class POLYMER_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WELTARG : public ParserKeyword {
   public:
       WELTARG();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CMODE {
       public:
           static const std::string itemName;
       };

       class NEW_VALUE {
       public:
           static const std::string itemName;
       };
   };



   class WFOAM : public ParserKeyword {
   public:
       WFOAM();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class FOAM_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class WFRICSEG : public ParserKeyword {
   public:
       WFRICSEG();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TUBINGD {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class FLOW_SCALING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WFRICSGL : public ParserKeyword {
   public:
       WFRICSGL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TUBINGD {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class FLOW_SCALING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WFRICTN : public ParserKeyword {
   public:
       WFRICTN();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TUBINGD {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class FLOW_SCALING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WFRICTNL : public ParserKeyword {
   public:
       WFRICTNL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TUBINGD {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class FLOW_SCALING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WGASPROD : public ParserKeyword {
   public:
       WGASPROD();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class INCREMENTAL_GAS_PRODUCTION_RATE {
       public:
           static const std::string itemName;
       };

       class MAX_INCREMENTS {
       public:
           static const std::string itemName;
       };
   };



   class WGORPEN : public ParserKeyword {
   public:
       WGORPEN();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class BASE_GOR {
       public:
           static const std::string itemName;
       };

       class MAX_OIL_RATE {
       public:
           static const std::string itemName;
       };

       class AVG_GOR0 {
       public:
           static const std::string itemName;
       };
   };



   class WGRUPCON : public ParserKeyword {
   public:
       WGRUPCON();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class GROUP_CONTROLLED {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GUIDE_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PHASE {
       public:
           static const std::string itemName;
       };

       class SCALING_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WH2NUM : public ParserKeyword {
   public:
       WH2NUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class WH3NUM : public ParserKeyword {
   public:
       WH3NUM();
       static const std::string keywordName;

       class data {
       public:
           static const std::string itemName;
       };
   };



   class WHEDREFD : public ParserKeyword {
   public:
       WHEDREFD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DEPTH {
       public:
           static const std::string itemName;
       };
   };



   class WHISTCTL : public ParserKeyword {
   public:
       WHISTCTL();
       static const std::string keywordName;

       class CMODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class BPH_TERMINATE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WHTEMP : public ParserKeyword {
   public:
       WHTEMP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class VFP_TABLE {
       public:
           static const std::string itemName;
       };

       class THP_TEMP {
       public:
           static const std::string itemName;
       };
   };



   class WINJMULT : public ParserKeyword {
   public:
       WINJMULT();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class FRACTURING_PRESSURE {
       public:
           static const std::string itemName;
       };

       class MULTIPLIER_GRADIENT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MODE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class I {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WINJTEMP : public ParserKeyword {
   public:
       WINJTEMP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class STEAM_QUALITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TEMPERATURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PRESSURE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ENTHALPY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WLIFT : public ParserKeyword {
   public:
       WLIFT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TRIGGER_LIMIT {
       public:
           static const std::string itemName;
       };

       class TRIGGRE_PHASE {
       public:
           static const std::string itemName;
       };

       class NEW_VFP_TABLE {
       public:
           static const std::string itemName;
       };

       class NEW_ALQ_VALUE {
       public:
           static const std::string itemName;
       };

       class NEW_WEFAC {
       public:
           static const std::string itemName;
       };

       class WWCT_LIMIT {
       public:
           static const std::string itemName;
       };

       class NEW_THP_LIMIT {
       public:
           static const std::string itemName;
       };

       class WGOR_LIMIT {
       public:
           static const std::string itemName;
       };

       class ALQ_SHIFT {
       public:
           static const std::string itemName;
       };

       class THP_SHIFT {
       public:
           static const std::string itemName;
       };
   };



   class WLIFTOPT : public ParserKeyword {
   public:
       WLIFTOPT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class USE_OPTIMIZER {
       public:
           static const std::string itemName;
       };

       class MAX_LIFT_GAS_RATE {
       public:
           static const std::string itemName;
       };

       class WEIGHT_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MIN_LIFT_GAS_RATE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DELTA_GAS_RATE_WEIGHT_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ALLOCATE_EXTRA_LIFT_GAS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WLIMTOL : public ParserKeyword {
   public:
       WLIMTOL();
       static const std::string keywordName;

       class TOLERANCE_FRACTION {
       public:
           static const std::string itemName;
       };
   };



   class WLIST : public ParserKeyword {
   public:
       WLIST();
       static const std::string keywordName;

       class NAME {
       public:
           static const std::string itemName;
       };

       class ACTION {
       public:
           static const std::string itemName;
       };

       class WELLS {
       public:
           static const std::string itemName;
       };
   };



   class WLISTARG : public ParserKeyword {
   public:
       WLISTARG();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CONTROL {
       public:
           static const std::string itemName;
       };

       class VALUES {
       public:
           static const std::string itemName;
       };
   };



   class WLISTNAM : public ParserKeyword {
   public:
       WLISTNAM();
       static const std::string keywordName;

       class LIST_NAME {
       public:
           static const std::string itemName;
       };

       class WELLS {
       public:
           static const std::string itemName;
       };
   };



   class WNETCTRL : public ParserKeyword {
   public:
       WNETCTRL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CONTROL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WNETDP : public ParserKeyword {
   public:
       WNETDP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DP {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WORKLIM : public ParserKeyword {
   public:
       WORKLIM();
       static const std::string keywordName;

       class LIMIT {
       public:
           static const std::string itemName;
       };
   };



   class WORKTHP : public ParserKeyword {
   public:
       WORKTHP();
       static const std::string keywordName;

       class WELL_NAME {
       public:
           static const std::string itemName;
       };

       class WORK_OVER_PROCEDURE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WPAVE : public ParserKeyword {
   public:
       WPAVE();
       static const std::string keywordName;

       class WEIGTH_FACTOR1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WEIGTH_FACTOR2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DEPTH_CORRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CONNECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WPAVEDEP : public ParserKeyword {
   public:
       WPAVEDEP();
       static const std::string keywordName;

       class WELLNAME {
       public:
           static const std::string itemName;
       };

       class REFDEPTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WPIMULT : public ParserKeyword {
   public:
       WPIMULT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WELLPI {
       public:
           static const std::string itemName;
           static const double defaultValue;
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

       class FIRST {
       public:
           static const std::string itemName;
       };

       class LAST {
       public:
           static const std::string itemName;
       };
   };



   class WPIMULTL : public ParserKeyword {
   public:
       WPIMULTL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class WELLPI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GRID {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
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

       class FIRST {
       public:
           static const std::string itemName;
       };

       class LAST {
       public:
           static const std::string itemName;
       };
   };



   class WPITAB : public ParserKeyword {
   public:
       WPITAB();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class PI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WPLUG : public ParserKeyword {
   public:
       WPLUG();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class LENGTH_TOP {
       public:
           static const std::string itemName;
       };

       class LENGTH_BOTTOM {
       public:
           static const std::string itemName;
       };

       class SOURCE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WPMITAB : public ParserKeyword {
   public:
       WPMITAB();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TABLE_NUMBER {
       public:
           static const std::string itemName;
       };
   };



   class WPOLYMER : public ParserKeyword {
   public:
       WPOLYMER();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class POLYMER_CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class GROUP_POLYMER_CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class GROUP_SALT_CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class WPOLYRED : public ParserKeyword {
   public:
       WPOLYRED();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class POLYMER_VISCOSITY_RED {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WPOTCALC : public ParserKeyword {
   public:
       WPOTCALC();
       static const std::string keywordName;

       class FORCE_CALC {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class USE_SEGMENT_VOLUMES {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WREGROUP : public ParserKeyword {
   public:
       WREGROUP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class QUANTITY {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GROUP_UPPER {
       public:
           static const std::string itemName;
       };

       class UPPER_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GROUP_LOWER {
       public:
           static const std::string itemName;
       };

       class LOWER_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WRFT : public ParserKeyword {
   public:
       WRFT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };
   };



   class WRFTPLT : public ParserKeyword {
   public:
       WRFTPLT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class OUTPUT_RFT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OUTPUT_PLT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OUTPUT_SEGMENT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WSALT : public ParserKeyword {
   public:
       WSALT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CONCENTRATION {
       public:
           static const std::string itemName;
       };
   };



   class WSCCLEAN : public ParserKeyword {
   public:
       WSCCLEAN();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SCALE_MULTIPLIER {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class I {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class C1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class C2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WSCCLENL : public ParserKeyword {
   public:
       WSCCLENL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SCALE_MULTIPLIER {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class LGR {
       public:
           static const std::string itemName;
       };

       class I {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class J {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class K {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class C1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class C2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WSCTAB : public ParserKeyword {
   public:
       WSCTAB();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SCALE_DEPOSITION_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class SCALE_DAMAGE_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class UNUSED {
       public:
           static const std::string itemName;
       };

       class SCALE_DISSOLUTION_TABLE {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WSEGAICD : public ParserKeyword {
   public:
       WSEGAICD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class AICD_STRENGTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class ICD_LENGTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class RHO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class VISCOSITY {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class TRANSITION_WIDTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_SOMETHING {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class SCALING_METHOD {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_QICD {
       public:
           static const std::string itemName;
       };

       class FLOW_RATE_EXPONENT {
       public:
           static const std::string itemName;
       };

       class VISC_EXPONENT {
       public:
           static const std::string itemName;
       };

       class ICD_FLAG {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class OIL_FLOW_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_FLOW_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_FLOW_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OIL_VSIC_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WATER_VISC_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAS_VISC_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGDFIN : public ParserKeyword {
   public:
       WSEGDFIN();
       static const std::string keywordName;

       class DATA {
       public:
           static const std::string itemName;
       };
   };



   class WSEGDFMD : public ParserKeyword {
   public:
       WSEGDFMD();
       static const std::string keywordName;

       class DRIFT_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class INCLINATION_FACTOR {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_EFFECT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WSEGDFPA : public ParserKeyword {
   public:
       WSEGDFPA();
       static const std::string keywordName;

       class GAS_LIQUID_VD_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class A1 {
       public:
           static const std::string itemName;
       };

       class A2 {
       public:
           static const std::string itemName;
       };

       class C0_A {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class C0_B {
       public:
           static const std::string itemName;
       };

       class FV {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class OIL_WATER_VD_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class A {
       public:
           static const std::string itemName;
       };

       class B1 {
       public:
           static const std::string itemName;
       };

       class B2 {
       public:
           static const std::string itemName;
       };

       class N {
       public:
           static const std::string itemName;
       };
   };



   class WSEGDIMS : public ParserKeyword {
   public:
       WSEGDIMS();
       static const std::string keywordName;

       class NSWLMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NSEGMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NLBRMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class NCRDMX {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WSEGEXSS : public ParserKeyword {
   public:
       WSEGEXSS();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT {
       public:
           static const std::string itemName;
       };

       class TYPE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_IMPORT_RATE {
       public:
           static const std::string itemName;
       };

       class R {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PEXT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGFLIM : public ParserKeyword {
   public:
       WSEGFLIM();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class LIMITED_PHASE1 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLOW_LIMIT1 {
       public:
           static const std::string itemName;
       };

       class A1 {
       public:
           static const std::string itemName;
       };

       class LIMITED_PHASE2 {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class FLOW_LIMIT2 {
       public:
           static const std::string itemName;
       };

       class A2 {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WSEGFMOD : public ParserKeyword {
   public:
       WSEGFMOD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class FLOW_MODEL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class GAS_LIQUID_VD_FACTOR {
       public:
           static const std::string itemName;
       };

       class A {
       public:
           static const std::string itemName;
       };

       class B {
       public:
           static const std::string itemName;
       };

       class FV {
       public:
           static const std::string itemName;
       };

       class B1 {
       public:
           static const std::string itemName;
       };

       class B2 {
       public:
           static const std::string itemName;
       };
   };



   class WSEGINIT : public ParserKeyword {
   public:
       WSEGINIT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class P0 {
       public:
           static const std::string itemName;
       };

       class W0 {
       public:
           static const std::string itemName;
       };

       class G0 {
       public:
           static const std::string itemName;
       };

       class RS {
       public:
           static const std::string itemName;
       };

       class RV {
       public:
           static const std::string itemName;
       };

       class API {
       public:
           static const std::string itemName;
       };

       class POLYMER {
       public:
           static const std::string itemName;
       };

       class BRINE {
       public:
           static const std::string itemName;
       };
   };



   class WSEGITER : public ParserKeyword {
   public:
       WSEGITER();
       static const std::string keywordName;

       class MAX_WELL_ITERATIONS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_TIMES_REDUCED {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class REDUCTION_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class INCREASING_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGLABY : public ParserKeyword {
   public:
       WSEGLABY();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class NCONFIG {
       public:
           static const std::string itemName;
       };

       class CHANNELS {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class A {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class L1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class L2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class D {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class R {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAMMA_INLET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAMMA_OUTLET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GAMMA_LAB {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WSEGLINK : public ParserKeyword {
   public:
       WSEGLINK();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };
   };



   class WSEGMULT : public ParserKeyword {
   public:
       WSEGMULT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class A {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class B {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class C {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class D {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class E {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class GOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGPROP : public ParserKeyword {
   public:
       WSEGPROP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class D {
       public:
           static const std::string itemName;
       };

       class R {
       public:
           static const std::string itemName;
       };

       class A {
       public:
           static const std::string itemName;
       };

       class V {
       public:
           static const std::string itemName;
       };

       class PIPEA {
       public:
           static const std::string itemName;
       };

       class CV {
       public:
           static const std::string itemName;
       };

       class K {
       public:
           static const std::string itemName;
       };
   };



   class WSEGSEP : public ParserKeyword {
   public:
       WSEGSEP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT1 {
       public:
           static const std::string itemName;
       };

       class SEGMENT2 {
       public:
           static const std::string itemName;
       };

       class PHASE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class EMAX {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class HOLDUP_FRACTION {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGSICD : public ParserKeyword {
   public:
       WSEGSICD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEG1 {
       public:
           static const std::string itemName;
       };

       class SEG2 {
       public:
           static const std::string itemName;
       };

       class STRENGTH {
       public:
           static const std::string itemName;
       };

       class LENGTH {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DENSITY_CALI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class VISCOSITY_CALI {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class CRITICAL_VALUE {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class WIDTH_TRANS {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class MAX_VISC_RATIO {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class METHOD_SCALING_FACTOR {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class MAX_ABS_RATE {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WSEGSOLV : public ParserKeyword {
   public:
       WSEGSOLV();
       static const std::string keywordName;

       class USE_ITERATIVE_SOLVER {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_LINEAR_ITER {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class CONVERGENCE_TARGET {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class PC_FILL {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class DROP_TOL {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class P_LIMIT {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class LOOP_THRESHOLD {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class LOOP_MULTIPLIER {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WSEGTABL : public ParserKeyword {
   public:
       WSEGTABL();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SURFACT {
       public:
           static const std::string itemName;
       };
   };



   class WSEGVALV : public ParserKeyword {
   public:
       WSEGVALV();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SEGMENT_NUMBER {
       public:
           static const std::string itemName;
       };

       class CV {
       public:
           static const std::string itemName;
       };

       class AREA {
       public:
           static const std::string itemName;
       };

       class EXTRA_LENGTH {
       public:
           static const std::string itemName;
       };

       class PIPE_D {
       public:
           static const std::string itemName;
       };

       class ROUGHNESS {
       public:
           static const std::string itemName;
       };

       class PIPE_A {
       public:
           static const std::string itemName;
       };

       class STATUS {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class MAX_A {
       public:
           static const std::string itemName;
       };
   };



   class WSKPTAB : public ParserKeyword {
   public:
       WSKPTAB();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TABLE_NUMBER_WATER {
       public:
           static const std::string itemName;
       };

       class TABLE_NUMBER_POLYMER {
       public:
           static const std::string itemName;
       };
   };



   class WSOLVENT : public ParserKeyword {
   public:
       WSOLVENT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class SOLVENT_FRACTION {
       public:
           static const std::string itemName;
       };
   };



   class WTADD : public ParserKeyword {
   public:
       WTADD();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CONTROL {
       public:
           static const std::string itemName;
       };

       class SHIFT {
       public:
           static const std::string itemName;
       };

       class NUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WTEMP : public ParserKeyword {
   public:
       WTEMP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TEMP {
       public:
           static const std::string itemName;
       };
   };



   class WTEMPQ : public ParserKeyword {
   public:
       WTEMPQ();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class LGR {
       public:
           static const std::string itemName;
       };
   };



   class WTEST : public ParserKeyword {
   public:
       WTEST();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class INTERVAL {
       public:
           static const std::string itemName;
       };

       class REASON {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class TEST_NUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };

       class START_TIME {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WTHPMAX : public ParserKeyword {
   public:
       WTHPMAX();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class MAX_THP_DESIGN_LIMIT {
       public:
           static const std::string itemName;
       };

       class CONTROL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CONTROL_LIMIT {
       public:
           static const std::string itemName;
       };

       class THP_OPEN_LIMIT {
       public:
           static const std::string itemName;
       };
   };



   class WTMULT : public ParserKeyword {
   public:
       WTMULT();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class CONTROL {
       public:
           static const std::string itemName;
       };

       class FACTOR {
       public:
           static const std::string itemName;
       };

       class NUM {
       public:
           static const std::string itemName;
           static const int defaultValue;
       };
   };



   class WTRACER : public ParserKeyword {
   public:
       WTRACER();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class TRACER {
       public:
           static const std::string itemName;
       };

       class CONCENTRATION {
       public:
           static const std::string itemName;
       };

       class CUM_TRACER_FACTOR {
       public:
           static const std::string itemName;
       };

       class PRODUCTION_GROUP {
       public:
           static const std::string itemName;
       };
   };



   class WVFPDP : public ParserKeyword {
   public:
       WVFPDP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class DELTA_P {
       public:
           static const std::string itemName;
       };

       class LOSS_SCALING_FACTOR {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };
   };



   class WVFPEXP : public ParserKeyword {
   public:
       WVFPEXP();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class EXPLICIT_IMPLICIT {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class CLOSE {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class PREVENT_THP {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class EXTRAPOLATION_CONTROL {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



   class WWPAVE : public ParserKeyword {
   public:
       WWPAVE();
       static const std::string keywordName;

       class WELL {
       public:
           static const std::string itemName;
       };

       class F1 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class F2 {
       public:
           static const std::string itemName;
           static const double defaultValue;
       };

       class DEPTH_CORRECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };

       class WELL_CONNECTION {
       public:
           static const std::string itemName;
           static const std::string defaultValue;
       };
   };



}
}
#endif
